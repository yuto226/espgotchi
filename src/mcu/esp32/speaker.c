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
#include "driver/gpio.h"

#include "board.h"
#include "system.h"
#include "gpio.h"
#include "speaker.h"

#define LEDC_TIMER				LEDC_TIMER_0
#define LEDC_MODE				LEDC_LOW_SPEED_MODE
#define LEDC_CHANNEL			LEDC_CHANNEL_0
#define LEDC_DUTY_RES			LEDC_TIMER_13_BIT
#define LEDC_DUTY				(4095)
#define LEDC_FREQUENCY			(5000)

static uint8_t state_lock = 0;
static uint8_t speaker_enabled = 0;

void speaker_init(void)
{
	ledc_timer_config_t ledc_timer = {
		.duty_resolution = LEDC_DUTY_RES,
		.freq_hz = LEDC_FREQUENCY,
		.speed_mode = LEDC_MODE,
		.timer_num = LEDC_TIMER,
		.clk_cfg = LEDC_AUTO_CLK,
	};
	ESP_ERROR_CHECK(ledc_timer_config(&ledc_timer));

	ledc_channel_config_t ledc_channel = {
		.channel    = LEDC_CHANNEL,
		.duty       = 0,
		.gpio_num   = BOARD_SPEAKER_PIN,
		.speed_mode = LEDC_MODE,
		.hpoint     = 0,
		.timer_sel  = LEDC_TIMER
	};
	ESP_ERROR_CHECK(ledc_channel_config(&ledc_channel));
}

void speaker_set_frequency(uint32_t freq)
{
	if (freq > 0) {
		ESP_ERROR_CHECK(ledc_set_freq(LEDC_MODE, LEDC_TIMER, freq / 10));
	}
}

void speaker_enable(uint8_t en)
{
	if (en && !speaker_enabled) {
		ESP_ERROR_CHECK(ledc_set_duty(LEDC_MODE, LEDC_CHANNEL, LEDC_DUTY / 2));
		ESP_ERROR_CHECK(ledc_update_duty(LEDC_MODE, LEDC_CHANNEL));
		
		system_lock_max_state(STATE_SLEEP_S1, &state_lock);
		speaker_enabled = 1;
	} else if (!en && speaker_enabled) {
		ESP_ERROR_CHECK(ledc_set_duty(LEDC_MODE, LEDC_CHANNEL, 0));
		ESP_ERROR_CHECK(ledc_update_duty(LEDC_MODE, LEDC_CHANNEL));
		
		gpio_set_level(BOARD_SPEAKER_PIN, 0);
		
		system_unlock_max_state(STATE_SLEEP_S1, &state_lock);
		speaker_enabled = 0;
	}
}