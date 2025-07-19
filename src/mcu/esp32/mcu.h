/*
 * MCUGotchi - A Tamagotchi P1 emulator for microcontrollers
 *
 * Copyright (C) 2021 Jean-Christophe Rona <jc@rona.fr>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */
#ifndef _MCU_H_
#define _MCU_H_

#define MCU_TIME_FREQ_NUM					128ULL
#define MCU_TIME_FREQ_DEN					15625ULL

#define STORAGE_BASE_ADDRESS					0x0
#define STORAGE_SIZE						0x100000
#define STORAGE_PAGE_SIZE					32

#define STORAGE_ROM_OFFSET					0x0
#define STORAGE_ROM_SIZE					0xC00

#define STORAGE_FS_OFFSET					0xC00
#define STORAGE_FS_SIZE						0x4000

#define ENTER_SLEEP_S1_LATENCY					5
#define EXIT_SLEEP_S1_LATENCY					2

#define ENTER_SLEEP_S2_LATENCY					10
#define EXIT_SLEEP_S2_LATENCY					5

#define ENTER_SLEEP_S3_LATENCY					20
#define EXIT_SLEEP_S3_LATENCY					10

#define HIGHEST_ALLOWED_STATE					STATE_SLEEP_S2

#define DEVICE_ID1						(0x0)
#define DEVICE_ID2						(0x0)
#define DEVICE_ID3						(0x0)

#endif /* _MCU_H_ */