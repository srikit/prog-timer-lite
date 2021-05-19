//-----------------------------------------------------------------------------
// Programmable Timer - Single Channel - Lite Version.
// Definitions for serial communication protocol (Response).
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

#ifndef UART_PROG_RESPONSE_DEF_H
#define UART_PROG_RESPONSE_DEF_H

// Header values.
#define RESP_HEADER                  0x2A    // Response header starts with '*'
#define RESP_DEVICE_HEADER           0x54    // Use 'T' as device header for this firmware.

// Command IDs.
#define RESP_CMD_SET_ALARM_START     0x53    // Set alarm start time - 'S'
#define RESP_CMD_SET_ALARM_END       0x45    // Set alarm end time - 'E'

#define RESP_CMD_GET_ALARM_START     0x42    // Get alarm start time - 'B'
#define RESP_CMD_GET_ALARM_END       0x57    // Get alarm end time - 'W'

#define RESP_CMD_GET_DEVICE_INFO     0x49    // Get device information - 'I'
#define RESP_CMD_HANDSHAKE           0x48    // Communication handshake - 'H'

#define RESP_CMD_SET_SYS_TIME        0x43    // Set system time - 'C'
#define RESP_CMD_GET_SYS_TIMR        0x47    // Get system time - 'G'

#define RESP_CMD_SET_TIMER_COUNT     0x4E    // Set number of active alarms - 'N'
#define RESP_CMD_GET_TIMER_COUNT     0x58    // Get number of active alarms - 'X'

#endif /* UART_PROG_RESPONSE_DEF_H */