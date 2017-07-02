/** 
 * <p>application name： EEPROMOperate.c</p> 
 * <p>application describing： 配置读写EEPROM函数</p> 
 * <p>copyright： Copyright (c) 2017 Beijing SOJO Electric CO., LTD.</p> 
 * <p>company： SOJO</p> 
 * <p>time： 2017.05.20</p> 
 * 
 * @updata:[日期YYYY-MM-DD] [更改人姓名][变更描述]
 * @author ZhangXiaomou 
 * @version ver 1.0
 */
#include "../Header.h"
#include "../SerialPort/RefParameter.h"

/**
 * EEPROM地址
 */
#define EEPROM_STAR_ADDRESS 0x7FF100    //EEPROM起始地址
#define EEPROM_END_ADDRESS  0x7FFFFE    //EEPROM结束地址
#define EEPROM_OFFSET_ADDRESS 0x04      //EEPROM偏移地址
#define ACCUMULATE_SUM_ADDRESS 0x7FFFFC   //累加和EEPROM地址

/**
 * 
 * <p>Function name: [WriteWord_EEPROM]</p>
 * <p>Discription: [按地址写入一个word]</p>
 * @param addr  EEPROM地址
 * @param data  读取的数据
 */
inline void WriteWord_EEPROM( _prog_addressT addr, uint16_t* data)
{
   //应禁止中断
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
        ClrWdt();
        if (cn++ >200) //超过20ms跳出
        {
            break;            
        }
    }
    __builtin_disi(0);      
}

/**
 * 
 * <p>Function name: [ReadWord_EEPROM]</p>
 * <p>Discription: [按地址读取一个word]</p>
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
 * <p>Function name: [ReadEEPROM]</p>
 * <p>Discription: [读取EEPROM中的定值]</p>
 * @param id  配置号
 * @param data 指向数据的指针
 */
void ReadEEPROM(uint8_t id,PointUint8* pPoint)
{    
    uint16_t readData = 0;
    uint8_t i = 0;
    ClrWdt();
    _prog_addressT address;
    address = (_prog_addressT)(id * EEPROM_OFFSET_ADDRESS) + EEPROM_STAR_ADDRESS;
    for(i = 0;i < pPoint->len;i += 2)
    { 
        ClrWdt();
        ReadWord_EEPROM(address + i,&readData);
        pPoint->pData[i] = (readData & 0x00FF); //低位先读取
        if((pPoint->len % 2) == 0)  //对长度进行奇数校验
        {
            pPoint->pData[i + 1] = (readData >> 8);               
        }
    }
}
/**
 * 
 * <p>Function name: [WriteEEPROM]</p>
 * <p>Discription: [将定值写入EEPROM]</p>
 * @param id  配置号
 * @param data 指向数据的指针
 */
void WriteEEPROM(uint8_t id,PointUint8* pPoint)
{
    uint16_t data[2] = {0,0};
    uint8_t i = 0;
    ClrWdt();
    _prog_addressT address;
    address = (_prog_addressT)(id * EEPROM_OFFSET_ADDRESS) + EEPROM_STAR_ADDRESS;
    for(i = 0;i < pPoint->len;i += 2)
    {
        ClrWdt();
        if((pPoint->len % 2) == 0)  //对长度进行奇数校验
        {
            data[i] = pPoint->pData[i + 1];               
        }
        data[i] = data[i] << 8 | pPoint->pData[i];
        WriteWord_EEPROM(address + i,&data[i]);
    }
}
/**
 * 
 * <p>Function name: [WriteAccumulateSum_EEPROM]</p>
 * <p>Discription: [写累加和到EEPROM中]</p>
 * @param writeData 所需写的数据
 */
void WriteAccumulateSum_EEPROM(uint16_t* writeData)
{
    OFF_CAN_INT();  //不允许CAN中断
    ClrWdt();
    _prog_addressT address = ACCUMULATE_SUM_ADDRESS;
    WriteWord_EEPROM(address,writeData);    //写EEPROM时关闭CAN中断
    
    ON_CAN_INT();  //允许CAN中断
}

/**
 * 
 * <p>Function name: [ReadAccumulateSum]</p>
 * <p>Discription: [读累加和]</p>
 * @param writeData 所需读取的数据
 */
void ReadAccumulateSum(uint16_t* readData)
{
    _prog_addressT address = ACCUMULATE_SUM_ADDRESS;
    ClrWdt();
    ReadWord_EEPROM(address,readData);    
}

/**
 * 
 * <p>Function name: [WriteFenzhaCount]</p>
 * <p>Discription: [写入分合闸次数]</p>
 * @param writeData 所需读取的数据
 */
void WriteFenzhaCount(_prog_addressT addr , uint16_t* eedata)
{
    ReadWord_EEPROM(addr , eedata);
    
    ClrWdt();
    *eedata += 1;//直接加1 对于未擦除的不予处理，直接溢出
    
    OFF_COMMUNICATION_INT();  //关闭通信中断，防止在写入EEPROM时被打断
    ClrWdt();
    WriteWord_EEPROM(addr , eedata);
    ON_COMMUNICATION_INT();
}


