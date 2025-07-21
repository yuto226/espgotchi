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
#ifndef _BOARD_DEF_H_
#define _BOARD_DEF_H_

#define BOARD_SSD1306_I2C

#define BOARD_SCREEN_I2C_NUM		I2C_NUM_0
#define BOARD_SCREEN_I2C_SDA_PIN	26
#define BOARD_SCREEN_I2C_SCL_PIN	25
#define BOARD_SCREEN_RST_PIN		4
#define BOARD_SCREEN_RST_PORT		0
#define BOARD_SCREEN_BL_PIN		5

#define BOARD_LEFT_BTN_PIN		12
#define BOARD_MIDDLE_BTN_PIN		13
#define BOARD_RIGHT_BTN_PIN		14

#define BOARD_LED_RED_PIN		25
#define BOARD_LED_GREEN_PIN		26
#define BOARD_LED_BLUE_PIN		27

#define BOARD_SPEAKER_PIN		32

#define BOARD_VBATT_ANA_PIN		36
#define BOARD_VBATT_ANA_ADC_CHANNEL	ADC_CHANNEL_0
#define BOARD_VBATT_ANA_RATIO		2

#define BOARD_USB_DP_PIN		20
#define BOARD_USB_DM_PIN		19

#endif /* _BOARD_DEF_H_ */