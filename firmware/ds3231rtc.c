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

#include "include/stm8.h"
#include "include/stm8-util.h"
#include "include/stm8-i2c.h"

#include "ds3231rtc.h"

void initRTC()
{
    i2cInit();
}

void getDateTime(DateTimeInfo* info)
{
    // Initialize read operation.
    i2cStart();
    i2cWriteAddr(DS3231_I2C_WRITE_ADDR);

    // Start reading from specified address.
    i2cWrite(DS3231_TIME_ADDR);
    i2cStop();

    // Get time information from the I2C link.
    i2cStart();
    i2cWriteAddr(DS3231_I2C_READ_ADDR);

    // Get time component from RTC.
    info->second = bcdToDec(i2cRead(1));
    info->minute = bcdToDec(i2cRead(1));
    info->hour = bcdToDec(i2cRead(1));

    // Ignore day component from RTC.
    i2cRead(1);

    // Get date time component from RTC.
    info->date = bcdToDec(i2cRead(1));
    info->month = bcdToDec(i2cRead(1));
    info->year = bcdToDec(i2cRead(0));

    i2cStop();
}

void setDateTime(DateTimeInfo* info)
{
    // Initialize write operation.
    i2cStart();
    i2cWriteAddr(DS3231_I2C_WRITE_ADDR);

    // Start reading from specified address.
    i2cWrite(DS3231_TIME_ADDR);

    // Write time component into RTC.
    i2cWrite(decToBCD(info->second));
    i2cWrite(decToBCD(info->minute));
    i2cWrite(decToBCD(info->hour));

    // Write date component into RTC.
    i2cWrite(0);
    i2cWrite(decToBCD(info->date));
    i2cWrite(decToBCD(info->month));
    i2cWrite(decToBCD(info->year));   

    i2cStop();
}