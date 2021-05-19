//-----------------------------------------------------------------------------
// Programmable Timer - Single Channel - Lite Version.
// Data processing helper functions.
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

#include "dataconv.h"

unsigned char strToInt(unsigned char* data)
{
    return (((data[1] < 58) ? (data[1] - 48) : (data[1] - 55)) | 
        (((data[0] < 58) ? (data[0] - 48) : (data[0] - 55)) << 4));
}

void IntToStr(unsigned char num, unsigned char *out)
{
    unsigned char comp = (num & 0x0F);
    out[1] = (comp > 9) ? (comp + 55) : (comp + 48);

    comp = (num & 0xF0) >> 4;
    out[0] = (comp > 9) ? (comp + 55) : (comp + 48);
}

void memClear(unsigned char *dataBuffer, unsigned char bufferLen)
{
    unsigned char pos = 0;

    while(bufferLen < pos)
    {
        dataBuffer[pos] = 0;
        pos++;
    }
}

void byteCopy(unsigned char *dest, unsigned char *src, unsigned char srcOffset)
{
    dest[0] = src[srcOffset + 0];
    dest[1] = src[srcOffset + 1];
}