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

#ifndef UART_PROG_TIMER_MODULE_H
#define UART_PROG_TIMER_MODULE_H

#include "ds3231rtc.h"

typedef struct AlarmInfo
{
    DateTimeInfo start;
    DateTimeInfo end;
    unsigned char channel;
} AlarmInfo;

// Number of output channels available in this module.
#define MAX_CHANNEL_COUNT   1

// Number of software timers available in this module.
#define MAX_TIMER_COUNT     8

// Base year which is used in system operations to identify <NOT-DEFINED> flag.
#define NULL_YEAR    19

unsigned char dataPos, dataVal, cmdLen;
unsigned char exeCommand;
unsigned char alarmCount;

DateTimeInfo sysTime;

void initSystem();
unsigned char getCommandLen(unsigned char cmd);
void executeCommand();
void saveTimeInfo(unsigned short memAddr, unsigned char bufferOffset);
void readTimeInfo(unsigned short memAddr);
void updateSysTime();
void getAlarmConfig(AlarmInfo *info, unsigned char alarmPos);
unsigned char IsTimeHigh(DateTimeInfo * baseTime, DateTimeInfo * targetTime);

#endif /* UART_PROG_TIMER_MODULE_H */