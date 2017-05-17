/*
 * File:   EEPROMOperate.c
 * Author: 张东旭
 *
 * Created on 2017年4月8日, 上午11:17
 */

#include "../Header.h"
#include "../SerialPort/RefParameter.h"

/**
 * EEPROM地址
 */
#define EEPROM_STAR_ADDRESS 0x7FF100    //EEPROM起始地址
#define EEPROM_END_ADDRESS  0x7FFFF0    //EEPROM结束地址
#define EEPROM_OFFSET_ADDRESS 0x04      //EEPROM偏移地址
#define ACCUMULATE_SUM_ADDRESS 0x7FFFFC   //累加和EEPROM地址

/**************************************************
 *函数名：WriteAWord()
 *形参： _prog_addressT addr, uint16* data
 *返回值：void
 *功能：  按地址写入一个word
****************************************************/
inline void WriteWord_EEPROM( _prog_addressT addr, uint16* data)
{
   //应禁止中断
    uint8 cn = 0;
    ClrWdt();
    __builtin_disi(0x3FFF); 
    _erase_eedata(addr, _EE_WORD); /* erase the dat[] array */ 
    ClrWdt(); 
    cn = 0;        
    while(NVMCONbits.WR)
    {
        ClrWdt();
        __builtin_disi(0x3FFF); 
        __delay_us(100);
        if (cn++ >200) //超过20ms跳出
        {
            break;             
        }
    }
    __builtin_disi(0x3FFF); 
    _write_eedata_word(addr, data[0]); /* write a word to dat[0] */
    __builtin_disi(0x3FFF); 
    ClrWdt();
    cn = 0;
    while(NVMCONbits.WR)
    {
        ClrWdt(); 
        __builtin_disi(0x3FFF); 
        __delay_us(100);
        if (cn++ >200) //超过20ms跳出
        {
            break;            
        }
    }
    __builtin_disi(0);      
}

/**************************************************
 *函数名：WriteAWord()
 *形参： _prog_addressT addr, uint16* data
 *返回值：void
 *功能：  按地址读取一个word
****************************************************/
inline void ReadWord_EEPROM( _prog_addressT addr, uint16* data)
{
    ClrWdt();
    __builtin_disi(0x3FFF); 
    _memcpy_p2d16(data, addr, _EE_WORD );
    __builtin_disi(0); 
}
/**
 * 读取EEPROM
 * @param id  配置号
 * @param data 指向数据的指针
 */
void ReadEEPROM(uint8 id,PointUint8* pPoint)
{    
    uint16 readData = 0;
    uint8 i = 0;
    _prog_addressT address;
    address = (_prog_addressT)(id * EEPROM_OFFSET_ADDRESS) + EEPROM_STAR_ADDRESS;
    for(i = 0;i < pPoint->len;i += 2)
    {
        ReadWord_EEPROM(address + i,&readData);
        pPoint->pData[i] = (readData & 0x00FF); //低位先读取
        if((pPoint->len % 2) == 0)  //对长度进行奇数校验
        {
            pPoint->pData[i + 1] = (readData >> 8);               
        }
    }
}
/**
 * 写入EEPROM
 * @param id  配置号
 * @param data 指向数据的指针
 */
void WriteEEPROM(uint8 id,PointUint8* pPoint)
{
    uint16 data[2] = {0,0};
    uint8 i = 0;
    _prog_addressT address;
    address = (_prog_addressT)(id * EEPROM_OFFSET_ADDRESS) + EEPROM_STAR_ADDRESS;
    for(i = 0;i < pPoint->len;i += 2)
    {
        if((pPoint->len % 2) == 0)  //对长度进行奇数校验
        {
            data[i] = pPoint->pData[i + 1];               
        }
        data[i] = data[i] << 8 | pPoint->pData[i];
        WriteWord_EEPROM(address + i,&data[i]);
    }
}
/**
 * 写累加和到EEPROM中
 * @param writeData 所需写的数据
 */
void WriteAccumulateSum_EEPROM(uint16* writeData)
{
    OFF_CAN_INT();  //不允许CAN中断
    
    _prog_addressT address = ACCUMULATE_SUM_ADDRESS;
    WriteWord_EEPROM(address,writeData);    //写EEPROM时关闭CAN中断
    
    ON_CAN_INT();  //允许CAN中断
}

/**
 * 读累加和
 * @param writeData 所需读取的数据
 */
void ReadAccumulateSum(uint16* readData)
{
    _prog_addressT address = ACCUMULATE_SUM_ADDRESS;
    ReadWord_EEPROM(address,readData);    
}


/**
 * 写入分合闸次数
 * @param writeData 所需读取的数据
 */
void WriteFenzhaCount(_prog_addressT addr , uint16* eedata)
{
    ReadWord_EEPROM(addr , eedata);
    
    OFF_INT();  //关闭通信中断，防止在写入EEPROM时被打断
    eedata += 1;//直接加1 对于未擦除的不予处理，直接溢出
    
    WriteWord_EEPROM(addr , eedata);
    ON_INT();
}
