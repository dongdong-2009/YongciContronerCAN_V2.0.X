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
SHORT_STRING  product_name = {8, (unsigned char *)"YongCi"};// 产品名称
////////////////////////////////////////////////////////////////////////////////


//////////////////////函数申明/////////////////////////////////
void ResponseMACID(struct DefFrameData* pSendFrame, BYTE config);
void VisibleMsgService(struct DefFrameData* pReciveFrame, struct DefFrameData* pSendFrame);
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

BYTE  SendBufferData[10];//接收缓冲数据
BYTE  ReciveBufferData[10];//接收缓冲数据
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
    DeviceNetReciveFrame.complteFlag = 0xff;
    DeviceNetReciveFrame.pBuffer = ReciveBufferData;
    DeviceNetSendFrame.complteFlag = 0xff;
    DeviceNetSendFrame.pBuffer = SendBufferData;
    ClrWdt();
 
    //////////初始化DeviceNetObj对象////////////////////////////////
	DeviceNetObj.MACID = 0x10;                   //如果跳键没有设置从站地址，默认从站地址0x02         
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
    
    while(result)   //时间超过10ms启动看门狗复位
    {
        WorkMode = MODE_FAULT;
    }

    ClrWdt();
    WorkMode = MODE_NORMAL;
}

/*******************************************************************************
* 函数名:	void DeviceNetClassService(struct DefFrameData* pReciveFrame, struct DefFrameData* pSendFrame)
* 形参  :	struct DefFrameData * pFrame，接收报文数组 
* 返回值:    	无
* 功能描述:	DeviceNet类服务函数
            DeviceNet类只有1个属性，可选执行Get_Attribute_Single服务，响应其版本信息
*******************************************************************************/
void DeviceNetClassService(struct DefFrameData* pReciveFrame, struct DefFrameData* pSendFrame)
{  
    ClrWdt();
	if(pReciveFrame->pBuffer[1] != SVC_GET_ATTRIBUTE_SINGLE)        //不支持的服务
	{	
        ClrWdt();
        //组2信息+源MAC ID(从站ID) 
        //信息id=3，表示从站响应主站的显示请求
        pSendFrame->ID =  MAKE_GROUP2_ID(GROUP2_VISIBLE_UCN, DeviceNetObj.MACID);        //
		pSendFrame->pBuffer[0] = (pReciveFrame->pBuffer[0] & 0x7F);        //目的MAC ID(主站ID) 
		pSendFrame->pBuffer[1]= 0x80 | SVC_ERROR_RESPONSE;  //R/R=1表示响应，SVC_ERROR_RESPONSE，错误响应服务代码
		pSendFrame->pBuffer[2] = ERR_SERVICE_NOT_SUPPORT;    //ERR_SERVICE_NOT_SUPPORT ,不支持的服务
		pSendFrame->pBuffer[3] = ERR_RES_INAVAIL;            //ERR_RES_INAVAIL ,对象执行服务的资源不可用(附加错误代码)
		pSendFrame->len = 4;
        ClrWdt();
        pReciveFrame->complteFlag = 0;
        //发送
		SendData(pSendFrame);                    //发送错误响应报文
		return ;
	}
	if(pReciveFrame->pBuffer[2] != 1)                               //不存在的属性
	{	
        ClrWdt();
	    pSendFrame->ID =  MAKE_GROUP2_ID(GROUP2_VISIBLE_UCN, DeviceNetObj.MACID); 
		pSendFrame->pBuffer[0] = (pReciveFrame->pBuffer[0] & 0x7F);
		pSendFrame->pBuffer[1]= 0x80 | SVC_ERROR_RESPONSE;
		pSendFrame->pBuffer[2] = ERR_ID_INAVAIL;             //ERR_ID_INAVAIL ,不存在指定的类/实例/属性ID(附加错误代码)
		pSendFrame->pBuffer[3] = ERR_NO_ADDITIONAL_DESC;     //ERR_NO_ADDITIONAL_DESC ,无附加描述代码
        pSendFrame->len = 4;
        ClrWdt();
         pReciveFrame->complteFlag = 0;
		//发送
		SendData(pSendFrame);                    //发送错误响应报文
		return ;
	}
	//执行显示请求
	pSendFrame->ID =  MAKE_GROUP2_ID(GROUP2_VISIBLE_UCN, DeviceNetObj.MACID); 
	pSendFrame->pBuffer[0] = (pReciveFrame->pBuffer[0] & 0x7F);            //目的MAC ID(主站ID) 
	pSendFrame->pBuffer[1]= 0x80 | SVC_GET_ATTRIBUTE_SINGLE;//R/R=1表示响应，VC_Get_Attribute_Single服务代码
	pSendFrame->pBuffer[2] = DeviceNetClass.version;
	pSendFrame->pBuffer[3] = DeviceNetClass.version >> 8;   //类的版本信息
    pSendFrame->len = 4;
    ClrWdt();
    pReciveFrame->complteFlag = 0;
	//发送
    ClrWdt();
	SendData(pSendFrame);                        //发送显示信息的响应报文
}
/******************************************************************************** 
*函数名 : void DeviceNetObjService(struct DefFrameData* pReciveFrame, struct DefFrameData* pSendFrame)
*形  参 : struct DefFrameData* pReciveFrame, struct DefFrameData* pSendFrame，接收报文数组 
*返回值 :  	无
*功能描述: DeviceNet对象服务函数
********************************************************************************/
void DeviceNetObjService(struct DefFrameData* pReciveFrame, struct DefFrameData* pSendFrame)
{
   
    switch( pReciveFrame->pBuffer[1])
    {
        ClrWdt();
        case (SVC_GET_ATTRIBUTE_SINGLE):         //获取单个属性服务
	    {        
            ClrWdt();
            switch(pReciveFrame->pBuffer[4]) //属性ID
            {
                ClrWdt();
                case DEVICENET_OBJ_MACID:
                {
                    pSendFrame->ID =  MAKE_GROUP2_ID(GROUP2_VISIBLE_UCN, DeviceNetObj.MACID);              
                    pSendFrame->pBuffer[0] = (pReciveFrame->pBuffer[0] & 0x7F);    //目的MAC ID(主站ID) 
                    pSendFrame->pBuffer[1]= 0x80 | SVC_GET_ATTRIBUTE_SINGLE;//R/R=1表示响应，SVC_GET_ATTRIBUTE_SINGLE服务代码
                    pSendFrame->pBuffer[2] = DeviceNetObj.MACID;    //源MAC ID(从站ID) 
                    pSendFrame->len = 3;
                    ClrWdt();
                    pReciveFrame->complteFlag = 0;
                    //发送
                    SendData(pSendFrame);                //发送显示信息的响应报文
                    return ;             
                }
                case  DEVICENET_OBJ_BAUD:
                {
                    pSendFrame->ID =  MAKE_GROUP2_ID(GROUP2_VISIBLE_UCN, DeviceNetObj.MACID);  
                    pSendFrame->pBuffer[0] = (pReciveFrame->pBuffer[0] & 0x7F); 
                    pSendFrame->pBuffer[1]= 0x80 | SVC_GET_ATTRIBUTE_SINGLE;
                    pSendFrame->pBuffer[2] = DeviceNetObj.baudrate;//从站波特率
                    pSendFrame->len = 3;
                    ClrWdt();
                    pReciveFrame->complteFlag = 0;
                    //发送
                    SendData(pSendFrame);
                    return;
                }
                case DEVICENET_OBJ_MASTERID:
                {
                    pSendFrame->ID =  MAKE_GROUP2_ID(GROUP2_VISIBLE_UCN, DeviceNetObj.MACID);  
                    pSendFrame->pBuffer[0] = (pReciveFrame->pBuffer[0] & 0x7F); 
                    pSendFrame->pBuffer[1]= 0x80 | SVC_GET_ATTRIBUTE_SINGLE;
                    pSendFrame->pBuffer[2] = DeviceNetObj.assign_info.select;       //分配选择
                    pSendFrame->pBuffer[3] = DeviceNetObj.assign_info.master_MACID; //主站MAC ID 
                    pSendFrame->len = 4;
                    ClrWdt();
                    pReciveFrame->complteFlag = 0;
                    //发送
                    SendData(pSendFrame);
                    return ;
                }
                default:
                {
                    pSendFrame->ID =  MAKE_GROUP2_ID(GROUP2_VISIBLE_UCN, DeviceNetObj.MACID); 
                    pSendFrame->pBuffer[0] = (pReciveFrame->pBuffer[0] & 0x7F);       //目的MAC ID(主站ID) 
                    pSendFrame->pBuffer[1]= 0x80 | SVC_ERROR_RESPONSE; //R/R=1表示响应，SVC_ERROR_RESPONSE，错误响应服务代码
                    pSendFrame->pBuffer[2] = ERR_ID_INAVAIL;            //ERR_ID_INAVAIL ,不存在指定的类/实例/属性ID(附加错误代码)
                    pSendFrame->pBuffer[3] = ERR_NO_ADDITIONAL_DESC;    //ERR_NO_ADDITIONAL_DESC ,无附加描述代码
                    pSendFrame->len = 4;
                    ClrWdt();
                    pReciveFrame->complteFlag = 0;
                    //发送
                    SendData(pSendFrame);                   //发送错误响应报文
                    return ;
                }        
            }  
            break;
	    }
	    case ( SVC_SET_ATTRIBUTE_SINGLE):      //设置单个属性服务
	    {
            switch (pReciveFrame->pBuffer[3])
            {
                case DEVICENET_OBJ_MACID:
                case DEVICENET_OBJ_BAUD:
                case DEVICENET_OBJ_MASTERID://不支持设置节点地址、波特率、分配选择和主站MAC ID 
                {
                    pSendFrame->ID =  MAKE_GROUP2_ID(GROUP2_VISIBLE_UCN, DeviceNetObj.MACID); 
                    pSendFrame->pBuffer[0] = (pReciveFrame->pBuffer[0] & 0x7F);
                    pSendFrame->pBuffer[1]= 0x80 | SVC_ERROR_RESPONSE;
                    pSendFrame->pBuffer[2] = ERR_PROPERTY_NOT_SET;
                    pSendFrame->pBuffer[3] = ERR_NO_ADDITIONAL_DESC;
                    pSendFrame->len = 4;
                    ClrWdt();
                    pReciveFrame->complteFlag = 0;
                    //发送
                    SendData(pSendFrame);
                    return ;              
                }
                default:
                {
                    pSendFrame->ID =  MAKE_GROUP2_ID(GROUP2_VISIBLE_UCN, DeviceNetObj.MACID); 
                    pSendFrame->pBuffer[0] = (pReciveFrame->pBuffer[0] & 0x7F);
                    pSendFrame->pBuffer[1]= 0x80 | SVC_ERROR_RESPONSE;
                    pSendFrame->pBuffer[2] = ERR_ID_INAVAIL;
                    pSendFrame->pBuffer[3] = ERR_NO_ADDITIONAL_DESC;
                    pSendFrame->len = 4;
                    ClrWdt();
                    pReciveFrame->complteFlag = 0;
                    //发送
                    SendData(pSendFrame);
                    return ;			           
                }
            }		
        }
        case (SVC_AllOCATE_MASTER_SlAVE_CONNECTION_SET)://建立连接服务代码
        case (SVC_RELEASE_GROUP2_IDENTIFIER_SET):  //释放组2连接服务
        {	
            ClrWdt();
            UnconVisibleMsgService(pReciveFrame, pSendFrame);                            //非连接显式信息服务，
            break;
        }	
        default:
        {
            pSendFrame->ID =  MAKE_GROUP2_ID(GROUP2_VISIBLE_UCN, DeviceNetObj.MACID); 
            pSendFrame->pBuffer[0] = (pReciveFrame->pBuffer[0] & 0x7F);
            pSendFrame->pBuffer[1]= 0x80 | SVC_ERROR_RESPONSE;
            pSendFrame->pBuffer[2] = ERR_SERVICE_NOT_SUPPORT;
            pSendFrame->pBuffer[3] = ERR_NO_ADDITIONAL_DESC;
            pSendFrame->len = 4;
            ClrWdt();
            pReciveFrame->complteFlag = 0;
            //发送
            SendData(pSendFrame);
            return ;
        }
    }
}
/*******************************************************************************
* 函数名:	void ConnectionClassService(BYTE  *buf)
* 形  参 :	BYTE  *buf，接收报文数组 
* 返回值:    	无
* 功能描述:	连接类服务函数
********************************************************************************/
void ConnectionClassService(struct DefFrameData* pReciveFrame, struct DefFrameData* pSendFrame)
{
	pSendFrame->ID =  MAKE_GROUP2_ID(GROUP2_VISIBLE_UCN, DeviceNetObj.MACID); 
    ClrWdt();
	pSendFrame->pBuffer[0] = (pReciveFrame->pBuffer[0] & 0x7F);               //目的MAC ID(主站ID) 
	pSendFrame->pBuffer[1]= 0x80 | SVC_ERROR_RESPONSE;         //R/R=1表示响应，SVC_ERROR_RESPONSE，错误响应服务代码
	pSendFrame->pBuffer[2] = ERR_SERVICE_NOT_SUPPORT;           //ERR_SERVICE_NOT_SUPPORT ,不支持的服务
	pSendFrame->pBuffer[3] = 0x01;                              //附加错误代码
    pSendFrame->len = 4;
    ClrWdt();
    pReciveFrame->complteFlag = 0;
	SendData(pSendFrame);//发送
	return ;
}
/*******************************************************************************
* 函数名:	void VisibleConnectObjService(BYTE  *buf)
* 形参:	BYTE  *buf，接收报文数组 
* 返回值:    	无
* 功能描述:	显式信息连接服务函数
*******************************************************************************/
void VisibleConnectObjService(struct DefFrameData* pReciveFrame, struct DefFrameData* pSendFrame)
{
	pSendFrame->ID =  MAKE_GROUP2_ID(GROUP2_VISIBLE_UCN, DeviceNetObj.MACID); 
	pSendFrame->pBuffer[0] = (pReciveFrame->pBuffer[0] & 0x7F);     // 目的MAC ID(主站ID) 
    ClrWdt();

    switch( pReciveFrame->pBuffer[1])
    {
        ClrWdt();
        case SVC_SET_ATTRIBUTE_SINGLE://设置服务，#define SVC_SET_ATTRIBUTE_SINGLE	0x10
        {
            ClrWdt();
            pSendFrame->pBuffer[1]= 0x80 | SVC_SET_ATTRIBUTE_SINGLE;// R/R=1表示响应，pReciveFrame->pBuffer[1]是接收报文中的服务代码
            pSendFrame->pBuffer[2] =  pReciveFrame->pBuffer[5]; //根据需要添加长度限制
            pSendFrame->pBuffer[3] =  pReciveFrame->pBuffer[6];;
            pSendFrame->len = 4;
            break;       
        }
        case SVC_GET_ATTRIBUTE_SINGLE://获取服务 
        {
            pSendFrame->pBuffer[1]= 0x80 | SVC_GET_ATTRIBUTE_SINGLE;
            ClrWdt();
            switch(pReciveFrame->pBuffer[4])
            {
                case 7:                   //属性ID为7，表示通过本连接发送的最大字节数为8个
                {	 		
                    ClrWdt();	
                    pSendFrame->pBuffer[2] = 0x08;
                    pSendFrame->pBuffer[3] = 0;
                    pSendFrame->len = 4;
                    break;
                }
                case 8:             //属性ID为8，表示通过本连接接收的最大字节数为8个
                { 
                    break;
                    pSendFrame->pBuffer[2] = 0x08;
                    pSendFrame->pBuffer[3] = 0;
                    pSendFrame->len = 4;
                    ClrWdt();
                }
                default:
                {
                    ClrWdt();
                    pSendFrame->pBuffer[1]= 0x80 | SVC_ERROR_RESPONSE;
                    pSendFrame->pBuffer[2] = ERR_SERVICE_NOT_SUPPORT;
                    pSendFrame->pBuffer[3] = 0x01;
                    pSendFrame->len = 4;
                    break;
                }
            }
            break;
        }
        default:
        {
            ClrWdt();
            pSendFrame->pBuffer[1]= 0x80 | SVC_ERROR_RESPONSE;
            pSendFrame->pBuffer[2] = ERR_SERVICE_NOT_SUPPORT;
            pSendFrame->pBuffer[3] = 0x01;
            pSendFrame->len = 4;
            break;
        }            
    }  	
	//发送
    pReciveFrame->complteFlag = 0;
    ClrWdt();
	SendData(pSendFrame);
}
/********************************************************************************
* 函数名:	void CycInquireConnectObjService(BYTE  *buf)
* 形  参:	BYTE  *buf，接收报文数组 
* 返回值:    	无
* 功能描述:	轮询信息连接实例服务函数
********************************************************************************/
void CycInquireConnectObjService(struct DefFrameData* pReciveFrame, struct DefFrameData* pSendFrame)
{
	pSendFrame->ID =  MAKE_GROUP2_ID(GROUP2_VISIBLE_UCN, DeviceNetObj.MACID); 
	pSendFrame->pBuffer[0] = (pReciveFrame->pBuffer[0] & 0x7F);	
    ClrWdt();
    switch(pReciveFrame->pBuffer[1])
    {
        ClrWdt();
        case  SVC_SET_ATTRIBUTE_SINGLE:
        {
            ClrWdt();
            pSendFrame->pBuffer[1]= 0x80 | SVC_SET_ATTRIBUTE_SINGLE;     
            switch (pReciveFrame->pBuffer[4]) //检查属性
            {
                case 1: //state 设置  4-超时 3-已建立好连接 1-配置状态
                {                  
                    if (pReciveFrame->pBuffer[5] <= 4) //检查是否为有效状态
                    {
                        ClrWdt();
                        //TODO:需要进一步检查状态--状态如何转变
                        CycleInquireConnedctionObj.state = pReciveFrame->pBuffer[5]; 
                        pSendFrame->pBuffer[2] = CycleInquireConnedctionObj.state;
                        pSendFrame->len = 3;
                    }
                    else
                    {
                        ClrWdt();
                        pSendFrame->pBuffer[1]= 0x80 | SVC_ERROR_RESPONSE;
                        pSendFrame->pBuffer[2] =  ERR_PROPERTY_VALUE_INAVAIL;
                        pSendFrame->pBuffer[3] = 0xFF;
                        pSendFrame->len = 4;
                    }
                    break;                                      
                }
            }
            break;
        }
        case SVC_GET_ATTRIBUTE_SINGLE:
        {
            ClrWdt();
            pSendFrame->pBuffer[1]= 0x80 | SVC_GET_ATTRIBUTE_SINGLE;
            switch(pReciveFrame->pBuffer[4])
            {
                case 7:                   //属性ID为7，表示通过本连接发送的最大字节数为8个
                {	 		
                    ClrWdt();	
                    pSendFrame->pBuffer[2] = 0x08;
                    pSendFrame->pBuffer[3] = 0;
                    pSendFrame->len = 4;
                    break;
                }
                case 8:             //属性ID为8，表示通过本连接接收的最大字节数为8个
                { 
                    ClrWdt();
                    pSendFrame->pBuffer[2] = 0x08;
                    pSendFrame->pBuffer[3] = 0;
                    pSendFrame->len = 4;
                    break;
                }
                default:
                {
                    ClrWdt();
                    pSendFrame->pBuffer[1]= 0x80 | SVC_ERROR_RESPONSE;
                    pSendFrame->pBuffer[2] = ERR_SERVICE_NOT_SUPPORT;
                    pSendFrame->pBuffer[3] = 0x01;
                    pSendFrame->len = 4;
                    break;
                }
            }
            break;
        }
        default:
        {
            ClrWdt();
            pSendFrame->pBuffer[1]= 0x80 | SVC_ERROR_RESPONSE;
            pSendFrame->pBuffer[2] = ERR_SERVICE_NOT_SUPPORT;
            pSendFrame->pBuffer[3] = 0x01;
            pSendFrame->len = 4;
            break;
        }
    }
    ClrWdt();
    pReciveFrame->complteFlag = 0;
	SendData(pSendFrame);
}

/*******************************************************************************
* 函数名:	void IdentifierClassService(struct DefFrameData* pReciveFrame, struct DefFrameData* pSendFrame)
* 形参:	BYTE  *buf，接收报文数组 
* 返回值:    	无
* 功能描述:	标识符类服务, 不支持任何服务，错误响应
********************************************************************************/
void IdentifierClassService(struct DefFrameData* pReciveFrame, struct DefFrameData* pSendFrame)
{
	pSendFrame->ID =  MAKE_GROUP2_ID(GROUP2_VISIBLE_UCN, DeviceNetObj.MACID); 
    ClrWdt();
	pSendFrame->pBuffer[0] = (pReciveFrame->pBuffer[0] & 0x7F);
	pSendFrame->pBuffer[1]= 0x80 | SVC_ERROR_RESPONSE;
	pSendFrame->pBuffer[2] = ERR_SERVICE_NOT_SUPPORT;
	pSendFrame->pBuffer[3] = 0x01;
    pSendFrame->len = 4;
    ClrWdt();
    pReciveFrame->complteFlag = 0;
	SendData(pSendFrame);
	return ;
}
/*******************************************************************************
* 函数名:	void IdentifierObjService(struct DefFrameData* pReciveFrame, struct DefFrameData* pSendFrame)
* 形  参:	BYTE  *buf，接收报文数组 
* 返回值:    	无
* 功能描述:	标识符对象服务函数，响应主站有关标示符的请求
********************************************************************************/   
void IdentifierObjService(struct DefFrameData* pReciveFrame, struct DefFrameData* pSendFrame)
{
    pSendFrame->ID =  MAKE_GROUP2_ID(GROUP2_VISIBLE_UCN, DeviceNetObj.MACID); 
    pSendFrame->pBuffer[0] = pReciveFrame->pBuffer[0] & 0x7F; 
    ClrWdt();
   
	if(pReciveFrame->pBuffer[1] == SVC_GET_ATTRIBUTE_SINGLE)
	{	 
		pSendFrame->pBuffer[1]= 0x80 | SVC_GET_ATTRIBUTE_SINGLE;        
        USINT attribute = pReciveFrame->pBuffer[4];
        pReciveFrame->complteFlag = 0;
        ClrWdt();
        
        switch(attribute)
        {
            case IDENTIFIER_OBJ_SUPPLIERID:  //属性ID1，表示用数字标识各供应商
            {
                ClrWdt();
                pSendFrame->pBuffer[2] = IdentifierObj.providerID;
			    pSendFrame->pBuffer[3] = IdentifierObj.providerID >> 8;
                pSendFrame->len = 4;
			    SendData(pSendFrame);                
                break;
            }
            case IDENTIFIER_OBJ_TYPE://属性ID2，表示产品通用类型说明
            {
                ClrWdt();
                pSendFrame->pBuffer[2] = IdentifierObj.device_type;
			    pSendFrame->pBuffer[3] = IdentifierObj.device_type >> 8;
                pSendFrame->len = 4;
			    SendData(pSendFrame);
                break;
            }
            case IDENTIFIER_OBJ_CODE://属性ID3，表示产品代码
            {
                ClrWdt();
                pSendFrame->pBuffer[2] = IdentifierObj.product_code;
			    pSendFrame->pBuffer[3] = IdentifierObj.product_code >> 8;
                pSendFrame->len = 4;
			    SendData(pSendFrame);
                break;
            }
            case IDENTIFIER_OBJ_VERSION://属性ID4，表示产品版本
            {
                ClrWdt();
                pSendFrame->pBuffer[2] = IdentifierObj.version.major_ver;
			    pSendFrame->pBuffer[3] = IdentifierObj.version.minor_ver;
                pSendFrame->len = 4;
			    SendData(pSendFrame);
                break;
            }
             case IDENTIFIER_OBJ_STATUES://属性ID5，表示设备状态概括
             {
                 ClrWdt();
                 pSendFrame->pBuffer[2] = IdentifierObj.device_state;
			     pSendFrame->pBuffer[3] = IdentifierObj.device_state >> 8;
                 pSendFrame->len = 4;
                 ClrWdt();
			     SendData(pSendFrame);
                 break;
             }
            case IDENTIFIER_OBJ_SERIALNUM://属性ID6，表示设备序列号
            {
                ClrWdt();
             	pSendFrame->pBuffer[2] = IdentifierObj.serialID;
			    pSendFrame->pBuffer[3] = IdentifierObj.serialID >> 8;
                pSendFrame->pBuffer[4] = IdentifierObj.serialID >> 16;
                pSendFrame->pBuffer[5] = IdentifierObj.serialID >> 24;
                pSendFrame->len = 6;
                ClrWdt();
                SendData(pSendFrame);
                break;
            }
            case IDENTIFIER_OBJ_NAME://属性ID7，表示产品名称
            {
                ClrWdt();
                pSendFrame->pBuffer[2] = IdentifierObj.product_name.length;
                pSendFrame->pBuffer[3] = IdentifierObj.product_name.ucdata[0];
                pSendFrame->pBuffer[4] = IdentifierObj.product_name.ucdata[1];
                pSendFrame->pBuffer[5]= IdentifierObj.product_name.ucdata[2];
                pSendFrame->pBuffer[6] = IdentifierObj.product_name.ucdata[3];
                pSendFrame->pBuffer[7] = IdentifierObj.product_name.ucdata[4];
                pSendFrame->len = 8;
                ClrWdt();
                SendData(pSendFrame);
                break;
            }
            default://不支持的服务
            {            
                ClrWdt();
                pSendFrame->pBuffer[1]= 0x80 | SVC_ERROR_RESPONSE;
                pSendFrame->pBuffer[2] = ERR_PROPERTY_NOT_SET;
                pSendFrame->pBuffer[3] = ERR_NO_ADDITIONAL_DESC;
                pSendFrame->len = 4;
                ClrWdt();
                SendData(pSendFrame);
                break;
            }
        }
    }
    else
    {
        ClrWdt();
        pSendFrame->pBuffer[1]= 0x80 | SVC_ERROR_RESPONSE;
        pSendFrame->pBuffer[2] = ERR_SERVICE_NOT_SUPPORT;
        pSendFrame->pBuffer[3] = ERR_NO_ADDITIONAL_DESC;
        pSendFrame->len = 4;	
        ClrWdt();
        SendData(pSendFrame);
    }   
}
/********************************************************************************
* 函数名:	void RountineClassObjService(struct DefFrameData* pReciveFrame, struct DefFrameData* pSendFrame)
* 形参:	无
* 返回值:    	无
* 功能描述:	信息路由器服务，不支持任何服务，错误响应
********************************************************************************/  
void RountineClassObjService(struct DefFrameData* pReciveFrame, struct DefFrameData* pSendFrame)
{
    ClrWdt();
	pSendFrame->ID =  MAKE_GROUP2_ID(GROUP2_VISIBLE_UCN, DeviceNetObj.MACID); 
	pSendFrame->pBuffer[0] = (pReciveFrame->pBuffer[0] & 0x7F);
	pSendFrame->pBuffer[1]= (0x80 | SVC_ERROR_RESPONSE);
	pSendFrame->pBuffer[2] = ERR_SERVICE_NOT_SUPPORT;
	pSendFrame->pBuffer[3] = 0x01;
    pSendFrame->len = 4;
    ClrWdt();
    pReciveFrame->complteFlag = 0;
	SendData(pSendFrame);
	return ;	
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
        switch( function)
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
				VisibleMsgService(pReciveFrame, pSendFrame);        //显式信息服务
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
	pSendFrame->pBuffer[0] = config;	                        //请求/响应标志=1，表示响应，端口号0
	pSendFrame->pBuffer[1]= IdentifierObj.providerID;	//制造商ID低字节
	pSendFrame->pBuffer[2] = IdentifierObj.providerID >> 8;	//制造商ID高字节
	pSendFrame->pBuffer[3] = IdentifierObj.serialID;	                    //序列号低字节
	pSendFrame->pBuffer[4] = IdentifierObj.serialID >> 8;                //序列号中间字节1
	pSendFrame->pBuffer[5] = IdentifierObj.serialID >>16;          //序列号中间字节2
	pSendFrame->pBuffer[6] = IdentifierObj.serialID >>24;	//序列号高字节
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
    int sendCount = 0; 
    do
    {
        ClrWdt();
        pReciveFrame->complteFlag = 0;  //发送请求
        ResponseMACID( pSendFrame, 0);
        StartOverTimer();//启动超时定时器
        while( IsTimeRemain())
        {
            ClrWdt();
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
    if((IdentifierObj.device_state & 0x01) && (pReciveFrame->pBuffer[5] != DeviceNetObj.assign_info.master_MACID))	//验证主站
    {	//不是来自当前主站，错误响应
        ClrWdt();
        error =  ERR_OBJECT_STATE_INFLICT;
        errorAdd = 0x01;     
    }
    //仅限组2无连接信息，这个报文指向DeviceNet对象，其类ID为3。在每个DeviceNet的物理连接中只有一个DeviceNet类的实例，因此实例ID为1
    else if(pReciveFrame->pBuffer[2] != 3 || pReciveFrame->pBuffer[3] != 1)	//验证类ID和实例ID
    {   //验证类ID和实例ID错误，错误响应
        ClrWdt();
        error =   ERR_PROPERTY_VALUE_INAVAIL;
        errorAdd = ERR_NO_ADDITIONAL_DESC;         
    }
    else if(pReciveFrame->pBuffer[4] == 0)	//验证分配选择字节
    {//配置字节为零，主站没有配置从站，错误响应
        ClrWdt();
        error =  ERR_PROPERTY_VALUE_INAVAIL; //(0x80 | ERR_PROPERTY_VALUE_INAVAIL)?
        errorAdd = 0x02;    
    }
    else if(pReciveFrame->pBuffer[4] & ~(CYC_INQUIRE | VISIBLE_MSG | BIT_STROKE|STATUS_CHANGE ))  
    {//如果不是轮询配置、显示连接、位选通，错误响应
        ClrWdt();
        error =  ERR_RES_INAVAIL;
        errorAdd = 0x02;        
    }
    if (error != 0)
    {
        ClrWdt();
        pSendFrame->ID =  MAKE_GROUP2_ID(GROUP2_VISIBLE_UCN, DeviceNetObj.MACID);
        pSendFrame->pBuffer[0] = (pReciveFrame->pBuffer[0] & 0x7F);
        pSendFrame->pBuffer[1]= (0x80 | SVC_ERROR_RESPONSE);
        pSendFrame->pBuffer[2] = error;
        pSendFrame->pBuffer[3] = errorAdd;
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
    USINT config = pReciveFrame->pBuffer[4];
    ClrWdt();
    if(config == 0)   //如果配置字节为0
    {	
        ClrWdt();
        pSendFrame->ID =  MAKE_GROUP2_ID(GROUP2_VISIBLE_UCN, DeviceNetObj.MACID);      
        pSendFrame->pBuffer[0] = (pReciveFrame->pBuffer[0] & 0x7F);
        pSendFrame->pBuffer[1]= (0x80 | SVC_RELEASE_GROUP2_IDENTIFIER_SET);
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
        pSendFrame->pBuffer[0] = (pReciveFrame->pBuffer[0] & 0x7F);
        pSendFrame->pBuffer[1]= (0x80 | SVC_ERROR_RESPONSE);
        pSendFrame->pBuffer[2] = error;
        pSendFrame->pBuffer[3] = errorAdd;
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
    if(pReciveFrame->pBuffer[1] == SVC_AllOCATE_MASTER_SlAVE_CONNECTION_SET)//pReciveFrame->pBuffer[1]是收到的服务代码                                                                                                                      
	{
        ClrWdt();
        if (!CheckAllocateCode(pReciveFrame, pSendFrame))
        {          
            //检测未通过返回
            ClrWdt();
            return;
        }        
        
		DeviceNetObj.assign_info.master_MACID = pReciveFrame->pBuffer[5];  //主站告诉从站：主站的地址
        USINT config = pReciveFrame->pBuffer[4];
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
			pSendFrame->pBuffer[0] = pReciveFrame->pBuffer[0] & 0x7F;   // 目的MAC ID(主站ID) 
			pSendFrame->pBuffer[1]= (0x80 | SVC_AllOCATE_MASTER_SlAVE_CONNECTION_SET);
			pSendFrame->pBuffer[2] = 0;	               //信息体格式0,8/8：Class ID = 8 位整数，Instance ID = 8 位整数
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
			pSendFrame->pBuffer[0] = pReciveFrame->pBuffer[0] & 0x7F;
			pSendFrame->pBuffer[1]= (0x80 | SVC_AllOCATE_MASTER_SlAVE_CONNECTION_SET);
			pSendFrame->pBuffer[2] = 0;	//信息体格式0,8/8：类Class ID = 8 位整数，实例Instance ID = 8 位整数
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
			pSendFrame->pBuffer[0] = pReciveFrame->pBuffer[0] & 0x7F;   // 目的MAC ID(主站ID) 
			pSendFrame->pBuffer[1]= (0x80 | SVC_AllOCATE_MASTER_SlAVE_CONNECTION_SET);
			pSendFrame->pBuffer[2] = 0;	               //信息体格式0,8/8：Class ID = 8 位整数，Instance ID = 8 位整数
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
//			pSendFrame->pBuffer[0] = pReciveFrame->pBuffer[0] & 0x7F;
//			pSendFrame->pBuffer[1]= (0x80 | SVC_AllOCATE_MASTER_SlAVE_CONNECTION_SET);
//			pSendFrame->pBuffer[2] = 0;	//信息体格式0,8/8
//          pSendFrame->len = 3;
            ClrWdt();
//			SendData(pSendFrame);
			return ;
		}
		IdentifierObj.device_state |= 0x01;	//设备已和主站连接
		return ;
	}
	else if(pReciveFrame->pBuffer[1] == SVC_RELEASE_GROUP2_IDENTIFIER_SET)   //释放连接服务
	{
		if(!CheckReleaseCode(pReciveFrame, pSendFrame))
        {
            ClrWdt();
            return;
        }
		//释放连接
        USINT config = pReciveFrame->pBuffer[4];
        
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
		pSendFrame->pBuffer[0] = pReciveFrame->pBuffer[0] & 0x7F;
		pSendFrame->pBuffer[1]= (0x80 | SVC_RELEASE_GROUP2_IDENTIFIER_SET);		
        pSendFrame->len = 2;
        pReciveFrame->complteFlag = 0;
        ClrWdt();
		SendData(pSendFrame);
	}
	else
	{	//组2非连接显式信息服务不支持，错误响应
        ClrWdt();
		pSendFrame->ID =  MAKE_GROUP2_ID(GROUP2_VISIBLE_UCN, DeviceNetObj.MACID);
		pSendFrame->pBuffer[0] = (pReciveFrame->pBuffer[0] & 0x7F);
		pSendFrame->pBuffer[1]= (0x80 | SVC_ERROR_RESPONSE);
		pSendFrame->pBuffer[2] = ERR_SERVICE_NOT_SUPPORT;
		pSendFrame->pBuffer[3] = 0x02;
        pSendFrame->len = 4;
        pReciveFrame->complteFlag = 0;
        ClrWdt();
		SendData(pSendFrame);
        
		return ;
	}
}
/*********************************************************************************
** 函数名:	void VisibleMsgService(struct DefFrameData* pReciveFrame, struct DefFrameData* pSendFrame)
** 形参:	struct DefFrameData* pReciveFrame, struct DefFrameData* pSendFrame，接收的报文数组 
** 返回值:      无 	
** 功能描述:	显式信息服务函数，执行主站的显示请求响应
*********************************************************************************/
void VisibleMsgService(struct DefFrameData* pReciveFrame, struct DefFrameData* pSendFrame)
{
	BYTE class, obj;

	class = pReciveFrame->pBuffer[2]; //类ID
	obj = pReciveFrame->pBuffer[3];   //实例ID
    ClrWdt();
	//信息路由
	if(!(DeviceNetObj.assign_info.select & VISIBLE_MSG))	//没有建立显式信息连接 
		return ;
    
    switch(class)
    {
        ClrWdt();
        case 0x01: //标识符对象
        {   
            ClrWdt();
            if(obj == 0)	    //类服务
            {
                ClrWdt();
                IdentifierClassService( pReciveFrame, pSendFrame);
            }
            else if(obj == 1)	//实例1服务	
            {
                ClrWdt();
                IdentifierObjService(pReciveFrame, pSendFrame);
            }              
            break;
        }
        case 0x02: //信息路由器对象
        {
            ClrWdt();
            RountineClassObjService(pReciveFrame, pSendFrame);
            break;
        }
        case 0x03://DeviceNet对象
        {
            ClrWdt();
            if(obj == 0)	    //类服务
            {
                DeviceNetClassService(pReciveFrame, pSendFrame);
                ClrWdt();
                return ;
            }
            else if(obj == 1)	//实例1服务
            {
                ClrWdt();
                DeviceNetObjService(pReciveFrame, pSendFrame);
                return ;
            }
            break;
        }
        case 0x05:	//连接对象
        {
            if(obj == 0)	    //类服务
            {
                ClrWdt();
                ConnectionClassService(pReciveFrame, pSendFrame); 
                return ;
            }
            else if(obj == 1)	//显式信息连接
            {
                ClrWdt();
                VisibleConnectObjService(pReciveFrame, pSendFrame); 
                return ;
            }
            else if(obj == 2)	//I/O轮询连接
            {
                ClrWdt();
                CycInquireConnectObjService(pReciveFrame, pSendFrame);
                return ;
            }
            break;
        }   
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
** 函数名:	void DeviceMonitorPluse(void)
** 功能描述:	设备监测脉冲函数
** 形参:	无
** 返回值:      无 	
********************************************************************************/
void DeviceMonitorPluse(void)
{
    ClrWdt();
//	if(query_time_event(2))
//	{
//		start_time(160);
//		//发送检测脉冲
//		pSendFrame->pBuffer[0] = 0x80 | DeviceNetObj.MACID;
//		pSendFrame->pBuffer[1] = 0x60;
//		pSendFrame->pBuffer[2] = DeviceNetObj.assign_info.master_MACID;
//		pSendFrame->pBuffer[3]= 0x80 | SVC_MONITOR_PLUSE;
//		pSendFrame->pBuffer[4] = 0x01;		//标识符对象ID=1
//		pSendFrame->pBuffer[5] = 0x00;
//		*(send_buf + 6) = IdentifierObj.device_state;
//		*(send_buf + 7) = 0;
//		*(send_buf + 8) = 0;
//		*(send_buf + 9) = 0;
//		SendData(10, send_buf);
//	}
}


/*******************************************************************************
* 函数名:	void DeviceNetReciveCenter(uint16* id, uint8 * pdata)
* 功能描述: 从缓冲中获取数据并解析
* 形  参: uint16* pID  11bitID标识, uint8 * pbuff 缓冲数据, uint8 len 数据长度
* 返回值:      BYTE  0-信息未进行处理  1-信息进行处理	
********************************************************************************/
BOOL DeviceNetReciveCenter(uint16* pID, uint8 * pbuff, uint8 len)
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
            DeviceNetReciveFrame.pBuffer[i] = pbuff[i];
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
    CANSendData(pFrame->ID, pFrame->pBuffer, pFrame->len);
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
    g_SysTimeStamp.StarTime = g_MsTicks;
    g_SysTimeStamp.delayTime = 1000;
    ClrWdt();
    IsOverTime(g_SysTimeStamp.StarTime,g_SysTimeStamp.delayTime);
}
/*******************************************************************************
* 函数名:	BOOL IsTimeRemain()----根据具体平台需要重新
* 功能描述: 启动超时定时器
* 形  参:   null
* 返回值:   TRUE-没有超时 FALSE-超时
********************************************************************************/
BOOL IsTimeRemain()
{
    if (IsOverTime(g_SysTimeStamp.StarTime,g_SysTimeStamp.delayTime))
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