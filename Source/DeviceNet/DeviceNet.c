/** 
 * <p>application name： DeviceNet.c</p> 
 * <p>application describing： 移植部分DeviceNet</p> 
 * <p>copyright： Copyright (c) 2017 Beijing SOJO Electric CO., LTD.</p> 
 * <p>company： SOJO</p> 
 * <p>time： 2017.05.20</p> 
 * 
 * @updata:[日期YYYY-MM-DD] [更改人姓名][变更描述]
 * @author FreeGo 
 * @version ver 1.0
 */
#include "DeviceNet.h"
#include "../Header.h"
#include "../Driver/CAN.h"
#include "../Driver/Timer.h"
#include "../SerialPort/Action.h"
#include "../Driver/Delay.h"


//DeviceNet 工作模式
#define  MODE_REPEAT_MAC  0xA1  //重复MAC检测
#define  MODE_NORMAL      0xA2  //正常工作模式
#define  MODE_FAULT       0xA4  //故障模式
#define  MODE_STOP        0xA8  //停止模式

////////////////////////////////////////////////////////////可考虑存入EEPROM
UINT  providerID = 0X1234;               // 供应商ID 
UINT  device_type = 0;                   // 通用设备
UINT  product_code = 0X00d2;             // 产品代码
USINT  major_ver = 0X01;
USINT  minor_ver = 0X01;                 // 版本
UDINT  serialID = 0x001169BC;            // 序列号
SHORT_STRING  product_name = {8, (uint8_t*)"YongCi"};// 产品名称
////////////////////////////////////////////////////////////////////////////////


//////////////////////函数申明/////////////////////////////////
void ResponseMACID(struct DefFrameData* pSendFrame, BYTE config);
static void CycleInquireMsgService(struct DefFrameData* pReciveFrame, struct DefFrameData* pSendFrame);
void UnconVisibleMsgService(struct DefFrameData* pReciveFrame, struct DefFrameData* pSendFrame);

BOOL IsTimeRemain();    //需要根据具体平台改写
void StartOverTimer();//需要根据具体平台改写


////////////////////连接对象变量////////////////////////////////
struct DefConnectionObj  CycleInquireConnedctionObj;//循环IO响应
struct DefConnectionObj  StatusChangedConnedctionObj;//状态改变响应
struct DefConnectionObj  VisibleConnectionObj;   //显示连接
//////////////////DeviceNet对象变量////////////////////////////
struct DefDeviceNetClass  DeviceNetClass = {2}; //
struct DefDeviceNetObj  DeviceNetObj;
struct DefIdentifierObject  IdentifierObj; 
/////////////////////////////////////////////////////////////////

BYTE  SendbufferData[10];//接收缓冲数据
BYTE  RecivebufferData[10];//接收缓冲数据
struct DefFrameData  DeviceNetReciveFrame; //接收帧处理
struct DefFrameData  DeviceNetSendFrame; //接收帧处理

BYTE  out_Data[8];//从站输出数组

static volatile USINT WorkMode = 0; //

/*******************************************************************************
* 函数名:	void InitDeviceNet()
* 形参  :	null
* 返回值:    null
* 功能描述:	初始化DeviceNet所涉及的基本数据
*******************************************************************************/
void InitDeviceNet()
{    
    uint32_t time = 0;
    
    DeviceNetReciveFrame.complteFlag = 0xff;
    DeviceNetReciveFrame.pbuffer = RecivebufferData;
    DeviceNetSendFrame.complteFlag = 0xff;
    DeviceNetSendFrame.pbuffer = SendbufferData;
    ClrWdt();
 
    //////////初始化DeviceNetObj对象////////////////////////////////
	DeviceNetObj.MACID = MAC_ID;                   //如果跳键没有设置从站地址，默认从站地址0x02
    DeviceNetObj.baudrate = 2;                   //500Kbit/s
	DeviceNetObj.assign_info.select = 0;         //初始的配置选择字节清零
	DeviceNetObj.assign_info.master_MACID =0x0A; //默认主站地址，在预定义主从连接建立过程中，主站还会告诉从站：主站的地址
    ClrWdt();

    //////////////连接对象为不存在状态//////////////////////////
	VisibleConnectionObj.state =  STATE_NOT_EXIST ;
	CycleInquireConnedctionObj.state =  STATE_NOT_EXIST ;//状态：没和主站连接，主站还没有配置从站
    StatusChangedConnedctionObj.state = STATE_NOT_EXIST;
    ClrWdt();
    ///////////////初始化标识符对象//////////////////////////
	IdentifierObj.providerID = providerID;        //providerID = 0X2620; 供应商ID 
	IdentifierObj.device_type = device_type;      //device_type = 0;通用设备
	IdentifierObj.product_code = product_code;    //product_code =0X00d2;产品代码
	IdentifierObj.version.major_ver = major_ver;  //major_ver = 1;
	IdentifierObj.version.minor_ver = minor_ver;  //minor_ver = 1;版本
	IdentifierObj.serialID = serialID;            //serialID = 0x001169BC;;序列号
	IdentifierObj.product_name = product_name;    //product_name = {8, "ADC4"};产品名称
    WorkMode = MODE_REPEAT_MAC;
    BOOL result = CheckMACID( &DeviceNetReciveFrame, &DeviceNetSendFrame);
    ClrWdt();
    
    time = g_TimeStampCollect.msTicks;
    
    //TODO:如何处理离线情况？？？？
//    while(result)   //时间超过10ms启动复位
//    {
//        WorkMode = MODE_FAULT;
//        if(IsOverTime(time,10))
//        {
//            Reset();    
//        }
//    }

    ClrWdt();
    if (result)
    {
        WorkMode =MODE_FAULT;
    }
    else
    {
        WorkMode = MODE_NORMAL;
    }
            
}

/******************************************************************************* 
* 函数名:	void InitVisibleConnectionObj(void)
* 形  参:	无
* 返回值:    	无
* 功能描述:	显式信息连接配置函数
********************************************************************************/   
void InitVisibleConnectionObj(void)
{
    ClrWdt();
	VisibleConnectionObj.state = STATE_LINKED;	                //已建立状态
	VisibleConnectionObj.instance_type = 0x00;	        //显式信息连接
	VisibleConnectionObj.transportClass_trigger = 0x83;	//传输分类3
	VisibleConnectionObj.produced_connection_id = 0xFF;	
	VisibleConnectionObj.consumed_connection_id = 0xFF;	
	VisibleConnectionObj.initial_comm_characteristics = 0x22;	//通过信息组1生产,通过信息组2消费
	VisibleConnectionObj.produced_connection_size = 0xFF;	    
    ClrWdt();
	VisibleConnectionObj.consumed_connection_size = 0xFF;		//最大接受FF字节
	VisibleConnectionObj.expected_packet_rate = 0x09C4;		    //必须设定期望包速率
	VisibleConnectionObj.watchdog_timeout_action = 1;		    //转为超时状态
	VisibleConnectionObj.produced_connection_path_length = 0;	
	VisibleConnectionObj.consumed_connection_path_length = 0xFF;	
	VisibleConnectionObj.produced_inhibit_time = 0;		        //无时间间隔
    ClrWdt();
}
/******************************************************************************* 
* 函数名:	void InitCycleInquireConnectionObj(void)
* 返回值:    	无
* 功能描述:	I/O轮询连接配置函数
* 形参:	无
********************************************************************************/    
void InitCycleInquireConnectionObj(void)
{
    ClrWdt();
	CycleInquireConnedctionObj.state = STATE_LINKED;	                //配置状态 
	CycleInquireConnedctionObj.instance_type = 0x01;	        //I/O连接
	CycleInquireConnedctionObj.transportClass_trigger = 0x82;	//传输分类2
	CycleInquireConnedctionObj.produced_connection_id = 0xFF;	
	CycleInquireConnedctionObj.consumed_connection_id = 0xFF;	
	CycleInquireConnedctionObj.initial_comm_characteristics = 0x01;	//通过信息组1生产,通过信息组2消费
	CycleInquireConnedctionObj.produced_connection_size = 0xFF;	    
	CycleInquireConnedctionObj.consumed_connection_size = 0xFF;	    
	CycleInquireConnedctionObj.expected_packet_rate = 0;		        //设定期望包速率
	CycleInquireConnedctionObj.watchdog_timeout_action = 0;		    //转为超时状态
	CycleInquireConnedctionObj.produced_connection_path_length = 0xFF;	
	CycleInquireConnedctionObj.consumed_connection_path_length = 0xFF;	
	CycleInquireConnedctionObj.produced_inhibit_time = 0;		        //无时间间隔
    ClrWdt();
}
/******************************************************************************* 
* 函数名:	void InitStatusChangedConnectionObj(void)
* 返回值:    	无
* 功能描述: 状态改变连接配置函数
* 形参:	无
********************************************************************************/    
void InitStatusChangedConnectionObj(void)
{
    ClrWdt();
	StatusChangedConnedctionObj.state = STATE_LINKED;	                //配置状态 
	StatusChangedConnedctionObj.instance_type = STATUS_CHANGE;	       
	StatusChangedConnedctionObj.transportClass_trigger = 0x82;	//传输分类2
	StatusChangedConnedctionObj.produced_connection_id = 0xFF;	
	StatusChangedConnedctionObj.consumed_connection_id = 0xFF;	
	StatusChangedConnedctionObj.initial_comm_characteristics = 0x01;	//通过信息组1生产,通过信息组2消费
	StatusChangedConnedctionObj.produced_connection_size = 0xFF;	    
	StatusChangedConnedctionObj.consumed_connection_size = 0xFF;	    
	StatusChangedConnedctionObj.expected_packet_rate = 0;		        //设定期望包速率
	StatusChangedConnedctionObj.watchdog_timeout_action = 0;		    //转为超时状态
	StatusChangedConnedctionObj.produced_connection_path_length = 0xFF;	
	StatusChangedConnedctionObj.consumed_connection_path_length = 0xFF;	
	StatusChangedConnedctionObj.produced_inhibit_time = 0;		        //无时间间隔
    ClrWdt();
}
/*******************************************************************************  
** 函数名:	void CANFrameFilter(struct DefFrameData* pReciveFrame, struct DefFrameData* pSendFrame)
** 形参:	    struct DefFrameData* pReciveFrame, struct DefFrameData* pSendFrame，接收的报文数组 
** 返回值:    	无
** 功能描述:	CAN信息过滤器函数，提取帧ID1和帧ID2中的信息，
                仅限组2设备，并对信息进行分类处理
*******************************************************************************/
void CANFrameFilter(struct DefFrameData* pReciveFrame, struct DefFrameData* pSendFrame)
{
    BYTE mac = GET_GROUP2_MAC(pReciveFrame->ID);
    BYTE function = GET_GROUP2_FUNCTION(pReciveFrame->ID);
	ClrWdt();

	if(mac == DeviceNetObj.MACID)  //仅限组2设备
	{	        
        ClrWdt();
        switch(function)
        {
            case GROUP2_REPEAT_MACID:  //重复MAC ID检查信息
            {          
                ClrWdt();
                if (mac == DeviceNetObj.MACID)
                {
                    ClrWdt();
                    ResponseMACID(pReciveFrame, 0x80);       //重复MACID检查响应函数,应答，物理端口为0
                }
                return; //MACID不匹配,丢弃
            }
            case GROUP2_VSILBLE_ONLY2: //1100 0000：非连接显示请求信息，预定义主从连接
            {           
                ClrWdt();     
				UnconVisibleMsgService(pReciveFrame, pSendFrame);    //非连接显式信息服务 
                return;
            }
            case  GROUP2_POLL_STATUS_CYCLE: //主站I/O轮询命令/状态变化/循环信息
            {     
                ClrWdt();
                CycleInquireMsgService(pReciveFrame, pSendFrame);     // I/O轮询信息服务
                return ;    
            }
            case GROUP2_VSILBLE:  //主站显式请求信息
            {         
                ClrWdt();      
                break;
            }
            default:
            {
                ClrWdt();
                break;
            }
        }
	}
}
/******************************************************************************
* 函数名:	void ResponseMACID(struct DefFrameData* pSendFrame, BYTE config)
* 形参:	无
* 返回值:    	无
* 功能描述:	检查重复MACID响应函数
******************************************************************************/
void ResponseMACID(struct DefFrameData* pSendFrame, BYTE config)
{                        
    ClrWdt();//重复MACID检查
    pSendFrame->ID =  MAKE_GROUP2_ID( GROUP2_REPEAT_MACID, DeviceNetObj.MACID); 
	pSendFrame->pbuffer[0] = config;	                        //请求/响应标志=1，表示响应，端口号0
	pSendFrame->pbuffer[1]= IdentifierObj.providerID;	//制造商ID低字节
	pSendFrame->pbuffer[2] = IdentifierObj.providerID >> 8;	//制造商ID高字节
	pSendFrame->pbuffer[3] = IdentifierObj.serialID;	                    //序列号低字节
	pSendFrame->pbuffer[4] = IdentifierObj.serialID >> 8;                //序列号中间字节1
	pSendFrame->pbuffer[5] = IdentifierObj.serialID >>16;          //序列号中间字节2
	pSendFrame->pbuffer[6] = IdentifierObj.serialID >>24;	//序列号高字节
    pSendFrame->len = 7;
    ClrWdt();
	SendData(pSendFrame);                      //发送报文
}
/*******************************************************************************
* 函数名:	BOOL CheckMACID(struct DefFrameData* pReciveFrame, struct DefFrameData* pSendFrame)
* 功能描述:	主动检查重复MACID函数。
* 形参:	无
* 返回值:       TRUE    网络上有和自己重复的地址
                FALSE   网络上没有和自己重复的地址   	
*******************************************************************************/
BOOL CheckMACID(struct DefFrameData* pReciveFrame, struct DefFrameData* pSendFrame)
{	
    CAN_msg reciveMsg;
    int sendCount = 0, result = 0; 
    do
    {
        ClrWdt();
        pReciveFrame->complteFlag = 0;  //发送请求
        ResponseMACID( pSendFrame, 0);
        StartOverTimer();//启动超时定时器
        while( IsTimeRemain())
        {
            ClrWdt();
           
            result = bufferDequeue(&reciveMsg);
            if (result)
            {
                DeviceNetReciveCenter(&reciveMsg.id, reciveMsg.data, reciveMsg.len);
            }
            
            
            if ( pReciveFrame->complteFlag)//判断是否有未发送的数据
            {
                ClrWdt();
                BYTE mac = GET_GROUP2_MAC(pReciveFrame->ID);
                BYTE function = GET_GROUP2_FUNCTION(pReciveFrame->ID);
                ClrWdt();
                if (function == GROUP2_REPEAT_MACID)
                {                  
                    if (mac == DeviceNetObj.MACID)
                    {
                        ClrWdt();
                        pReciveFrame->complteFlag = 0;
                        return TRUE; //只要有MACID一致，无论应答还是发出，均认为重复                  
                    }
                }                
                else
                {
                    continue;
                }
            }
        }
      
    }
    while(++sendCount < 2); //超过10ms触发看门狗复位
    ClrWdt();
    pReciveFrame->complteFlag = 0;
	return FALSE;	//没有重复地址
}
/********************************************************************************
** 函数名:	void CheckAllocateCode(struct DefFrameData* pReciveFrame, struct DefFrameData* pSendFrame)
** 形参:	struct DefFrameData* pReciveFrame, struct DefFrameData* pSendFrame，接收的报文数组 
** 返回值:      BYTE 0-溢出 非零为检测通过 	
** 功能描述:j检测非连接显式信息服务设置连接代码
********************************************************************************/
BOOL CheckAllocateCode(struct DefFrameData* pReciveFrame, struct DefFrameData* pSendFrame)
{  
    BYTE error = 0; //错误
    BYTE errorAdd = 0; //附加错误描述
    ClrWdt();
    
    //如果已分配主站,则检查是否来自同一主站
    if((IdentifierObj.device_state & 0x01) && (pReciveFrame->pbuffer[5] != DeviceNetObj.assign_info.master_MACID))	//验证主站
    {	//不是来自当前主站，错误响应
        ClrWdt();
        error =  ERR_OBJECT_STATE_INFLICT;
        errorAdd = 0x01;     
    }
    //仅限组2无连接信息，这个报文指向DeviceNet对象，其类ID为3。在每个DeviceNet的物理连接中只有一个DeviceNet类的实例，因此实例ID为1
    else if(pReciveFrame->pbuffer[2] != 3 || pReciveFrame->pbuffer[3] != 1)	//验证类ID和实例ID
    {   //验证类ID和实例ID错误，错误响应
        ClrWdt();
        error =   ERR_PROPERTY_VALUE_INAVAIL;
        errorAdd = ERR_NO_ADDITIONAL_DESC;         
    }
    else if(pReciveFrame->pbuffer[4] == 0)	//验证分配选择字节
    {//配置字节为零，主站没有配置从站，错误响应
        ClrWdt();
        error =  ERR_PROPERTY_VALUE_INAVAIL; //(0x80 | ERR_PROPERTY_VALUE_INAVAIL)?
        errorAdd = 0x02;    
    }
    else if(pReciveFrame->pbuffer[4] & ~(CYC_INQUIRE | VISIBLE_MSG | BIT_STROKE|STATUS_CHANGE ))  
    {//如果不是轮询配置、显示连接、位选通，错误响应
        ClrWdt();
        error =  ERR_RES_INAVAIL;
        errorAdd = 0x02;        
    }
    if (error != 0)
    {
        ClrWdt();
        pSendFrame->ID =  MAKE_GROUP2_ID(GROUP2_VISIBLE_UCN, DeviceNetObj.MACID);
        pSendFrame->pbuffer[0] = (pReciveFrame->pbuffer[0] & 0x7F);
        pSendFrame->pbuffer[1]= (0x80 | SVC_ERROR_RESPONSE);
        pSendFrame->pbuffer[2] = error;
        pSendFrame->pbuffer[3] = errorAdd;
        pSendFrame->len = 4;
        ClrWdt();
        pReciveFrame->complteFlag = 0;
        SendData(pSendFrame);  //发送报文
        return FALSE;
    }
    return TRUE;
}
    
/********************************************************************************
** 函数名:	void CheckReleaseCode(struct DefFrameData* pReciveFrame, struct DefFrameData* pSendFrame)
** 形参:	struct DefFrameData* pReciveFrame, struct DefFrameData* pSendFrame，接收的报文数组 
** 返回值:      BYTE 0-溢出 非零为检测通过 	
** 功能描述:检测非连接显式信息服务释放连接代码
********************************************************************************/
BOOL CheckReleaseCode(struct DefFrameData* pReciveFrame, struct DefFrameData* pSendFrame)
{  
    BYTE error = 0; //错误
    BYTE errorAdd = 0; //附加错误描述
    USINT config = pReciveFrame->pbuffer[4];
    ClrWdt();
    if(config == 0)   //如果配置字节为0
    {	
        ClrWdt();
        pSendFrame->ID =  MAKE_GROUP2_ID(GROUP2_VISIBLE_UCN, DeviceNetObj.MACID);      
        pSendFrame->pbuffer[0] = (pReciveFrame->pbuffer[0] & 0x7F);
        pSendFrame->pbuffer[1]= (0x80 | SVC_RELEASE_GROUP2_IDENTIFIER_SET);
        pSendFrame->len = 2;
        ClrWdt();
        pReciveFrame->complteFlag = 0;
        SendData(pSendFrame);
        return FALSE;
    }
    if(config & ~(CYC_INQUIRE | VISIBLE_MSG |BIT_STROKE|STATUS_CHANGE))//不支持的连接，错误响应
    {
        ClrWdt();
        error = ERR_RES_INAVAIL;
        errorAdd = 0x02;
    }
    else if((config & DeviceNetObj.assign_info.select) == 0)//连接不存在，错误响应
    {    
        ClrWdt();   
        error = ERR_EXISTED_MODE;
        errorAdd = 0x02;       
    }
     if (error != 0)
    {
        ClrWdt();
        pSendFrame->ID =  MAKE_GROUP2_ID(GROUP2_VISIBLE_UCN, DeviceNetObj.MACID);
        pSendFrame->pbuffer[0] = (pReciveFrame->pbuffer[0] & 0x7F);
        pSendFrame->pbuffer[1]= (0x80 | SVC_ERROR_RESPONSE);
        pSendFrame->pbuffer[2] = error;
        pSendFrame->pbuffer[3] = errorAdd;
        pSendFrame->len = 4;
        pReciveFrame->complteFlag = 0;
        ClrWdt();
        SendData(pSendFrame);  //发送报文
        return FALSE;
    }
    return TRUE;
}


/********************************************************************************
** 函数名:	void UnconVisibleMsgService(struct DefFrameData* pReciveFrame, struct DefFrameData* pSendFrame)
** 形参:	struct DefFrameData* pReciveFrame, struct DefFrameData* pSendFrame，接收的报文数组 
** 返回值:      无 	
** 功能描述:	非连接显式信息服务函数，主站用该报文命令从站配置连接
********************************************************************************/
void UnconVisibleMsgService(struct DefFrameData* pReciveFrame, struct DefFrameData* pSendFrame)
{ 
    if(pReciveFrame->pbuffer[1] == SVC_AllOCATE_MASTER_SlAVE_CONNECTION_SET)//pReciveFrame->pbuffer[1]是收到的服务代码                                                                                                                      
	{
        ClrWdt();
        if (!CheckAllocateCode(pReciveFrame, pSendFrame))
        {          
            //检测未通过返回
            ClrWdt();
            return;
        }        
        
		DeviceNetObj.assign_info.master_MACID = pReciveFrame->pbuffer[5];  //主站告诉从站：主站的地址
        USINT config = pReciveFrame->pbuffer[4];
		DeviceNetObj.assign_info.select |= config;       //配置字节
        ClrWdt();

		if(config & CYC_INQUIRE)                          //分配I/O轮询连接
		{	
            ClrWdt();
			InitCycleInquireConnectionObj();                       //I/O轮询连接配置函数
			CycleInquireConnedctionObj.produced_connection_id = MAKE_GROUP1_ID(GROUP1_POLL_STATUS_CYCLER_ACK , DeviceNetObj.MACID) ;//	produced_connection_id ?
			CycleInquireConnedctionObj.consumed_connection_id = MAKE_GROUP2_ID(GROUP2_POLL_STATUS_CYCLE, DeviceNetObj.MACID) ;// consumed_connection_id
 	        //成功执行响应
			pSendFrame->ID =  MAKE_GROUP2_ID(GROUP2_VISIBLE_UCN, DeviceNetObj.MACID);   
			pSendFrame->pbuffer[0] = pReciveFrame->pbuffer[0] & 0x7F;   // 目的MAC ID(主站ID) 
			pSendFrame->pbuffer[1]= (0x80 | SVC_AllOCATE_MASTER_SlAVE_CONNECTION_SET);
			pSendFrame->pbuffer[2] = 0;	               //信息体格式0,8/8：Class ID = 8 位整数，Instance ID = 8 位整数
            pSendFrame->len = 3;
            pReciveFrame->complteFlag = 0;
            ClrWdt();
			SendData(pSendFrame);             //发送报文
			return ;
		}
		if(config & VISIBLE_MSG)
		{	
            ClrWdt();
			InitVisibleConnectionObj();//分配显式信息连接  
			VisibleConnectionObj.produced_connection_id =  MAKE_GROUP2_ID(GROUP2_VISIBLE_UCN, DeviceNetObj.MACID);
			VisibleConnectionObj.consumed_connection_id =  MAKE_GROUP2_ID(GROUP2_VSILBLE, DeviceNetObj.MACID);
			//成功执行响应
            ClrWdt();
			pSendFrame->ID =  MAKE_GROUP2_ID(GROUP2_VISIBLE_UCN, DeviceNetObj.MACID);   
			pSendFrame->pbuffer[0] = pReciveFrame->pbuffer[0] & 0x7F;
			pSendFrame->pbuffer[1]= (0x80 | SVC_AllOCATE_MASTER_SlAVE_CONNECTION_SET);
			pSendFrame->pbuffer[2] = 0;	//信息体格式0,8/8：类Class ID = 8 位整数，实例Instance ID = 8 位整数
            pSendFrame->len = 3;
            pReciveFrame->complteFlag = 0;
            ClrWdt();
			//发送
			SendData(pSendFrame);			
		}
      if(config & STATUS_CHANGE)                          //分配主从
		{	
            ClrWdt();
			InitStatusChangedConnectionObj();                       //状态改变连接配置函数
			StatusChangedConnedctionObj.produced_connection_id = MAKE_GROUP1_ID( GROUP1_STATUS_CYCLE_ACK , DeviceNetObj.MACID) ;//	produced_connection_id ?
			StatusChangedConnedctionObj.consumed_connection_id = MAKE_GROUP2_ID(GROUP2_POLL_STATUS_CYCLE, DeviceNetObj.MACID) ;// consumed_connection_id
 	        //成功执行响应
            ClrWdt();
			pSendFrame->ID =  MAKE_GROUP2_ID(GROUP2_VISIBLE_UCN, DeviceNetObj.MACID);   
			pSendFrame->pbuffer[0] = pReciveFrame->pbuffer[0] & 0x7F;   // 目的MAC ID(主站ID) 
			pSendFrame->pbuffer[1]= (0x80 | SVC_AllOCATE_MASTER_SlAVE_CONNECTION_SET);
			pSendFrame->pbuffer[2] = 0;	               //信息体格式0,8/8：Class ID = 8 位整数，Instance ID = 8 位整数
            pSendFrame->len = 3;
            pReciveFrame->complteFlag = 0;
            ClrWdt();
			SendData(pSendFrame);             //发送报文
			return ;
		}
		if(config &  BIT_STROKE) //分配位选通连接
		{	
            ClrWdt();
//			pSendFrame->ID =  MAKE_GROUP2_ID(GROUP2_VISIBLE_UCN, DeviceNetObj.MACID);   
//			pSendFrame->pbuffer[0] = pReciveFrame->pbuffer[0] & 0x7F;
//			pSendFrame->pbuffer[1]= (0x80 | SVC_AllOCATE_MASTER_SlAVE_CONNECTION_SET);
//			pSendFrame->pbuffer[2] = 0;	//信息体格式0,8/8
//          pSendFrame->len = 3;
            ClrWdt();
//			SendData(pSendFrame);
			return ;
		}
		IdentifierObj.device_state |= 0x01;	//设备已和主站连接
		return ;
	}
	else if(pReciveFrame->pbuffer[1] == SVC_RELEASE_GROUP2_IDENTIFIER_SET)   //释放连接服务
	{
		if(!CheckReleaseCode(pReciveFrame, pSendFrame))
        {
            ClrWdt();
            return;
        }
		//释放连接
        USINT config = pReciveFrame->pbuffer[4];
        
		DeviceNetObj.assign_info.select |= (config^0xff); //取反释放相应的连接
        if(config & CYC_INQUIRE)    
        {
            ClrWdt();
            CycleInquireConnedctionObj.produced_connection_id = 0;
			CycleInquireConnedctionObj.consumed_connection_id = 0;
            CycleInquireConnedctionObj.state = STATE_NOT_EXIST ;	
        }
        if(config & STATUS_CHANGE)    
        {
            ClrWdt();
            StatusChangedConnedctionObj.produced_connection_id = 0;
			StatusChangedConnedctionObj.consumed_connection_id = 0;
            StatusChangedConnedctionObj.state = STATE_NOT_EXIST ;	
        }
        if (config & VISIBLE_MSG)
        {
            ClrWdt();
            VisibleConnectionObj.produced_connection_id =  0;
			VisibleConnectionObj.consumed_connection_id =  0;
            VisibleConnectionObj.state = STATE_NOT_EXIST ;	
        }
		//执行成功响应
		pSendFrame->ID =  MAKE_GROUP2_ID(GROUP2_VISIBLE_UCN, DeviceNetObj.MACID);   
		pSendFrame->pbuffer[0] = pReciveFrame->pbuffer[0] & 0x7F;
		pSendFrame->pbuffer[1]= (0x80 | SVC_RELEASE_GROUP2_IDENTIFIER_SET);		
        pSendFrame->len = 2;
        pReciveFrame->complteFlag = 0;
        ClrWdt();
		SendData(pSendFrame);
	}
	else
	{	//组2非连接显式信息服务不支持，错误响应
        ClrWdt();
		pSendFrame->ID =  MAKE_GROUP2_ID(GROUP2_VISIBLE_UCN, DeviceNetObj.MACID);
		pSendFrame->pbuffer[0] = (pReciveFrame->pbuffer[0] & 0x7F);
		pSendFrame->pbuffer[1]= (0x80 | SVC_ERROR_RESPONSE);
		pSendFrame->pbuffer[2] = ERR_SERVICE_NOT_SUPPORT;
		pSendFrame->pbuffer[3] = 0x02;
        pSendFrame->len = 4;
        pReciveFrame->complteFlag = 0;
        ClrWdt();
		SendData(pSendFrame);
        
		return ;
	}
}

/********************************************************************************
** 函数名:	void  CycleInquireMsgService(struct DefFrameData* pReciveFrame, struct DefFrameData* pSendFrame)
** 功能描述:	I/O轮询信息服务函数，在主站和从站之间传输数据
** 形参:	struct DefFrameData* pReciveFrame, struct DefFrameData* pSendFrame，接收的报文数组 
** 返回值:      无 	
*********************************************************************************/
static void  CycleInquireMsgService(struct DefFrameData* pReciveFrame, struct DefFrameData* pSendFrame)
{

    if(CycleInquireConnedctionObj.state != STATE_LINKED )	//轮询I/O连接没建立 
    {
        ClrWdt();
        return ;
    }
    
    FrameServer(pReciveFrame,pSendFrame);
    ClrWdt();

    pReciveFrame->complteFlag = 0;
    
	return ;
}



/*******************************************************************************
* 函数名:	void DeviceNetReciveCenter(uint16* id, uint8 * pdata)
* 功能描述: 从缓冲中获取数据并解析
* 形  参: uint16* pID  11bitID标识, uint8 * pbuff 缓冲数据, uint8 len 数据长度
* 返回值:      BYTE  0-信息未进行处理  1-信息进行处理	
********************************************************************************/
BOOL DeviceNetReciveCenter(WORD* pID, BYTE * pbuff, BYTE len)
{   
    BYTE i= 0;
    //判断是否为仅限组2---可以在滤波器设置屏蔽
    if( ((*pID) & 0x0600) != 0x0400)  //不是仅限组2报文处理
	{       
        ClrWdt();
        return FALSE;    
    }        
    
    if( DeviceNetReciveFrame.complteFlag) //
    {
        ClrWdt();
        return FALSE;
    }
   
    if (len <= 8) //最大长度限制
    {
        ClrWdt();
        DeviceNetReciveFrame.ID = *pID;   
        DeviceNetReciveFrame.len = len;
        for(i = 0; i< len; i++) //复制数据
        {
            ClrWdt();
            DeviceNetReciveFrame.pbuffer[i] = pbuff[i];
        }
        ClrWdt();
        DeviceNetReciveFrame.complteFlag = 0xff;
         
         
        switch(WorkMode)
        {
            ClrWdt();
            case MODE_NORMAL: //正常工作模式
            {
                ClrWdt();
                CANFrameFilter(&DeviceNetReciveFrame, &DeviceNetSendFrame);
                break;
            }        
        }
        ClrWdt();
        DeviceNetReciveFrame.complteFlag = 0;// 默认此处处理完成
        return TRUE;
    }
    
    return 0;
}
/*******************************************************************************
* 函数名:	void  SendData(uint16* id, uint8 * pdata)----根据具体平台需要重新
* 功能描述: 从缓冲中获取数据并解析
* 形  参: struct DefFrameData* pFrame
* 返回值:      null
********************************************************************************/
void SendData(struct DefFrameData* pFrame)
{
    ClrWdt();
    CANSendData(pFrame->ID, pFrame->pbuffer, pFrame->len);
    pFrame->complteFlag = 0;
}
/*******************************************************************************
* 函数名:	void  StartOverTimer()----根据具体平台需要重新
* 功能描述: 启动超时定时器
* 形  参:   null
* 返回值:   null
********************************************************************************/
void StartOverTimer()
{
    g_TimeStampCollect.canStartTime.startTime = g_TimeStampCollect.msTicks;
    g_TimeStampCollect.canStartTime.delayTime = 1000;  
    ClrWdt();
}
/*******************************************************************************
* 函数名:	BOOL IsTimeRemain()----根据具体平台需要重新
* 功能描述: 启动超时定时器
* 形  参:   null
* 返回值:   TRUE-没有超时 FALSE-超时
********************************************************************************/
BOOL IsTimeRemain()
{
    if (IsOverTimeStamp( &g_TimeStampCollect.canStartTime))
    {
        ClrWdt();
        return FALSE;
    }
    else
    {
        ClrWdt();
        return TRUE;
    }
}