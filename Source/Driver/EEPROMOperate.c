/**
 * @file EEPROMOperate.c
 * @brief 配置读写EEPROM函数
 * copyright： Copyright (c) 2017 Beijing SOJO Electric CO., LTD.
 * company： SOJO
 * @date 2017.05.20
 *
 * @author Zhangxiaomou
 * @version ver 1.0
 */
#include "../Header.h"
#include "../SerialPort/RefParameter.h"

/**
 * 
 * @fn WriteWord_EEPROM
 * @drief 按地址写入一个word
 * @param addr  EEPROM地址
 * @param data  写入的数据
 */
inline void WriteWord_EEPROM( _prog_addressT addr, uint16_t* data)
{
   //!< 应禁止中断
    uint8_t cn = 0;
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
        ClrWdt();
        if (cn++ >200) //!< 超过20ms跳出
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
        ClrWdt();
        if (cn++ >200) //!< 超过20ms跳出
        {
            break;            
        }
    }
    __builtin_disi(0);      
}

/**
 * 
 * @fn ReadWord_EEPROM
 * @drief 按地址读取一个word
 * @param addr  EEPROM地址
 * @param data  读取的数据
 */
inline void ReadWord_EEPROM( _prog_addressT addr, uint16_t* data)
{
    ClrWdt();
    __builtin_disi(0x3FFF); 
    _memcpy_p2d16(data, addr, _EE_WORD );
    __builtin_disi(0); 
}

/**
 * 
 * @fn SaveActionCount
 * @drief 写入分合闸次数
 * @param addr EEPROM地址
 * @param eedata 所需写入的数据
 */
void SaveActionCount(_prog_addressT addr , uint16_t* eedata)
{
    ReadWord_EEPROM(addr, eedata);
    
    ClrWdt();
    *eedata += 1;//!< 直接加1 对于未擦除的不予处理，直接溢出
    
    OFF_COMMUNICATION_INT();  //!< 关闭通信中断，防止在写入EEPROM时被打断
    ClrWdt();
    WriteWord_EEPROM(addr, eedata);
    ON_COMMUNICATION_INT();
}

/**
 * 
 * @fn ReadActionCount
 * @drief 读取分合闸次数
 * @param addr EEPROM地址
 * @param pdata 读取的数据
 */
void ReadActionCount(_prog_addressT addr , uint16_t* pdata)
{
    ClrWdt();
    ReadWord_EEPROM(addr, pdata);
    
    if(*pdata >= UINT16_MAX)
    {
        *pdata = 0;
        OFF_COMMUNICATION_INT();  //!< 关闭通信中断，防止在写入EEPROM时被打断
        ClrWdt();
        WriteWord_EEPROM(addr, pdata);
        ON_COMMUNICATION_INT();
    }  
}


