//-----------------------------------------------------------------------------
// Programmable Timer - Single Channel - Lite Version.
// DS3231 based real time clock driver.
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

#ifndef DS3231_RTC_MODULE_H
#define DS3231_RTC_MODULE_H

#define DS3231_I2C_READ_ADDR    0xD1
#define DS3231_I2C_WRITE_ADDR   0xD0

#define DS3231_TIME_ADDR        0x00

typedef struct DateTimeInfo
{
    unsigned char year;
    unsigned char month;
    unsigned char date;

    unsigned char hour;
    unsigned char minute;
    unsigned char second;
} DateTimeInfo;

void initRTC();

void getDateTime(DateTimeInfo* info);
void setDateTime(DateTimeInfo* info);

#endif /* DS3231_RTC_MODULE_H */