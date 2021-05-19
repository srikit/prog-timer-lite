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

#ifndef DATA_CONV_MODULE_H
#define DATA_CONV_MODULE_H

unsigned char strToInt(unsigned char* data);
void IntToStr(unsigned char num, unsigned char *out);

void memClear(unsigned char *dataBuffer, unsigned char bufferLen);
void byteCopy(unsigned char *dest, unsigned char *src, unsigned char srcOffset);

#endif /* DATA_CONV_MODULE_H */