#ifndef _Header_H_
#define _Header_H_

#define FCY 4e6
#include <libpic30.h>

#include "Driver/tydef.h"
#include "Driver/AdcSample.h"
#include "Driver/DevicdIO.h"
#include "Driver/Usart.h"
#include "Driver/Timer.h"
#include "Driver/EEPROMOperate.h"
#include "Driver/CAN.h"
#include "Driver/InitTemp.h"

#include "SerialPort/Action.h"
#include "SerialPort/RtuFrame.h"
#include "Yongci/yongci.h"
#include "Yongci/SwtichCondition.h"
#include "Yongci/DeviceParameter.h"

#define MAIN_ADDRESS    0xF0
#define LOCAL_ADDRESS   0xA2 //双路调试控制板子地址



#define Reset() {__asm__ volatile ("RESET");}
#endif
