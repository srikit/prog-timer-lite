//-----------------------------------------------------------------------------
// STM8S EEPROM library for standard mode communication.
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
// Last updated: Dilshan Jayakody [24th Oct 2020]
//
// Update log:
// [24/10/2020] - Initial version - Dilshan Jayakody.
//----------------------------------------------------------------------------- 

#ifndef HARDWARE_EEPROM_H
#define HARDWARE_EEPROM_H

#include "../include/stm8.h"

void eepromWrite(unsigned short addr, unsigned char value);

static inline unsigned char eepromRead(unsigned short addr)
{
    return GLOBAL_MEM(addr);
}

#endif /* HARDWARE_EEPROM_H */