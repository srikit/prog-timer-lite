//-----------------------------------------------------------------------------
// Programmable Timer - Single Channel - Lite Version.
// Main source file.
// 
// Copyright (C) 2020 SriKIT contributors.
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.
// 
// Last updated: Dilshan Jayakody [24th Nov 2020]
//
// Update log:
// [24/11/2020] - Initial version - Dilshan Jayakody.
//-----------------------------------------------------------------------------

#include "include/stm8.h"
#include "include/stm8-util.h"
#include "include/stm8-uart.h"
#include "include/stm8-eeprom.h"

#include "main.h"
#include "commdata.h"
#include "commresponse.h"
#include "dataconv.h"

// Startup delay limit.
#define INIT_RESET_DELAY    157456

// Size of the input command buffer.
#define DATA_BUFFER_SIZE        19
#define EEPROM_DATA_BLOCK_SIZE  16

#define COMMAND_AVAILABLE   1
#define COMMAND_NOT_FOUND   0

#define ALARM_ON_OUT    0x38
#define ALARM_OFF_OUT   0x00

unsigned char dataBuffer[DATA_BUFFER_SIZE];

void UART_received() __interrupt(UART1_RX_IRQ)
{
    if(uartDataAvailable())
    {
        // UART data is available.  
        dataVal = uartGetByte();  

        // The first byte of the command must be '#', else ignore the command.
        if((dataPos == 0) && (dataVal != COM_HEADER)) 
        {     
            return;
        }

        // Check for valid device command.
        if((dataPos == 1) && (dataVal != COM_DEVICE_HEADER)) 
        {     
            dataPos = 0;
            return;
        }

        // Identify the command type and data length.
        if(dataPos == 2)
        {
            cmdLen = getCommandLen(dataVal);
            if(cmdLen == 0)
            {
                // Unknown command, reset command buffer.
                dataPos = 0;
                return;
            }
        }

        // Update data buffer with received data.
        dataBuffer[dataPos] = dataVal;
        dataPos++; 

        // Check complete command is received to the buffer.
        if((cmdLen > 2) && (dataPos >= cmdLen))
        {
            // Full command is received. Raise the flag!
            exeCommand = COMMAND_AVAILABLE;
        }
    }
}

void main()
{
    // Wait few seconds to provide slot to enter programming / debug mode.
    unsigned long resetTime = 0;
    unsigned char alarmConfigPos, alarmOutput, startIsHigh;
    AlarmInfo currentAlarmConfig;

    for(resetTime = 0; resetTime < INIT_RESET_DELAY; resetTime++); 

    cli();

    // Start device boot.
    initSystem();
    initRTC();
    memClear(dataBuffer, DATA_BUFFER_SIZE);

    // Initialize serial interface with 9600 - 8N1 configuration.
    uartInit();

    // Get system time from RTC.
    getDateTime(&sysTime);

    // Get total number of alarm configurations from EEPROM.
    alarmCount = eepromRead(EEPROM_START_ADDR);

    // Enable UART RX interrupt.
    sei();
    UART1_CR2 |= UART1_CR2_RIEN;

    // Main service loop.
    while(1)
    {
        // UART command processor.
        if(exeCommand)
        {
            // Disable UART RX interrupt while at the command execution.
            UART1_CR2 &= ~UART1_CR2_RIEN;
            
            // Clear command received flag and execute the current command.
            exeCommand = COMMAND_NOT_FOUND;
            executeCommand();

            // Clear command buffer.
            memClear(dataBuffer, DATA_BUFFER_SIZE);
            dataPos = cmdLen = 0;

            // Enable UART RX interrupt.
            UART1_CR2 |= UART1_CR2_RIEN;
        }

        // Get system time from RTC.
        getDateTime(&sysTime);

        // Check for alarm on/off times.
        if(alarmCount > 0)
        {
            alarmConfigPos = 0;
            alarmOutput = 0;
            startIsHigh = 0;

            while(alarmConfigPos < alarmCount)
            {
                getAlarmConfig(&currentAlarmConfig, alarmConfigPos);
                            
                // Check if the date component is disable / enable with this configuration.
                if((currentAlarmConfig.start.year == NULL_YEAR) && (currentAlarmConfig.start.month == 0x00) && (currentAlarmConfig.start.date == 0x00))
                {
                    // Date component is disable in the current alarm configuration.

                    if(currentAlarmConfig.start.hour == currentAlarmConfig.end.hour)
                    {                        
                        if(currentAlarmConfig.start.minute == currentAlarmConfig.end.minute)
                        {
                            startIsHigh = (currentAlarmConfig.start.second > currentAlarmConfig.end.second);
                        }
                        else
                        {
                            startIsHigh = (currentAlarmConfig.start.minute > currentAlarmConfig.end.minute);
                        }
                    }
                    else
                    {
                        startIsHigh = (currentAlarmConfig.start.hour > currentAlarmConfig.end.hour);
                    }
                    
                    // Check for alarm start condition.
                    alarmOutput = IsTimeHigh(&sysTime, &currentAlarmConfig.start);
                    
                    if(startIsHigh)
                    {
                        // Current alarm configuration moves into the next day.
                        if(alarmOutput == 0)
                        {
                            // Check for alarm start conditions in next day.
                            if(sysTime.hour == currentAlarmConfig.end.hour)
                            {
                                if(sysTime.minute == currentAlarmConfig.end.minute)
                                {
                                    alarmOutput = (sysTime.second <= currentAlarmConfig.end.second);
                                }
                                else if(sysTime.minute < currentAlarmConfig.end.minute)
                                {
                                    alarmOutput = 1;
                                }
                            }
                            else if(sysTime.hour < currentAlarmConfig.end.hour)
                            {
                                alarmOutput = 1;
                            }
                        }                        
                    }
                    else
                    {
                        // Both alarm on and off times are in same day.
                        if(alarmOutput)
                        {
                            if(sysTime.hour == currentAlarmConfig.end.hour)
                            {
                                if(sysTime.minute == currentAlarmConfig.end.minute)
                                {
                                    alarmOutput = (sysTime.second < currentAlarmConfig.end.second);
                                }
                                else if(sysTime.minute > currentAlarmConfig.end.minute)
                                {
                                    alarmOutput = 0;
                                }
                            }
                            else if(sysTime.hour > currentAlarmConfig.end.hour)
                            {
                                alarmOutput = 0;
                            }
                        }
                    }                
                }
                else
                {
                    // Both date and time components are enabled in the current alarm configuration.

                    if((currentAlarmConfig.start.year == sysTime.year) && (currentAlarmConfig.start.month == sysTime.month) && (currentAlarmConfig.start.date == sysTime.date))
                    {
                        // Alarm start date is reached. Check for alarm start time.                        
                        alarmOutput = IsTimeHigh(&sysTime, &currentAlarmConfig.start);
                        
                        // Check for alarm end time to shutdown the alarm output.
                        if((alarmOutput) && ((currentAlarmConfig.end.year == sysTime.year) && (currentAlarmConfig.end.month == sysTime.month) && (currentAlarmConfig.end.date == sysTime.date)))
                        {
                            if(sysTime.hour == currentAlarmConfig.end.hour)
                            {
                                if(currentAlarmConfig.end.minute == sysTime.minute)
                                {
                                    alarmOutput = (sysTime.second < currentAlarmConfig.end.second);
                                }
                                else
                                {
                                    alarmOutput = (sysTime.minute < currentAlarmConfig.end.minute);
                                }
                            }
                            else
                            {
                                alarmOutput = (sysTime.hour < currentAlarmConfig.end.hour);
                            }
                        }
                    }
                    else if((currentAlarmConfig.end.year == sysTime.year) && (currentAlarmConfig.end.month == sysTime.month) && (currentAlarmConfig.end.date == sysTime.date))
                    {
                        // Alarm end date is reached. Check for alarm stop time.
                        if(currentAlarmConfig.end.hour == sysTime.hour)
                        {
                            if(currentAlarmConfig.end.minute == sysTime.minute)
                            {
                                alarmOutput = (currentAlarmConfig.end.second > sysTime.second);
                            }
                            else if(currentAlarmConfig.end.minute > sysTime.minute)
                            {
                                alarmOutput = 1;
                            }
                        }
                        else if(currentAlarmConfig.end.hour > sysTime.hour)
                        {
                            alarmOutput = 1;
                        }
                    }
                    else
                    {
                        // Check if the current date is inbetween start and end days.
                        if(sysTime.year == currentAlarmConfig.start.year)
                        {
                            if(sysTime.month == currentAlarmConfig.start.month)
                            {
                                alarmOutput = (sysTime.date >= currentAlarmConfig.start.date);
                            }
                            else if(sysTime.month > currentAlarmConfig.start.month)
                            {
                                alarmOutput = 1;
                            }
                        }
                        else if(sysTime.year > currentAlarmConfig.start.year)
                        {
                            alarmOutput = 1;
                        }
                        
                        // Check for end date.
                        if(alarmOutput)
                        {
                            if(sysTime.year == currentAlarmConfig.end.year)
                            {
                                if(sysTime.month == currentAlarmConfig.end.month)
                                {
                                    alarmOutput = (sysTime.date < currentAlarmConfig.end.date);
                                }
                                else if(sysTime.month > currentAlarmConfig.end.month)
                                {
                                    alarmOutput = 0;
                                }
                            }
                            else if(sysTime.year > currentAlarmConfig.end.year)
                            {
                                alarmOutput = 0;
                            }
                        }
                    }
                }

                // Check for alarm active flag to avoid multipal condition conflits.    
                if(alarmOutput)
                {
                    // Continue alarm configuration only if the alarm output is OFF.
                    break;
                }

                // Move to next alarm configuration.
                alarmConfigPos++;
            }

            // Activate alarm output terminal based on the output flag.
            PC_ODR = (alarmOutput ? ALARM_ON_OUT : ALARM_OFF_OUT);
        }
    }
}

unsigned char getCommandLen(unsigned char cmd)
{
    switch (cmd)
    {
    case COM_CMD_HANDSHAKE:         // Shakehand.
        return COM_LEN_HANDSHAKE;
    case COM_CMD_GET_DEVICE_INFO:   // Get device information.
        return COM_LEN_GET_DEVICE_INFO;
    case COM_CMD_SET_ALARM_START:   // Set alarm start time.
        return COM_LEN_SET_ALARM_START;
    case COM_CMD_SET_ALARM_END:     // Set alarm end time.
        return COM_LEN_SET_ALARM_END;
    case COM_CMD_GET_ALARM_START:   // Get alarm start time.
        return COM_LEN_GET_ALARM_START;
    case COM_CMD_GET_ALARM_END:     // Get alarm end time.
        return COM_LEN_GET_ALARM_END;
    case COM_CMD_SET_SYS_TIME:      // Set system time.
        return COM_LEN_SET_SYS_TIME;
    case COM_CMD_GET_SYS_TIMR:      // Get system time.
        return COM_LEN_GET_SYS_TIME;
    case COM_CMD_SET_TIMER_COUNT:   // Set active alarm count.
        return COM_LEN_SET_TIMER_COUNT;
    case COM_CMD_GET_TIMER_COUNT:   // Get active alarm count.
        return COM_LEN_GET_TIMER_COUNT;
    default:                        // Unknown command and reset the data buffer.
        return 0;
    }
}

void initSystem()
{
    // Setup alarm output terminal.
    PC_DDR = 0x38;
    PC_CR1 = 0x38;
    PC_CR2 = 0x00;
    PC_ODR = ALARM_OFF_OUT;

    // Initialize global variables.
    dataPos = 0;
    cmdLen = 0;
    dataVal = 0;
    exeCommand = 0;
}

unsigned char IsTimeHigh(DateTimeInfo * baseTime, DateTimeInfo * targetTime)
{
    if(baseTime->hour == targetTime->hour)
    {
       if(baseTime->minute == targetTime->minute)
       {
           if(baseTime->second >= targetTime->second)
           {
               return 1;
           }
       }
       else if(baseTime->minute > targetTime->minute)
       {
           return 1;
       }
    }
    else if(baseTime->hour > targetTime->hour)
    {
       return 1;
    }
    
    return 0;
}

void getAlarmConfig(AlarmInfo *info, unsigned char alarmPos)
{
    unsigned short startAddr = EEPROM_START_ADDR + 1 + (EEPROM_DATA_BLOCK_SIZE * alarmPos);

    // Get alarm start configuration.
    info->start.year = eepromRead(startAddr) + NULL_YEAR;
    info->start.month = eepromRead(startAddr + 0x01);
    info->start.date = eepromRead(startAddr + 0x02);
    info->start.hour = eepromRead(startAddr + 0x03);
    info->start.minute = eepromRead(startAddr + 0x04);
    info->start.second = eepromRead(startAddr + 0x05);

    // Get alarm end configuration
    info->end.year = eepromRead(startAddr + 0x06) + NULL_YEAR;
    info->end.month = eepromRead(startAddr + 0x07);
    info->end.date = eepromRead(startAddr + 0x08);
    info->end.hour = eepromRead(startAddr + 0x09);
    info->end.minute = eepromRead(startAddr + 0x0A);
    info->end.second = eepromRead(startAddr + 0x0B);

    // Get channel number from configuration
    info->channel = eepromRead(startAddr + 0x0C);
}

void saveTimeInfo(unsigned short memAddr, unsigned char bufferOffset)
{
    unsigned char strCharBuffer[2];
    unsigned char compVal;

    // Reference: eeprom.txt

    // Extract value of the year.
    byteCopy(strCharBuffer, dataBuffer, bufferOffset); 
    compVal = strToInt(strCharBuffer);
    eepromWrite(memAddr, compVal);          // XY

    // Extract value of the month.
    byteCopy(strCharBuffer, dataBuffer, bufferOffset + 2); 
    compVal = strToInt(strCharBuffer);
    eepromWrite(memAddr + 1, compVal);      // XM

    // Extract value of the date.
    byteCopy(strCharBuffer, dataBuffer, bufferOffset + 4); 
    compVal = strToInt(strCharBuffer);
    eepromWrite(memAddr + 2, compVal);      // XD

    // Extract value of the hour.
    byteCopy(strCharBuffer, dataBuffer, bufferOffset + 6); 
    compVal = strToInt(strCharBuffer);
    eepromWrite(memAddr + 3, compVal);      // XH

    // Extract value of the minute.
    byteCopy(strCharBuffer, dataBuffer, bufferOffset + 8); 
    compVal = strToInt(strCharBuffer);
    eepromWrite(memAddr + 4, compVal);      // XN

    // Extract value of the seconds.
    byteCopy(strCharBuffer, dataBuffer, bufferOffset + 10); 
    compVal = strToInt(strCharBuffer);
    eepromWrite(memAddr + 5, compVal);      // XS
}

void readTimeInfo(unsigned short memAddr)
{
    unsigned char strCharBuffer[2];
    unsigned char tmpVal;
    
    // Extract year and send it via UART.
    tmpVal = eepromRead(memAddr);
    IntToStr(tmpVal, strCharBuffer);
    uartWrite(strCharBuffer[0]);
    uartWrite(strCharBuffer[1]);

    // Extract month and send it via UART.
    tmpVal = eepromRead(memAddr + 1);
    IntToStr(tmpVal, strCharBuffer);
    uartWrite(strCharBuffer[0]);
    uartWrite(strCharBuffer[1]);

    // Extract date and send it via UART.
    tmpVal = eepromRead(memAddr + 2);
    IntToStr(tmpVal, strCharBuffer);
    uartWrite(strCharBuffer[0]);
    uartWrite(strCharBuffer[1]);

    // Extract hour and send it via UART.
    tmpVal = eepromRead(memAddr + 3);
    IntToStr(tmpVal, strCharBuffer);
    uartWrite(strCharBuffer[0]);
    uartWrite(strCharBuffer[1]);

    // Extract minute and send it via UART.
    tmpVal = eepromRead(memAddr + 4);
    IntToStr(tmpVal, strCharBuffer);
    uartWrite(strCharBuffer[0]);
    uartWrite(strCharBuffer[1]);

    // Extract second and send it via UART.
    tmpVal = eepromRead(memAddr + 5);
    IntToStr(tmpVal, strCharBuffer);
    uartWrite(strCharBuffer[0]);
    uartWrite(strCharBuffer[1]);
}

void updateSysTime()
{
    unsigned char strCharBuffer[2];
    DateTimeInfo tempSysTime;

    // Get year from the input buffer.
    byteCopy(strCharBuffer, dataBuffer, 3); 
    tempSysTime.year = strToInt(strCharBuffer);
    
    // Get month from the input buffer.
    byteCopy(strCharBuffer, dataBuffer, 5); 
    tempSysTime.month = strToInt(strCharBuffer);

    // Get date from the input buffer.
    byteCopy(strCharBuffer, dataBuffer, 7); 
    tempSysTime.date = strToInt(strCharBuffer);

    // Get hour from the input buffer.
    byteCopy(strCharBuffer, dataBuffer, 9); 
    tempSysTime.hour = strToInt(strCharBuffer);

    // Get minute from the input buffer.
    byteCopy(strCharBuffer, dataBuffer, 11); 
    tempSysTime.minute = strToInt(strCharBuffer);

    // Get seconds from the input buffer.
    byteCopy(strCharBuffer, dataBuffer, 13); 
    tempSysTime.second = strToInt(strCharBuffer);

    // Update RTC time.
    setDateTime(&tempSysTime);
}

void executeCommand()
{
    unsigned char strCharBuffer[2];
    unsigned char tmpVal1, tmpVal2;

    // Reference: communication_protocol.txt
    switch(dataBuffer[2])
    {
    case COM_CMD_HANDSHAKE:         // Shakehand.
        uartWrite(RESP_HEADER);         // Handshake response [0] : Response Header
        uartWrite(RESP_DEVICE_HEADER);  // Handshake response [1] : Device Header
        uartWrite(RESP_CMD_HANDSHAKE);  // Handshake response [2] : Command ID
        break;
    case COM_CMD_GET_DEVICE_INFO:   // Get device information.
        uartWrite(RESP_HEADER);                 // Device info response [0] : Response Header
        uartWrite(RESP_DEVICE_HEADER);          // Device info response [1] : Device Header
        uartWrite(RESP_CMD_GET_DEVICE_INFO);    // Device info response [2] : Command ID
        uartWrite(0x30);                        // Device info response [3] : Number of Channels #1
        uartWrite(0x30 + MAX_CHANNEL_COUNT);    // Device info response [4] : Number of Channels #2
        uartWrite(0x30);                        // Device info response [5] : Timer Count #1
        uartWrite(0x30 + MAX_TIMER_COUNT);      // Device info response [6] : Timer Count #2
        break;    
    case COM_CMD_SET_ALARM_START:   // Set alarm start time.
        // Get alarm ID and calculate EEPROM address offset.
        byteCopy(strCharBuffer, dataBuffer, 3);     
        tmpVal1 = (strToInt(strCharBuffer) * EEPROM_DATA_BLOCK_SIZE) + 1;

        // Get channel ID.
        byteCopy(strCharBuffer, dataBuffer, 5);
        tmpVal2 = strToInt(strCharBuffer);

        // Save start time into EEPROM block.
        saveTimeInfo((unsigned short)(EEPROM_START_ADDR + tmpVal1), 7);

        // Save channel number into EEPROM block.        
        eepromWrite((unsigned short)(EEPROM_START_ADDR + tmpVal1 + 0x0C), tmpVal2);

        // Send acknowledge to the host terminal.
        uartWrite(RESP_HEADER);                 // Set alarm start time response [0] : Response Header
        uartWrite(RESP_DEVICE_HEADER);          // Set alarm start time response [1] : Device Header
        uartWrite(RESP_CMD_SET_ALARM_START);    // Set alarm start time response [2] : Command ID
        break;
    case COM_CMD_SET_ALARM_END:     // Set alarm end time.
        // Get alarm ID and calculate EEPROM address offset.
        byteCopy(strCharBuffer, dataBuffer, 3);     
        tmpVal1 = (strToInt(strCharBuffer) * EEPROM_DATA_BLOCK_SIZE) + 1;

        // Save end time into EEPROM block.
        saveTimeInfo((unsigned short)(EEPROM_START_ADDR + tmpVal1 + 6), 5);

        // Send acknowledge to the host terminal.
        uartWrite(RESP_HEADER);                 // Set alarm end time response [0] : Response Header
        uartWrite(RESP_DEVICE_HEADER);          // Set alarm end time response [1] : Device Header
        uartWrite(RESP_CMD_SET_ALARM_END);      // Set alarm end time response [2] : Command ID
        break;
    case COM_CMD_GET_ALARM_START:   // Get alarm start time.
        // Get alarm ID and calculate EEPROM address offset.
        byteCopy(strCharBuffer, dataBuffer, 3);     
        tmpVal1 = (strToInt(strCharBuffer) * EEPROM_DATA_BLOCK_SIZE) + 1;

        uartWrite(RESP_HEADER);                 // Get alarm start time response [0] : Response Header
        uartWrite(RESP_DEVICE_HEADER);          // Get alarm start time response [1] : Device Header
        uartWrite(RESP_CMD_GET_ALARM_START);    // Get alarm start time response [2] : Command ID

        // Get channel number from EEPROM.
        tmpVal2 = eepromRead(EEPROM_START_ADDR + tmpVal1 + 0x0C);
        IntToStr(tmpVal2, strCharBuffer);
        uartWrite(strCharBuffer[0]);            // Get alarm start time response [3] : Channel ID
        uartWrite(strCharBuffer[1]);            // Get alarm start time response [4] : Channel ID

        // Load start time from EEPROM and send it.
        readTimeInfo((unsigned short)(EEPROM_START_ADDR + tmpVal1));    // Get alarm start time response [5..16] : Date/Time
        break;
    case COM_CMD_GET_ALARM_END:     // Get alarm end time.
        // Get alarm ID and calculate EEPROM address offset.
        byteCopy(strCharBuffer, dataBuffer, 3);     
        tmpVal1 = (strToInt(strCharBuffer) * EEPROM_DATA_BLOCK_SIZE) + 1;

        uartWrite(RESP_HEADER);                 // Get alarm end time response [0] : Response Header
        uartWrite(RESP_DEVICE_HEADER);          // Get alarm end time response [1] : Device Header
        uartWrite(RESP_CMD_GET_ALARM_END);      // Get alarm end time response [2] : Command ID

        // Load end time from EEPROM and send it.
        readTimeInfo((unsigned short)(EEPROM_START_ADDR + tmpVal1 + 6));    // Get alarm start time response [3..14] : Date/Time
        break;
    case COM_CMD_SET_SYS_TIME:      // Set system time.
        updateSysTime(); 

        uartWrite(RESP_HEADER);                 // Set system time response [0] : Response Header
        uartWrite(RESP_DEVICE_HEADER);          // Set system time response [1] : Device Header
        uartWrite(RESP_CMD_SET_SYS_TIME);       // Set system time response [2] : Command ID
        break;
    case COM_CMD_GET_SYS_TIMR:      // Get system time.
        uartWrite(RESP_HEADER);                 // Get system time response [0] : Response Header
        uartWrite(RESP_DEVICE_HEADER);          // Get system time response [1] : Device Header
        uartWrite(RESP_CMD_GET_SYS_TIMR);       // Get system time response [2] : Command ID

        // Send current date component.
        IntToStr(sysTime.year, strCharBuffer);
        uartWrite(strCharBuffer[0]);            // Get system time response [3] : Year
        uartWrite(strCharBuffer[1]);            // Get system time response [4] : Year

        IntToStr(sysTime.month, strCharBuffer);
        uartWrite(strCharBuffer[0]);            // Get system time response [5] : Month
        uartWrite(strCharBuffer[1]);            // Get system time response [6] : Month

        IntToStr(sysTime.date, strCharBuffer);
        uartWrite(strCharBuffer[0]);            // Get system time response [7] : Date
        uartWrite(strCharBuffer[1]);            // Get system time response [8] : Date

        // Send current time component.
        IntToStr(sysTime.hour, strCharBuffer);
        uartWrite(strCharBuffer[0]);            // Get system time response [9] : Hour
        uartWrite(strCharBuffer[1]);            // Get system time response [10] : Hour

        IntToStr(sysTime.minute, strCharBuffer);
        uartWrite(strCharBuffer[0]);            // Get system time response [11] : Minute
        uartWrite(strCharBuffer[1]);            // Get system time response [12] : Minute

        IntToStr(sysTime.second, strCharBuffer);
        uartWrite(strCharBuffer[0]);            // Get system time response [13] : Seconds
        uartWrite(strCharBuffer[1]);            // Get system time response [14] : Seconds
        break;
    case COM_CMD_SET_TIMER_COUNT:   // Set active alarm count.
        // Get Alarm count from input buffer.
        byteCopy(strCharBuffer, dataBuffer, 3);     
        alarmCount = strToInt(strCharBuffer);
        eepromWrite(EEPROM_START_ADDR, alarmCount); 

        // Send acknowledge to the host terminal.
        uartWrite(RESP_HEADER);                 // Set active alarm count response [0] : Response Header
        uartWrite(RESP_DEVICE_HEADER);          // Set active alarm count response [1] : Device Header
        uartWrite(RESP_CMD_SET_TIMER_COUNT);    // Set active alarm count response [2] : Command ID
        break;
    case COM_CMD_GET_TIMER_COUNT:   // Get active alarm count.
        tmpVal1 = eepromRead(EEPROM_START_ADDR);
        IntToStr(tmpVal1, strCharBuffer);

        // Send alarm count response to the host.
        uartWrite(RESP_HEADER);                 // Get active alarm count response [0] : Response Header
        uartWrite(RESP_DEVICE_HEADER);          // Get active alarm count response [1] : Device Header
        uartWrite(RESP_CMD_GET_TIMER_COUNT);    // Get active alarm count response [2] : Command ID
        uartWrite(strCharBuffer[0]);            // Get active alarm count response [3] : Alarm Count
        uartWrite(strCharBuffer[1]);            // Get active alarm count response [4] : Alarm Count
        break;
    }
}