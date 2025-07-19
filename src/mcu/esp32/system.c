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
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_pm.h"
#include "esp_sleep.h"
#include "esp_system.h"

#include "system.h"

static uint8_t state_lock_counters[STATE_NUM] = {0};

void system_disable_irq(void)
{
	taskDISABLE_INTERRUPTS();
}

void system_enable_irq(void)
{
	taskENABLE_INTERRUPTS();
}

void system_init(void)
{
#ifdef CONFIG_PM_ENABLE
	esp_pm_config_esp32_t pm_config = {
		.max_freq_mhz = 240,
		.min_freq_mhz = 80,
		.light_sleep_enable = true
	};
	ESP_ERROR_CHECK(esp_pm_configure(&pm_config));
#endif
}

void system_enter_state(exec_state_t state)
{
	switch (state) {
		case STATE_RUN:
			break;
		case STATE_SLEEP_S1:
			vTaskDelay(1);
			break;
		case STATE_SLEEP_S2:
			esp_sleep_enable_timer_wakeup(1000);
			esp_light_sleep_start();
			break;
		case STATE_SLEEP_S3:
			esp_sleep_enable_timer_wakeup(10000);
			esp_light_sleep_start();
			break;
		default:
			break;
	}
}

exec_state_t system_get_max_state(void)
{
	exec_state_t state;

	for (state = HIGHEST_ALLOWED_STATE; state > STATE_RUN; state--) {
		if (state_lock_counters[state] == 0) {
			return state;
		}
	}

	return STATE_RUN;
}

void system_lock_max_state(exec_state_t state, uint8_t *lock)
{
	if (*lock == 0) {
		state_lock_counters[state]++;
		*lock = 1;
	}
}

void system_unlock_max_state(exec_state_t state, uint8_t *lock)
{
	if (*lock == 1) {
		if (state_lock_counters[state] > 0) {
			state_lock_counters[state]--;
		}
		*lock = 0;
	}
}

void system_fatal_error(void)
{
	while (1) {
		vTaskDelay(1000 / portTICK_PERIOD_MS);
	}
}

void system_reset(void)
{
	esp_restart();
}

void system_dfu_reset(void)
{
	esp_restart();
}