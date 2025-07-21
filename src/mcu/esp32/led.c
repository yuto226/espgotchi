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
#include "led.h"

#define LEDC_TIMER_LED			LEDC_TIMER_1
#define LEDC_MODE_LED			LEDC_LOW_SPEED_MODE
#define LEDC_CH_RED				LEDC_CHANNEL_1
#define LEDC_CH_GREEN			LEDC_CHANNEL_2
#define LEDC_CH_BLUE			LEDC_CHANNEL_3
#define LEDC_DUTY_RES_LED		LEDC_TIMER_8_BIT
#define LEDC_FREQUENCY_LED		(5000)

void led_init(void)
{
	ledc_timer_config_t ledc_timer = {
		.duty_resolution = LEDC_DUTY_RES_LED,
		.freq_hz = LEDC_FREQUENCY_LED,
		.speed_mode = LEDC_MODE_LED,
		.timer_num = LEDC_TIMER_LED,
		.clk_cfg = LEDC_AUTO_CLK,
	};
	ESP_ERROR_CHECK(ledc_timer_config(&ledc_timer));

	ledc_channel_config_t ledc_channel_red = {
		.channel    = LEDC_CH_RED,
		.duty       = 0,
		.gpio_num   = BOARD_LED_RED_PIN,
		.speed_mode = LEDC_MODE_LED,
		.hpoint     = 0,
		.timer_sel  = LEDC_TIMER_LED
	};
	ESP_ERROR_CHECK(ledc_channel_config(&ledc_channel_red));

	ledc_channel_config_t ledc_channel_green = {
		.channel    = LEDC_CH_GREEN,
		.duty       = 0,
		.gpio_num   = BOARD_LED_GREEN_PIN,
		.speed_mode = LEDC_MODE_LED,
		.hpoint     = 0,
		.timer_sel  = LEDC_TIMER_LED
	};
	ESP_ERROR_CHECK(ledc_channel_config(&ledc_channel_green));

	ledc_channel_config_t ledc_channel_blue = {
		.channel    = LEDC_CH_BLUE,
		.duty       = 0,
		.gpio_num   = BOARD_LED_BLUE_PIN,
		.speed_mode = LEDC_MODE_LED,
		.hpoint     = 0,
		.timer_sel  = LEDC_TIMER_LED
	};
	ESP_ERROR_CHECK(ledc_channel_config(&ledc_channel_blue));
}

void led_set(uint8_t r, uint8_t g, uint8_t b)
{
	// TODO: 後ほど必要であれば
	// ESP_ERROR_CHECK(ledc_set_duty(LEDC_MODE_LED, LEDC_CH_RED, r));
	// ESP_ERROR_CHECK(ledc_update_duty(LEDC_MODE_LED, LEDC_CH_RED));

	// ESP_ERROR_CHECK(ledc_set_duty(LEDC_MODE_LED, LEDC_CH_GREEN, g));
	// ESP_ERROR_CHECK(ledc_update_duty(LEDC_MODE_LED, LEDC_CH_GREEN));

	// ESP_ERROR_CHECK(ledc_set_duty(LEDC_MODE_LED, LEDC_CH_BLUE, b));
	// ESP_ERROR_CHECK(ledc_update_duty(LEDC_MODE_LED, LEDC_CH_BLUE));
}