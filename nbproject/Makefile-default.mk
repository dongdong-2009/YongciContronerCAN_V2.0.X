#
# Generated Makefile - do not edit!
#
# Edit the Makefile in the project folder instead (../Makefile). Each target
# has a -pre and a -post target defined where you can add customized code.
#
# This makefile implements configuration specific macros and targets.


# Include project Makefile
ifeq "${IGNORE_LOCAL}" "TRUE"
# do not include local makefile. User is passing all local related variables already
else
include Makefile
# Include makefile containing local settings
ifeq "$(wildcard nbproject/Makefile-local-default.mk)" "nbproject/Makefile-local-default.mk"
include nbproject/Makefile-local-default.mk
endif
endif

# Environment
MKDIR=gnumkdir -p
RM=rm -f 
MV=mv 
CP=cp 

# Macros
CND_CONF=default
ifeq ($(TYPE_IMAGE), DEBUG_RUN)
IMAGE_TYPE=debug
OUTPUT_SUFFIX=elf
DEBUGGABLE_SUFFIX=elf
FINAL_IMAGE=dist/${CND_CONF}/${IMAGE_TYPE}/YongciContronerCAN_V2.0.X.${IMAGE_TYPE}.${OUTPUT_SUFFIX}
else
IMAGE_TYPE=production
OUTPUT_SUFFIX=hex
DEBUGGABLE_SUFFIX=elf
FINAL_IMAGE=dist/${CND_CONF}/${IMAGE_TYPE}/YongciContronerCAN_V2.0.X.${IMAGE_TYPE}.${OUTPUT_SUFFIX}
endif

ifeq ($(COMPARE_BUILD), true)
COMPARISON_BUILD=-mafrlcsj
else
COMPARISON_BUILD=
endif

# Object Directory
OBJECTDIR=build/${CND_CONF}/${IMAGE_TYPE}

# Distribution Directory
DISTDIR=dist/${CND_CONF}/${IMAGE_TYPE}

# Source Files Quoted if spaced
SOURCEFILES_QUOTED_IF_SPACED=Source/DeviceNet/DeviceNet.c Source/Driver/AdcSample.c Source/Driver/CAN.c Source/Driver/DeviceIO.c Source/Driver/Timer.c Source/Driver/Usart.c Source/Driver/traps.c Source/Driver/EEPROMOperate.c Source/Driver/InitTemp.c Source/Driver/Delay.c Source/Driver/ImitationIIC.c Source/Driver/SD2405.c Source/SerialPort/Action.c Source/SerialPort/CRC16.c Source/SerialPort/RtuFrame.c Source/SerialPort/RefParameter.c Source/Yongci/DeviceParameter.c Source/Yongci/SwtichCondition.c Source/Yongci/yongci.c Source/main.c

# Object Files Quoted if spaced
OBJECTFILES_QUOTED_IF_SPACED=${OBJECTDIR}/Source/DeviceNet/DeviceNet.o ${OBJECTDIR}/Source/Driver/AdcSample.o ${OBJECTDIR}/Source/Driver/CAN.o ${OBJECTDIR}/Source/Driver/DeviceIO.o ${OBJECTDIR}/Source/Driver/Timer.o ${OBJECTDIR}/Source/Driver/Usart.o ${OBJECTDIR}/Source/Driver/traps.o ${OBJECTDIR}/Source/Driver/EEPROMOperate.o ${OBJECTDIR}/Source/Driver/InitTemp.o ${OBJECTDIR}/Source/Driver/Delay.o ${OBJECTDIR}/Source/Driver/ImitationIIC.o ${OBJECTDIR}/Source/Driver/SD2405.o ${OBJECTDIR}/Source/SerialPort/Action.o ${OBJECTDIR}/Source/SerialPort/CRC16.o ${OBJECTDIR}/Source/SerialPort/RtuFrame.o ${OBJECTDIR}/Source/SerialPort/RefParameter.o ${OBJECTDIR}/Source/Yongci/DeviceParameter.o ${OBJECTDIR}/Source/Yongci/SwtichCondition.o ${OBJECTDIR}/Source/Yongci/yongci.o ${OBJECTDIR}/Source/main.o
POSSIBLE_DEPFILES=${OBJECTDIR}/Source/DeviceNet/DeviceNet.o.d ${OBJECTDIR}/Source/Driver/AdcSample.o.d ${OBJECTDIR}/Source/Driver/CAN.o.d ${OBJECTDIR}/Source/Driver/DeviceIO.o.d ${OBJECTDIR}/Source/Driver/Timer.o.d ${OBJECTDIR}/Source/Driver/Usart.o.d ${OBJECTDIR}/Source/Driver/traps.o.d ${OBJECTDIR}/Source/Driver/EEPROMOperate.o.d ${OBJECTDIR}/Source/Driver/InitTemp.o.d ${OBJECTDIR}/Source/Driver/Delay.o.d ${OBJECTDIR}/Source/Driver/ImitationIIC.o.d ${OBJECTDIR}/Source/Driver/SD2405.o.d ${OBJECTDIR}/Source/SerialPort/Action.o.d ${OBJECTDIR}/Source/SerialPort/CRC16.o.d ${OBJECTDIR}/Source/SerialPort/RtuFrame.o.d ${OBJECTDIR}/Source/SerialPort/RefParameter.o.d ${OBJECTDIR}/Source/Yongci/DeviceParameter.o.d ${OBJECTDIR}/Source/Yongci/SwtichCondition.o.d ${OBJECTDIR}/Source/Yongci/yongci.o.d ${OBJECTDIR}/Source/main.o.d

# Object Files
OBJECTFILES=${OBJECTDIR}/Source/DeviceNet/DeviceNet.o ${OBJECTDIR}/Source/Driver/AdcSample.o ${OBJECTDIR}/Source/Driver/CAN.o ${OBJECTDIR}/Source/Driver/DeviceIO.o ${OBJECTDIR}/Source/Driver/Timer.o ${OBJECTDIR}/Source/Driver/Usart.o ${OBJECTDIR}/Source/Driver/traps.o ${OBJECTDIR}/Source/Driver/EEPROMOperate.o ${OBJECTDIR}/Source/Driver/InitTemp.o ${OBJECTDIR}/Source/Driver/Delay.o ${OBJECTDIR}/Source/Driver/ImitationIIC.o ${OBJECTDIR}/Source/Driver/SD2405.o ${OBJECTDIR}/Source/SerialPort/Action.o ${OBJECTDIR}/Source/SerialPort/CRC16.o ${OBJECTDIR}/Source/SerialPort/RtuFrame.o ${OBJECTDIR}/Source/SerialPort/RefParameter.o ${OBJECTDIR}/Source/Yongci/DeviceParameter.o ${OBJECTDIR}/Source/Yongci/SwtichCondition.o ${OBJECTDIR}/Source/Yongci/yongci.o ${OBJECTDIR}/Source/main.o

# Source Files
SOURCEFILES=Source/DeviceNet/DeviceNet.c Source/Driver/AdcSample.c Source/Driver/CAN.c Source/Driver/DeviceIO.c Source/Driver/Timer.c Source/Driver/Usart.c Source/Driver/traps.c Source/Driver/EEPROMOperate.c Source/Driver/InitTemp.c Source/Driver/Delay.c Source/Driver/ImitationIIC.c Source/Driver/SD2405.c Source/SerialPort/Action.c Source/SerialPort/CRC16.c Source/SerialPort/RtuFrame.c Source/SerialPort/RefParameter.c Source/Yongci/DeviceParameter.c Source/Yongci/SwtichCondition.c Source/Yongci/yongci.c Source/main.c


CFLAGS=
ASFLAGS=
LDLIBSOPTIONS=

############# Tool locations ##########################################
# If you copy a project from one host to another, the path where the  #
# compiler is installed may be different.                             #
# If you open this project with MPLAB X in the new host, this         #
# makefile will be regenerated and the paths will be corrected.       #
#######################################################################
# fixDeps replaces a bunch of sed/cat/printf statements that slow down the build
FIXDEPS=fixDeps

.build-conf:  ${BUILD_SUBPROJECTS}
ifneq ($(INFORMATION_MESSAGE), )
	@echo $(INFORMATION_MESSAGE)
endif
	${MAKE}  -f nbproject/Makefile-default.mk dist/${CND_CONF}/${IMAGE_TYPE}/YongciContronerCAN_V2.0.X.${IMAGE_TYPE}.${OUTPUT_SUFFIX}

MP_PROCESSOR_OPTION=30F6012A
MP_LINKER_FILE_OPTION=,--script=p30F6012A.gld
# ------------------------------------------------------------------------------------
# Rules for buildStep: compile
ifeq ($(TYPE_IMAGE), DEBUG_RUN)
${OBJECTDIR}/Source/DeviceNet/DeviceNet.o: Source/DeviceNet/DeviceNet.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/Source/DeviceNet" 
	@${RM} ${OBJECTDIR}/Source/DeviceNet/DeviceNet.o.d 
	@${RM} ${OBJECTDIR}/Source/DeviceNet/DeviceNet.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  Source/DeviceNet/DeviceNet.c  -o ${OBJECTDIR}/Source/DeviceNet/DeviceNet.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MMD -MF "${OBJECTDIR}/Source/DeviceNet/DeviceNet.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_PK3=1    -omf=elf -DXPRJ_default=$(CND_CONF)  -legacy-libc  $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off   -std=gnu99
	@${FIXDEPS} "${OBJECTDIR}/Source/DeviceNet/DeviceNet.o.d" $(SILENT)  -rsi ${MP_CC_DIR}../ 
	
${OBJECTDIR}/Source/Driver/AdcSample.o: Source/Driver/AdcSample.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/Source/Driver" 
	@${RM} ${OBJECTDIR}/Source/Driver/AdcSample.o.d 
	@${RM} ${OBJECTDIR}/Source/Driver/AdcSample.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  Source/Driver/AdcSample.c  -o ${OBJECTDIR}/Source/Driver/AdcSample.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MMD -MF "${OBJECTDIR}/Source/Driver/AdcSample.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_PK3=1    -omf=elf -DXPRJ_default=$(CND_CONF)  -legacy-libc  $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off   -std=gnu99
	@${FIXDEPS} "${OBJECTDIR}/Source/Driver/AdcSample.o.d" $(SILENT)  -rsi ${MP_CC_DIR}../ 
	
${OBJECTDIR}/Source/Driver/CAN.o: Source/Driver/CAN.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/Source/Driver" 
	@${RM} ${OBJECTDIR}/Source/Driver/CAN.o.d 
	@${RM} ${OBJECTDIR}/Source/Driver/CAN.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  Source/Driver/CAN.c  -o ${OBJECTDIR}/Source/Driver/CAN.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MMD -MF "${OBJECTDIR}/Source/Driver/CAN.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_PK3=1    -omf=elf -DXPRJ_default=$(CND_CONF)  -legacy-libc  $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off   -std=gnu99
	@${FIXDEPS} "${OBJECTDIR}/Source/Driver/CAN.o.d" $(SILENT)  -rsi ${MP_CC_DIR}../ 
	
${OBJECTDIR}/Source/Driver/DeviceIO.o: Source/Driver/DeviceIO.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/Source/Driver" 
	@${RM} ${OBJECTDIR}/Source/Driver/DeviceIO.o.d 
	@${RM} ${OBJECTDIR}/Source/Driver/DeviceIO.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  Source/Driver/DeviceIO.c  -o ${OBJECTDIR}/Source/Driver/DeviceIO.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MMD -MF "${OBJECTDIR}/Source/Driver/DeviceIO.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_PK3=1    -omf=elf -DXPRJ_default=$(CND_CONF)  -legacy-libc  $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off   -std=gnu99
	@${FIXDEPS} "${OBJECTDIR}/Source/Driver/DeviceIO.o.d" $(SILENT)  -rsi ${MP_CC_DIR}../ 
	
${OBJECTDIR}/Source/Driver/Timer.o: Source/Driver/Timer.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/Source/Driver" 
	@${RM} ${OBJECTDIR}/Source/Driver/Timer.o.d 
	@${RM} ${OBJECTDIR}/Source/Driver/Timer.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  Source/Driver/Timer.c  -o ${OBJECTDIR}/Source/Driver/Timer.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MMD -MF "${OBJECTDIR}/Source/Driver/Timer.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_PK3=1    -omf=elf -DXPRJ_default=$(CND_CONF)  -legacy-libc  $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off   -std=gnu99
	@${FIXDEPS} "${OBJECTDIR}/Source/Driver/Timer.o.d" $(SILENT)  -rsi ${MP_CC_DIR}../ 
	
${OBJECTDIR}/Source/Driver/Usart.o: Source/Driver/Usart.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/Source/Driver" 
	@${RM} ${OBJECTDIR}/Source/Driver/Usart.o.d 
	@${RM} ${OBJECTDIR}/Source/Driver/Usart.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  Source/Driver/Usart.c  -o ${OBJECTDIR}/Source/Driver/Usart.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MMD -MF "${OBJECTDIR}/Source/Driver/Usart.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_PK3=1    -omf=elf -DXPRJ_default=$(CND_CONF)  -legacy-libc  $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off   -std=gnu99
	@${FIXDEPS} "${OBJECTDIR}/Source/Driver/Usart.o.d" $(SILENT)  -rsi ${MP_CC_DIR}../ 
	
${OBJECTDIR}/Source/Driver/traps.o: Source/Driver/traps.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/Source/Driver" 
	@${RM} ${OBJECTDIR}/Source/Driver/traps.o.d 
	@${RM} ${OBJECTDIR}/Source/Driver/traps.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  Source/Driver/traps.c  -o ${OBJECTDIR}/Source/Driver/traps.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MMD -MF "${OBJECTDIR}/Source/Driver/traps.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_PK3=1    -omf=elf -DXPRJ_default=$(CND_CONF)  -legacy-libc  $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off   -std=gnu99
	@${FIXDEPS} "${OBJECTDIR}/Source/Driver/traps.o.d" $(SILENT)  -rsi ${MP_CC_DIR}../ 
	
${OBJECTDIR}/Source/Driver/EEPROMOperate.o: Source/Driver/EEPROMOperate.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/Source/Driver" 
	@${RM} ${OBJECTDIR}/Source/Driver/EEPROMOperate.o.d 
	@${RM} ${OBJECTDIR}/Source/Driver/EEPROMOperate.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  Source/Driver/EEPROMOperate.c  -o ${OBJECTDIR}/Source/Driver/EEPROMOperate.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MMD -MF "${OBJECTDIR}/Source/Driver/EEPROMOperate.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_PK3=1    -omf=elf -DXPRJ_default=$(CND_CONF)  -legacy-libc  $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off   -std=gnu99
	@${FIXDEPS} "${OBJECTDIR}/Source/Driver/EEPROMOperate.o.d" $(SILENT)  -rsi ${MP_CC_DIR}../ 
	
${OBJECTDIR}/Source/Driver/InitTemp.o: Source/Driver/InitTemp.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/Source/Driver" 
	@${RM} ${OBJECTDIR}/Source/Driver/InitTemp.o.d 
	@${RM} ${OBJECTDIR}/Source/Driver/InitTemp.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  Source/Driver/InitTemp.c  -o ${OBJECTDIR}/Source/Driver/InitTemp.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MMD -MF "${OBJECTDIR}/Source/Driver/InitTemp.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_PK3=1    -omf=elf -DXPRJ_default=$(CND_CONF)  -legacy-libc  $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off   -std=gnu99
	@${FIXDEPS} "${OBJECTDIR}/Source/Driver/InitTemp.o.d" $(SILENT)  -rsi ${MP_CC_DIR}../ 
	
${OBJECTDIR}/Source/Driver/Delay.o: Source/Driver/Delay.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/Source/Driver" 
	@${RM} ${OBJECTDIR}/Source/Driver/Delay.o.d 
	@${RM} ${OBJECTDIR}/Source/Driver/Delay.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  Source/Driver/Delay.c  -o ${OBJECTDIR}/Source/Driver/Delay.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MMD -MF "${OBJECTDIR}/Source/Driver/Delay.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_PK3=1    -omf=elf -DXPRJ_default=$(CND_CONF)  -legacy-libc  $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off   -std=gnu99
	@${FIXDEPS} "${OBJECTDIR}/Source/Driver/Delay.o.d" $(SILENT)  -rsi ${MP_CC_DIR}../ 
	
${OBJECTDIR}/Source/Driver/ImitationIIC.o: Source/Driver/ImitationIIC.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/Source/Driver" 
	@${RM} ${OBJECTDIR}/Source/Driver/ImitationIIC.o.d 
	@${RM} ${OBJECTDIR}/Source/Driver/ImitationIIC.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  Source/Driver/ImitationIIC.c  -o ${OBJECTDIR}/Source/Driver/ImitationIIC.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MMD -MF "${OBJECTDIR}/Source/Driver/ImitationIIC.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_PK3=1    -omf=elf -DXPRJ_default=$(CND_CONF)  -legacy-libc  $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off   -std=gnu99
	@${FIXDEPS} "${OBJECTDIR}/Source/Driver/ImitationIIC.o.d" $(SILENT)  -rsi ${MP_CC_DIR}../ 
	
${OBJECTDIR}/Source/Driver/SD2405.o: Source/Driver/SD2405.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/Source/Driver" 
	@${RM} ${OBJECTDIR}/Source/Driver/SD2405.o.d 
	@${RM} ${OBJECTDIR}/Source/Driver/SD2405.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  Source/Driver/SD2405.c  -o ${OBJECTDIR}/Source/Driver/SD2405.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MMD -MF "${OBJECTDIR}/Source/Driver/SD2405.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_PK3=1    -omf=elf -DXPRJ_default=$(CND_CONF)  -legacy-libc  $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off   -std=gnu99
	@${FIXDEPS} "${OBJECTDIR}/Source/Driver/SD2405.o.d" $(SILENT)  -rsi ${MP_CC_DIR}../ 
	
${OBJECTDIR}/Source/SerialPort/Action.o: Source/SerialPort/Action.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/Source/SerialPort" 
	@${RM} ${OBJECTDIR}/Source/SerialPort/Action.o.d 
	@${RM} ${OBJECTDIR}/Source/SerialPort/Action.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  Source/SerialPort/Action.c  -o ${OBJECTDIR}/Source/SerialPort/Action.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MMD -MF "${OBJECTDIR}/Source/SerialPort/Action.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_PK3=1    -omf=elf -DXPRJ_default=$(CND_CONF)  -legacy-libc  $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off   -std=gnu99
	@${FIXDEPS} "${OBJECTDIR}/Source/SerialPort/Action.o.d" $(SILENT)  -rsi ${MP_CC_DIR}../ 
	
${OBJECTDIR}/Source/SerialPort/CRC16.o: Source/SerialPort/CRC16.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/Source/SerialPort" 
	@${RM} ${OBJECTDIR}/Source/SerialPort/CRC16.o.d 
	@${RM} ${OBJECTDIR}/Source/SerialPort/CRC16.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  Source/SerialPort/CRC16.c  -o ${OBJECTDIR}/Source/SerialPort/CRC16.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MMD -MF "${OBJECTDIR}/Source/SerialPort/CRC16.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_PK3=1    -omf=elf -DXPRJ_default=$(CND_CONF)  -legacy-libc  $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off   -std=gnu99
	@${FIXDEPS} "${OBJECTDIR}/Source/SerialPort/CRC16.o.d" $(SILENT)  -rsi ${MP_CC_DIR}../ 
	
${OBJECTDIR}/Source/SerialPort/RtuFrame.o: Source/SerialPort/RtuFrame.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/Source/SerialPort" 
	@${RM} ${OBJECTDIR}/Source/SerialPort/RtuFrame.o.d 
	@${RM} ${OBJECTDIR}/Source/SerialPort/RtuFrame.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  Source/SerialPort/RtuFrame.c  -o ${OBJECTDIR}/Source/SerialPort/RtuFrame.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MMD -MF "${OBJECTDIR}/Source/SerialPort/RtuFrame.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_PK3=1    -omf=elf -DXPRJ_default=$(CND_CONF)  -legacy-libc  $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off   -std=gnu99
	@${FIXDEPS} "${OBJECTDIR}/Source/SerialPort/RtuFrame.o.d" $(SILENT)  -rsi ${MP_CC_DIR}../ 
	
${OBJECTDIR}/Source/SerialPort/RefParameter.o: Source/SerialPort/RefParameter.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/Source/SerialPort" 
	@${RM} ${OBJECTDIR}/Source/SerialPort/RefParameter.o.d 
	@${RM} ${OBJECTDIR}/Source/SerialPort/RefParameter.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  Source/SerialPort/RefParameter.c  -o ${OBJECTDIR}/Source/SerialPort/RefParameter.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MMD -MF "${OBJECTDIR}/Source/SerialPort/RefParameter.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_PK3=1    -omf=elf -DXPRJ_default=$(CND_CONF)  -legacy-libc  $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off   -std=gnu99
	@${FIXDEPS} "${OBJECTDIR}/Source/SerialPort/RefParameter.o.d" $(SILENT)  -rsi ${MP_CC_DIR}../ 
	
${OBJECTDIR}/Source/Yongci/DeviceParameter.o: Source/Yongci/DeviceParameter.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/Source/Yongci" 
	@${RM} ${OBJECTDIR}/Source/Yongci/DeviceParameter.o.d 
	@${RM} ${OBJECTDIR}/Source/Yongci/DeviceParameter.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  Source/Yongci/DeviceParameter.c  -o ${OBJECTDIR}/Source/Yongci/DeviceParameter.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MMD -MF "${OBJECTDIR}/Source/Yongci/DeviceParameter.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_PK3=1    -omf=elf -DXPRJ_default=$(CND_CONF)  -legacy-libc  $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off   -std=gnu99
	@${FIXDEPS} "${OBJECTDIR}/Source/Yongci/DeviceParameter.o.d" $(SILENT)  -rsi ${MP_CC_DIR}../ 
	
${OBJECTDIR}/Source/Yongci/SwtichCondition.o: Source/Yongci/SwtichCondition.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/Source/Yongci" 
	@${RM} ${OBJECTDIR}/Source/Yongci/SwtichCondition.o.d 
	@${RM} ${OBJECTDIR}/Source/Yongci/SwtichCondition.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  Source/Yongci/SwtichCondition.c  -o ${OBJECTDIR}/Source/Yongci/SwtichCondition.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MMD -MF "${OBJECTDIR}/Source/Yongci/SwtichCondition.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_PK3=1    -omf=elf -DXPRJ_default=$(CND_CONF)  -legacy-libc  $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off   -std=gnu99
	@${FIXDEPS} "${OBJECTDIR}/Source/Yongci/SwtichCondition.o.d" $(SILENT)  -rsi ${MP_CC_DIR}../ 
	
${OBJECTDIR}/Source/Yongci/yongci.o: Source/Yongci/yongci.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/Source/Yongci" 
	@${RM} ${OBJECTDIR}/Source/Yongci/yongci.o.d 
	@${RM} ${OBJECTDIR}/Source/Yongci/yongci.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  Source/Yongci/yongci.c  -o ${OBJECTDIR}/Source/Yongci/yongci.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MMD -MF "${OBJECTDIR}/Source/Yongci/yongci.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_PK3=1    -omf=elf -DXPRJ_default=$(CND_CONF)  -legacy-libc  $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off   -std=gnu99
	@${FIXDEPS} "${OBJECTDIR}/Source/Yongci/yongci.o.d" $(SILENT)  -rsi ${MP_CC_DIR}../ 
	
${OBJECTDIR}/Source/main.o: Source/main.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/Source" 
	@${RM} ${OBJECTDIR}/Source/main.o.d 
	@${RM} ${OBJECTDIR}/Source/main.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  Source/main.c  -o ${OBJECTDIR}/Source/main.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MMD -MF "${OBJECTDIR}/Source/main.o.d"      -g -D__DEBUG -D__MPLAB_DEBUGGER_PK3=1    -omf=elf -DXPRJ_default=$(CND_CONF)  -legacy-libc  $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off   -std=gnu99
	@${FIXDEPS} "${OBJECTDIR}/Source/main.o.d" $(SILENT)  -rsi ${MP_CC_DIR}../ 
	
else
${OBJECTDIR}/Source/DeviceNet/DeviceNet.o: Source/DeviceNet/DeviceNet.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/Source/DeviceNet" 
	@${RM} ${OBJECTDIR}/Source/DeviceNet/DeviceNet.o.d 
	@${RM} ${OBJECTDIR}/Source/DeviceNet/DeviceNet.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  Source/DeviceNet/DeviceNet.c  -o ${OBJECTDIR}/Source/DeviceNet/DeviceNet.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MMD -MF "${OBJECTDIR}/Source/DeviceNet/DeviceNet.o.d"        -g -omf=elf -DXPRJ_default=$(CND_CONF)  -legacy-libc  $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off   -std=gnu99
	@${FIXDEPS} "${OBJECTDIR}/Source/DeviceNet/DeviceNet.o.d" $(SILENT)  -rsi ${MP_CC_DIR}../ 
	
${OBJECTDIR}/Source/Driver/AdcSample.o: Source/Driver/AdcSample.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/Source/Driver" 
	@${RM} ${OBJECTDIR}/Source/Driver/AdcSample.o.d 
	@${RM} ${OBJECTDIR}/Source/Driver/AdcSample.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  Source/Driver/AdcSample.c  -o ${OBJECTDIR}/Source/Driver/AdcSample.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MMD -MF "${OBJECTDIR}/Source/Driver/AdcSample.o.d"        -g -omf=elf -DXPRJ_default=$(CND_CONF)  -legacy-libc  $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off   -std=gnu99
	@${FIXDEPS} "${OBJECTDIR}/Source/Driver/AdcSample.o.d" $(SILENT)  -rsi ${MP_CC_DIR}../ 
	
${OBJECTDIR}/Source/Driver/CAN.o: Source/Driver/CAN.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/Source/Driver" 
	@${RM} ${OBJECTDIR}/Source/Driver/CAN.o.d 
	@${RM} ${OBJECTDIR}/Source/Driver/CAN.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  Source/Driver/CAN.c  -o ${OBJECTDIR}/Source/Driver/CAN.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MMD -MF "${OBJECTDIR}/Source/Driver/CAN.o.d"        -g -omf=elf -DXPRJ_default=$(CND_CONF)  -legacy-libc  $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off   -std=gnu99
	@${FIXDEPS} "${OBJECTDIR}/Source/Driver/CAN.o.d" $(SILENT)  -rsi ${MP_CC_DIR}../ 
	
${OBJECTDIR}/Source/Driver/DeviceIO.o: Source/Driver/DeviceIO.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/Source/Driver" 
	@${RM} ${OBJECTDIR}/Source/Driver/DeviceIO.o.d 
	@${RM} ${OBJECTDIR}/Source/Driver/DeviceIO.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  Source/Driver/DeviceIO.c  -o ${OBJECTDIR}/Source/Driver/DeviceIO.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MMD -MF "${OBJECTDIR}/Source/Driver/DeviceIO.o.d"        -g -omf=elf -DXPRJ_default=$(CND_CONF)  -legacy-libc  $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off   -std=gnu99
	@${FIXDEPS} "${OBJECTDIR}/Source/Driver/DeviceIO.o.d" $(SILENT)  -rsi ${MP_CC_DIR}../ 
	
${OBJECTDIR}/Source/Driver/Timer.o: Source/Driver/Timer.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/Source/Driver" 
	@${RM} ${OBJECTDIR}/Source/Driver/Timer.o.d 
	@${RM} ${OBJECTDIR}/Source/Driver/Timer.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  Source/Driver/Timer.c  -o ${OBJECTDIR}/Source/Driver/Timer.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MMD -MF "${OBJECTDIR}/Source/Driver/Timer.o.d"        -g -omf=elf -DXPRJ_default=$(CND_CONF)  -legacy-libc  $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off   -std=gnu99
	@${FIXDEPS} "${OBJECTDIR}/Source/Driver/Timer.o.d" $(SILENT)  -rsi ${MP_CC_DIR}../ 
	
${OBJECTDIR}/Source/Driver/Usart.o: Source/Driver/Usart.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/Source/Driver" 
	@${RM} ${OBJECTDIR}/Source/Driver/Usart.o.d 
	@${RM} ${OBJECTDIR}/Source/Driver/Usart.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  Source/Driver/Usart.c  -o ${OBJECTDIR}/Source/Driver/Usart.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MMD -MF "${OBJECTDIR}/Source/Driver/Usart.o.d"        -g -omf=elf -DXPRJ_default=$(CND_CONF)  -legacy-libc  $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off   -std=gnu99
	@${FIXDEPS} "${OBJECTDIR}/Source/Driver/Usart.o.d" $(SILENT)  -rsi ${MP_CC_DIR}../ 
	
${OBJECTDIR}/Source/Driver/traps.o: Source/Driver/traps.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/Source/Driver" 
	@${RM} ${OBJECTDIR}/Source/Driver/traps.o.d 
	@${RM} ${OBJECTDIR}/Source/Driver/traps.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  Source/Driver/traps.c  -o ${OBJECTDIR}/Source/Driver/traps.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MMD -MF "${OBJECTDIR}/Source/Driver/traps.o.d"        -g -omf=elf -DXPRJ_default=$(CND_CONF)  -legacy-libc  $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off   -std=gnu99
	@${FIXDEPS} "${OBJECTDIR}/Source/Driver/traps.o.d" $(SILENT)  -rsi ${MP_CC_DIR}../ 
	
${OBJECTDIR}/Source/Driver/EEPROMOperate.o: Source/Driver/EEPROMOperate.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/Source/Driver" 
	@${RM} ${OBJECTDIR}/Source/Driver/EEPROMOperate.o.d 
	@${RM} ${OBJECTDIR}/Source/Driver/EEPROMOperate.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  Source/Driver/EEPROMOperate.c  -o ${OBJECTDIR}/Source/Driver/EEPROMOperate.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MMD -MF "${OBJECTDIR}/Source/Driver/EEPROMOperate.o.d"        -g -omf=elf -DXPRJ_default=$(CND_CONF)  -legacy-libc  $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off   -std=gnu99
	@${FIXDEPS} "${OBJECTDIR}/Source/Driver/EEPROMOperate.o.d" $(SILENT)  -rsi ${MP_CC_DIR}../ 
	
${OBJECTDIR}/Source/Driver/InitTemp.o: Source/Driver/InitTemp.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/Source/Driver" 
	@${RM} ${OBJECTDIR}/Source/Driver/InitTemp.o.d 
	@${RM} ${OBJECTDIR}/Source/Driver/InitTemp.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  Source/Driver/InitTemp.c  -o ${OBJECTDIR}/Source/Driver/InitTemp.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MMD -MF "${OBJECTDIR}/Source/Driver/InitTemp.o.d"        -g -omf=elf -DXPRJ_default=$(CND_CONF)  -legacy-libc  $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off   -std=gnu99
	@${FIXDEPS} "${OBJECTDIR}/Source/Driver/InitTemp.o.d" $(SILENT)  -rsi ${MP_CC_DIR}../ 
	
${OBJECTDIR}/Source/Driver/Delay.o: Source/Driver/Delay.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/Source/Driver" 
	@${RM} ${OBJECTDIR}/Source/Driver/Delay.o.d 
	@${RM} ${OBJECTDIR}/Source/Driver/Delay.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  Source/Driver/Delay.c  -o ${OBJECTDIR}/Source/Driver/Delay.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MMD -MF "${OBJECTDIR}/Source/Driver/Delay.o.d"        -g -omf=elf -DXPRJ_default=$(CND_CONF)  -legacy-libc  $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off   -std=gnu99
	@${FIXDEPS} "${OBJECTDIR}/Source/Driver/Delay.o.d" $(SILENT)  -rsi ${MP_CC_DIR}../ 
	
${OBJECTDIR}/Source/Driver/ImitationIIC.o: Source/Driver/ImitationIIC.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/Source/Driver" 
	@${RM} ${OBJECTDIR}/Source/Driver/ImitationIIC.o.d 
	@${RM} ${OBJECTDIR}/Source/Driver/ImitationIIC.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  Source/Driver/ImitationIIC.c  -o ${OBJECTDIR}/Source/Driver/ImitationIIC.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MMD -MF "${OBJECTDIR}/Source/Driver/ImitationIIC.o.d"        -g -omf=elf -DXPRJ_default=$(CND_CONF)  -legacy-libc  $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off   -std=gnu99
	@${FIXDEPS} "${OBJECTDIR}/Source/Driver/ImitationIIC.o.d" $(SILENT)  -rsi ${MP_CC_DIR}../ 
	
${OBJECTDIR}/Source/Driver/SD2405.o: Source/Driver/SD2405.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/Source/Driver" 
	@${RM} ${OBJECTDIR}/Source/Driver/SD2405.o.d 
	@${RM} ${OBJECTDIR}/Source/Driver/SD2405.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  Source/Driver/SD2405.c  -o ${OBJECTDIR}/Source/Driver/SD2405.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MMD -MF "${OBJECTDIR}/Source/Driver/SD2405.o.d"        -g -omf=elf -DXPRJ_default=$(CND_CONF)  -legacy-libc  $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off   -std=gnu99
	@${FIXDEPS} "${OBJECTDIR}/Source/Driver/SD2405.o.d" $(SILENT)  -rsi ${MP_CC_DIR}../ 
	
${OBJECTDIR}/Source/SerialPort/Action.o: Source/SerialPort/Action.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/Source/SerialPort" 
	@${RM} ${OBJECTDIR}/Source/SerialPort/Action.o.d 
	@${RM} ${OBJECTDIR}/Source/SerialPort/Action.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  Source/SerialPort/Action.c  -o ${OBJECTDIR}/Source/SerialPort/Action.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MMD -MF "${OBJECTDIR}/Source/SerialPort/Action.o.d"        -g -omf=elf -DXPRJ_default=$(CND_CONF)  -legacy-libc  $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off   -std=gnu99
	@${FIXDEPS} "${OBJECTDIR}/Source/SerialPort/Action.o.d" $(SILENT)  -rsi ${MP_CC_DIR}../ 
	
${OBJECTDIR}/Source/SerialPort/CRC16.o: Source/SerialPort/CRC16.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/Source/SerialPort" 
	@${RM} ${OBJECTDIR}/Source/SerialPort/CRC16.o.d 
	@${RM} ${OBJECTDIR}/Source/SerialPort/CRC16.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  Source/SerialPort/CRC16.c  -o ${OBJECTDIR}/Source/SerialPort/CRC16.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MMD -MF "${OBJECTDIR}/Source/SerialPort/CRC16.o.d"        -g -omf=elf -DXPRJ_default=$(CND_CONF)  -legacy-libc  $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off   -std=gnu99
	@${FIXDEPS} "${OBJECTDIR}/Source/SerialPort/CRC16.o.d" $(SILENT)  -rsi ${MP_CC_DIR}../ 
	
${OBJECTDIR}/Source/SerialPort/RtuFrame.o: Source/SerialPort/RtuFrame.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/Source/SerialPort" 
	@${RM} ${OBJECTDIR}/Source/SerialPort/RtuFrame.o.d 
	@${RM} ${OBJECTDIR}/Source/SerialPort/RtuFrame.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  Source/SerialPort/RtuFrame.c  -o ${OBJECTDIR}/Source/SerialPort/RtuFrame.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MMD -MF "${OBJECTDIR}/Source/SerialPort/RtuFrame.o.d"        -g -omf=elf -DXPRJ_default=$(CND_CONF)  -legacy-libc  $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off   -std=gnu99
	@${FIXDEPS} "${OBJECTDIR}/Source/SerialPort/RtuFrame.o.d" $(SILENT)  -rsi ${MP_CC_DIR}../ 
	
${OBJECTDIR}/Source/SerialPort/RefParameter.o: Source/SerialPort/RefParameter.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/Source/SerialPort" 
	@${RM} ${OBJECTDIR}/Source/SerialPort/RefParameter.o.d 
	@${RM} ${OBJECTDIR}/Source/SerialPort/RefParameter.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  Source/SerialPort/RefParameter.c  -o ${OBJECTDIR}/Source/SerialPort/RefParameter.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MMD -MF "${OBJECTDIR}/Source/SerialPort/RefParameter.o.d"        -g -omf=elf -DXPRJ_default=$(CND_CONF)  -legacy-libc  $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off   -std=gnu99
	@${FIXDEPS} "${OBJECTDIR}/Source/SerialPort/RefParameter.o.d" $(SILENT)  -rsi ${MP_CC_DIR}../ 
	
${OBJECTDIR}/Source/Yongci/DeviceParameter.o: Source/Yongci/DeviceParameter.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/Source/Yongci" 
	@${RM} ${OBJECTDIR}/Source/Yongci/DeviceParameter.o.d 
	@${RM} ${OBJECTDIR}/Source/Yongci/DeviceParameter.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  Source/Yongci/DeviceParameter.c  -o ${OBJECTDIR}/Source/Yongci/DeviceParameter.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MMD -MF "${OBJECTDIR}/Source/Yongci/DeviceParameter.o.d"        -g -omf=elf -DXPRJ_default=$(CND_CONF)  -legacy-libc  $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off   -std=gnu99
	@${FIXDEPS} "${OBJECTDIR}/Source/Yongci/DeviceParameter.o.d" $(SILENT)  -rsi ${MP_CC_DIR}../ 
	
${OBJECTDIR}/Source/Yongci/SwtichCondition.o: Source/Yongci/SwtichCondition.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/Source/Yongci" 
	@${RM} ${OBJECTDIR}/Source/Yongci/SwtichCondition.o.d 
	@${RM} ${OBJECTDIR}/Source/Yongci/SwtichCondition.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  Source/Yongci/SwtichCondition.c  -o ${OBJECTDIR}/Source/Yongci/SwtichCondition.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MMD -MF "${OBJECTDIR}/Source/Yongci/SwtichCondition.o.d"        -g -omf=elf -DXPRJ_default=$(CND_CONF)  -legacy-libc  $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off   -std=gnu99
	@${FIXDEPS} "${OBJECTDIR}/Source/Yongci/SwtichCondition.o.d" $(SILENT)  -rsi ${MP_CC_DIR}../ 
	
${OBJECTDIR}/Source/Yongci/yongci.o: Source/Yongci/yongci.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/Source/Yongci" 
	@${RM} ${OBJECTDIR}/Source/Yongci/yongci.o.d 
	@${RM} ${OBJECTDIR}/Source/Yongci/yongci.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  Source/Yongci/yongci.c  -o ${OBJECTDIR}/Source/Yongci/yongci.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MMD -MF "${OBJECTDIR}/Source/Yongci/yongci.o.d"        -g -omf=elf -DXPRJ_default=$(CND_CONF)  -legacy-libc  $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off   -std=gnu99
	@${FIXDEPS} "${OBJECTDIR}/Source/Yongci/yongci.o.d" $(SILENT)  -rsi ${MP_CC_DIR}../ 
	
${OBJECTDIR}/Source/main.o: Source/main.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} "${OBJECTDIR}/Source" 
	@${RM} ${OBJECTDIR}/Source/main.o.d 
	@${RM} ${OBJECTDIR}/Source/main.o 
	${MP_CC} $(MP_EXTRA_CC_PRE)  Source/main.c  -o ${OBJECTDIR}/Source/main.o  -c -mcpu=$(MP_PROCESSOR_OPTION)  -MMD -MF "${OBJECTDIR}/Source/main.o.d"        -g -omf=elf -DXPRJ_default=$(CND_CONF)  -legacy-libc  $(COMPARISON_BUILD)  -O0 -msmart-io=1 -Wall -msfr-warn=off   -std=gnu99
	@${FIXDEPS} "${OBJECTDIR}/Source/main.o.d" $(SILENT)  -rsi ${MP_CC_DIR}../ 
	
endif

# ------------------------------------------------------------------------------------
# Rules for buildStep: assemble
ifeq ($(TYPE_IMAGE), DEBUG_RUN)
else
endif

# ------------------------------------------------------------------------------------
# Rules for buildStep: assemblePreproc
ifeq ($(TYPE_IMAGE), DEBUG_RUN)
else
endif

# ------------------------------------------------------------------------------------
# Rules for buildStep: link
ifeq ($(TYPE_IMAGE), DEBUG_RUN)
dist/${CND_CONF}/${IMAGE_TYPE}/YongciContronerCAN_V2.0.X.${IMAGE_TYPE}.${OUTPUT_SUFFIX}: ${OBJECTFILES}  nbproject/Makefile-${CND_CONF}.mk    
	@${MKDIR} dist/${CND_CONF}/${IMAGE_TYPE} 
	${MP_CC} $(MP_EXTRA_LD_PRE)  -o dist/${CND_CONF}/${IMAGE_TYPE}/YongciContronerCAN_V2.0.X.${IMAGE_TYPE}.${OUTPUT_SUFFIX}  ${OBJECTFILES_QUOTED_IF_SPACED}      -mcpu=$(MP_PROCESSOR_OPTION)        -D__DEBUG -D__MPLAB_DEBUGGER_PK3=1  -omf=elf -DXPRJ_default=$(CND_CONF)  -legacy-libc  $(COMPARISON_BUILD)   -mreserve=data@0x800:0x81F -mreserve=data@0x820:0x821 -mreserve=data@0x822:0x823 -mreserve=data@0x824:0x84F   -Wl,,,--defsym=__MPLAB_BUILD=1,--defsym=__MPLAB_DEBUG=1,--defsym=__DEBUG=1,--defsym=__MPLAB_DEBUGGER_PK3=1,$(MP_LINKER_FILE_OPTION),--stack=16,--check-sections,--data-init,--pack-data,--handles,--isr,--no-gc-sections,--fill-upper=0,--stackguard=16,--no-force-link,--smart-io,-Map="${DISTDIR}/${PROJECTNAME}.${IMAGE_TYPE}.map",--report-mem,--memorysummary,dist/${CND_CONF}/${IMAGE_TYPE}/memoryfile.xml$(MP_EXTRA_LD_POST) 
	
else
dist/${CND_CONF}/${IMAGE_TYPE}/YongciContronerCAN_V2.0.X.${IMAGE_TYPE}.${OUTPUT_SUFFIX}: ${OBJECTFILES}  nbproject/Makefile-${CND_CONF}.mk   
	@${MKDIR} dist/${CND_CONF}/${IMAGE_TYPE} 
	${MP_CC} $(MP_EXTRA_LD_PRE)  -o dist/${CND_CONF}/${IMAGE_TYPE}/YongciContronerCAN_V2.0.X.${IMAGE_TYPE}.${DEBUGGABLE_SUFFIX}  ${OBJECTFILES_QUOTED_IF_SPACED}      -mcpu=$(MP_PROCESSOR_OPTION)        -omf=elf -DXPRJ_default=$(CND_CONF)  -legacy-libc  $(COMPARISON_BUILD)  -Wl,,,--defsym=__MPLAB_BUILD=1,$(MP_LINKER_FILE_OPTION),--stack=16,--check-sections,--data-init,--pack-data,--handles,--isr,--no-gc-sections,--fill-upper=0,--stackguard=16,--no-force-link,--smart-io,-Map="${DISTDIR}/${PROJECTNAME}.${IMAGE_TYPE}.map",--report-mem,--memorysummary,dist/${CND_CONF}/${IMAGE_TYPE}/memoryfile.xml$(MP_EXTRA_LD_POST) 
	${MP_CC_DIR}\\xc16-bin2hex dist/${CND_CONF}/${IMAGE_TYPE}/YongciContronerCAN_V2.0.X.${IMAGE_TYPE}.${DEBUGGABLE_SUFFIX} -a  -omf=elf  
	
endif


# Subprojects
.build-subprojects:


# Subprojects
.clean-subprojects:

# Clean Targets
.clean-conf: ${CLEAN_SUBPROJECTS}
	${RM} -r build/default
	${RM} -r dist/default

# Enable dependency checking
.dep.inc: .depcheck-impl

DEPFILES=$(shell mplabwildcard ${POSSIBLE_DEPFILES})
ifneq (${DEPFILES},)
include ${DEPFILES}
endif
