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
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_timer.h"
#include "esp_pm.h"

#include "system.h"
#include "time.h"

static volatile uint64_t time_offset = 0;

void time_init(void)
{
	time_offset = esp_timer_get_time();
}

mcu_time_t time_get(void)
{
	uint64_t current_time = esp_timer_get_time() - time_offset;
	return (mcu_time_t)(current_time * MCU_TIME_FREQ_NUM / MCU_TIME_FREQ_DEN);
}

void time_wait_until(mcu_time_t time)
{
	while ((int32_t)(time - time_get()) > 0) {
		vTaskDelay(1);
	}
}

void time_delay(mcu_time_t time)
{
	time_wait_until(time_get() + time);
}

exec_state_t time_configure_wakeup(mcu_time_t time)
{
	mcu_time_t t = time_get();
	int32_t delta = time - t;
	exec_state_t max_state = system_get_max_state();
	exec_state_t state;

	if (delta < SLEEP_S1_THRESHOLD || max_state == STATE_RUN) {
		return STATE_RUN;
	} else if (delta < SLEEP_S2_THRESHOLD || max_state == STATE_SLEEP_S1) {
		state = STATE_SLEEP_S1;
	} else if (delta < SLEEP_S3_THRESHOLD || max_state == STATE_SLEEP_S2) {
		state = STATE_SLEEP_S2;
	} else {
		state = STATE_SLEEP_S2;
	}

	return state;
}