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
#ifndef _I2C_H_
#define _I2C_H_

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#define I2C_TIMEOUT_MS		100

void i2c_init(void);
bool i2c_write_byte(uint8_t device_addr, uint8_t data);
bool i2c_write_data(uint8_t device_addr, uint8_t *data, size_t length);
bool i2c_read_data(uint8_t device_addr, uint8_t *data, size_t length);
bool i2c_device_detect(uint8_t device_addr);

#endif /* _I2C_H_ */