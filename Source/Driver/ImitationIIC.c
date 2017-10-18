/**
 * @file ImitationIIC.c
 * @brief 软件模拟IIC总线协议
 * copyright： Copyright (c) 2017 Beijing SOJO Electric CO., LTD.
 * company： SOJO
 * @date 2017.05.20
 *
 * @author Zhangxiaomou
 * @version ver 1.0
 */
#include "../Header.h"
#include "ImitationIIC.h"

void IIC_Start(void);
void IIC_Stop(void);
uint8_t IIC_Wait_Ack(void);
void IIC_Ack(void);
void IIC_SendByte(uint8_t byte);
uint8_t IIC_ReadByte(void);

void I2CMasterWrite(uint8_t memoryaddr , uint8_t* data , uint8_t writelen);

/**
 * 
 * @fn IIC_Start
 * @brief 产生IIC起始信号
 */
void IIC_Start(void)
{
    SDA_DIR = 0;    //SDA线为输出模式
    SDA_OUT = 1;
    SCL = 1;
    __delay_us(5);
           
    SDA_OUT = 0;    //START:when CLK is high,DATA change form high to low 
    __delay_us(5);
    SCL = 0;
}

/**
 * 
 * @fn IIC_Stop
 * @brief 产生IIC停止信号
 */
void IIC_Stop(void)
{
    SDA_DIR = 0;    //SDA线为输出模式
    SCL = 0;
    SDA_OUT = 0;    //STOP:when CLK is high DATA change form low to high
    __delay_us(5);
    SCL = 1;        //先置SCL为高
    __delay_us(5);
    SDA_OUT = 1;    //发送I2C总线结束信号
    __delay_us(5);
}

/**
 * 
 * @fn IIC_Wait_Ack
 * @brief 等待应答信号到来
 * @return 0xFF，接收应答失败
 *          0x00，接收应答成功
 */
uint8_t IIC_Wait_Ack(void)
{
    uint16_t errorCn = 0;
    SDA_DIR = 1;    //SDA为输入
    SDA_IN = 1;
    __delay_us(5);
    SCL = 1;
    __delay_us(5);
    while(SDA_IN == 1)
    {
        errorCn++;
        if(errorCn > 2250)
        {
            IIC_Stop();
			return 0xFF;
        }
    }
    SCL = 0;
    
	return 0;      
}


/**
 * 
 * @fn IIC_Ack
 * @brief 产生ACK应答
 */
void IIC_Ack(void)
{	
    SCL = 0;
	SDA_DIR = 0;    //SDA线为输出模式
	SDA_OUT = 0;
    __delay_us(5);
    SCL = 1;
    __delay_us(5);
    SCL = 0;
}

/**
 * 
 * @fn IIC_SendByte
 * @brief IIC发送一个字节
 * @param byte 要发送的字节
 */
void IIC_SendByte(uint8_t byte)
{
    uint8_t i = 0;
    SDA_DIR = 0;    //SDA设置为输出
    SCL = 0;        //拉低时钟开始数据传输
    __delay_us(5); 
    for(i = 0;i < 8;i++)
    {
        if((byte & 0x80) == 0x80)
        {
            SDA_OUT = 1;
        }
        else
        {
            SDA_OUT = 0;
        }
        byte <<= 1; 
        __delay_us(5);  //对byte5767这三个延时都是必须的
        SCL = 1;
        __delay_us(5);
		SCL = 0;	
        __delay_us(5); 
    }
}

/**
 * 
 * @fn IIC_ReadByte
 * @brief 读1个字节
 * @return 返回读取到的数据
 */
uint8_t IIC_ReadByte(void)
{
    uint8_t i = 0;
    uint8_t data = 0;
    SDA_DIR = 1;    //SDA设置为输入
    for(i = 0;i < 8;i++)
    {
        SCL = 0;
        __delay_us(5);
        SCL = 1;
        data <<= 1;
        if(SDA_IN)
        {
            data++;
        }
        __delay_us(5);
    }
    IIC_Ack();
    
    return data;
}

/**
 * 
 * @fn IIC_WriteByte
 * @brief I2C写一个字节
 * @param memoryAddr    写入缓存的地址
 * @param Deviceaddr    器件地址
 * @param data  一个字节的数据
 */
void IIC_WriteByte(uint8_t memoryAddr , uint8_t data)
{
    uint8_t deviceID = SD2405_ADDR | WD_BIT; //从机地址+写操作
    
    IIC_Start();    //产生一个启动信号
    
    IIC_SendByte(deviceID); //发送从机地址+写操作    
    if(IIC_Wait_Ack()) //等待应答
    {
        return;
    }
    
    IIC_SendByte(memoryAddr);   //发送数据写入地址
    if(IIC_Wait_Ack()) //等待应答
    {
        return;
    }
    
    IIC_SendByte(data); //发送数据
    if(IIC_Wait_Ack()) //等待应答
    {
        return;
    }
    
    IIC_Stop(); //产生一个停止条件
    ClrWdt();
    __delay_ms(1);
}

/**
 * 
 * @fn I2CMasterWrite
 * @brief 向起始地址为memoryaddr，写入数据长度为 writelen 的data
 * @param memoryaddr    指定存储器操作地址
 * @param data  指定要写入的数据
 * @param writelen  指定数据长度
 */
void I2CMasterWrite(uint8_t memoryaddr , uint8_t* data , uint8_t writelen)
{
    uint32_t i;
    for(i = 0;i< writelen;i++)
    {
       IIC_WriteByte(memoryaddr , data[i]);   //写入一个字节
       ClrWdt();
       memoryaddr++;
    }	 
}

/**
 * 
 * @fn IIC_MasterReadByte
 * @brief I2C读一个字节
 * @param memoryAddr    要读取的数据缓冲区地址
 * @return 读取到的数据 如果返回值为0xFF则无应答
 */
void IIC_MasterReadByte(uint8_t memoryAddr , uint8_t* data)
{
    uint8_t readOperate = SD2405_ADDR | RD_BIT;     //从机地址+读操作
    uint8_t writeOperate = SD2405_ADDR | WD_BIT;    //从机地址+写操作
    
    IIC_Start();    //产生一个启动信号
    IIC_SendByte(writeOperate); //发送写指令
    IIC_Wait_Ack();
    IIC_SendByte(memoryAddr);   //发送要读取的缓冲区地址
    IIC_Wait_Ack();
    
    IIC_Start();    //产生一个启动信号
    IIC_SendByte(readOperate);   //发送读指令    
    IIC_Wait_Ack();
    *data = IIC_ReadByte();  //读取数据   
    
    IIC_Stop(); //产生一个停止条件
}

/**
 * 
 * @fn I2CMasterRead
 * @brief 读 memoryaddr 为首地址的 redlen 个字节
 * @param memoryaddr 指定存储器操作地址
 * @param Rxdata 读取数据缓冲区
 * @param readlen 指定要读取的数据长度
 */
void I2CMasterRead(uint8_t* Rxdata , uint8_t readlen)
{
    uint8_t i;
    uint8_t readOperate = SD2405_ADDR | RD_BIT;     //从机地址+读操作
    
    IIC_Start();    //产生一个启动信号
    IIC_SendByte(readOperate);   //发送读指令   
    IIC_Wait_Ack();
	for ( i = 0; i < readlen; i++ )		
	{
		Rxdata[i] = IIC_ReadByte();
	}
}



