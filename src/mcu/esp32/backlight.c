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
#include <stdint.h>
#include "driver/ledc.h"

#include "board.h"
#include "backlight.h"

#define LEDC_TIMER_BL			LEDC_TIMER_2
#define LEDC_MODE_BL			LEDC_LOW_SPEED_MODE
#define LEDC_CHANNEL_BL			LEDC_CHANNEL_4
#define LEDC_DUTY_RES_BL		LEDC_TIMER_8_BIT
#define LEDC_FREQUENCY_BL		(5000)

void backlight_init(void)
{
	ledc_timer_config_t ledc_timer = {
		.duty_resolution = LEDC_DUTY_RES_BL,
		.freq_hz = LEDC_FREQUENCY_BL,
		.speed_mode = LEDC_MODE_BL,
		.timer_num = LEDC_TIMER_BL,
		.clk_cfg = LEDC_AUTO_CLK,
	};
	ESP_ERROR_CHECK(ledc_timer_config(&ledc_timer));

	ledc_channel_config_t ledc_channel = {
		.channel    = LEDC_CHANNEL_BL,
		.duty       = 0,
		.gpio_num   = BOARD_SCREEN_BL_PIN,
		.speed_mode = LEDC_MODE_BL,
		.hpoint     = 0,
		.timer_sel  = LEDC_TIMER_BL
	};
	ESP_ERROR_CHECK(ledc_channel_config(&ledc_channel));
}

void backlight_set_level(uint8_t level)
{
	ESP_ERROR_CHECK(ledc_set_duty(LEDC_MODE_BL, LEDC_CHANNEL_BL, level));
	ESP_ERROR_CHECK(ledc_update_duty(LEDC_MODE_BL, LEDC_CHANNEL_BL));
}

void backlight_set(uint8_t v)
{
	backlight_set_level(v);
}