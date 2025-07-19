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
#include "esp_adc/adc_oneshot.h"
#include "esp_adc/adc_cali.h"
#include "esp_adc/adc_cali_scheme.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "board.h"
#include "system.h"
#include "job.h"
#include "time.h"
#include "gpio.h"
#include "battery.h"

#define ADC_DATA_SIZE					32
#define DEFAULT_VREF					1100
#define NO_OF_SAMPLES					64

static adc_oneshot_unit_handle_t adc1_handle;
static adc_cali_handle_t adc1_cali_handle;
static adc_channel_t channel = BOARD_VBATT_ANA_ADC_CHANNEL;
static adc_atten_t atten = ADC_ATTEN_DB_11;
static bool adc_calibration_init = false;

static void (*battery_cb)(uint16_t) = NULL;
static job_t battery_processing_job;
static uint8_t measurement_ongoing = 0;
static uint8_t state_lock = 0;

static uint32_t adc_readings[ADC_DATA_SIZE];

static void adc_init(void)
{
	// ADC1 Init
	adc_oneshot_unit_init_cfg_t init_config1 = {
		.unit_id = ADC_UNIT_1,
	};
	ESP_ERROR_CHECK(adc_oneshot_new_unit(&init_config1, &adc1_handle));

	// ADC1 config
	adc_oneshot_chan_cfg_t config = {
		.bitwidth = ADC_BITWIDTH_12,
		.atten = atten,
	};
	ESP_ERROR_CHECK(adc_oneshot_config_channel(adc1_handle, channel, &config));

	// ADC1 Calibration Init
	adc_cali_curve_fitting_config_t cali_config = {
		.unit_id = ADC_UNIT_1,
		.atten = atten,
		.bitwidth = ADC_BITWIDTH_12,
	};
	ESP_ERROR_CHECK(adc_cali_create_scheme_curve_fitting(&cali_config, &adc1_cali_handle));
	adc_calibration_init = true;
}

static void adc_deinit(void)
{
	if (adc_calibration_init) {
		ESP_ERROR_CHECK(adc_cali_delete_scheme_curve_fitting(adc1_cali_handle));
		adc_calibration_init = false;
	}
	if (adc1_handle) {
		ESP_ERROR_CHECK(adc_oneshot_del_unit(adc1_handle));
		adc1_handle = NULL;
	}
}

void battery_init(void)
{
}

void battery_register_cb(void (*cb)(uint16_t))
{
	battery_cb = cb;
}

void battery_start_meas(void)
{
	if (measurement_ongoing) {
		return;
	}

	measurement_ongoing = 1;
	system_lock_max_state(STATE_SLEEP_S1, &state_lock);

	adc_init();

	for (int i = 0; i < ADC_DATA_SIZE; i++) {
		uint32_t adc_reading = 0;
		for (int j = 0; j < NO_OF_SAMPLES; j++) {
			int raw_value;
			ESP_ERROR_CHECK(adc_oneshot_read(adc1_handle, channel, &raw_value));
			adc_reading += raw_value;
		}
		adc_reading /= NO_OF_SAMPLES;
		int voltage;
		ESP_ERROR_CHECK(adc_cali_raw_to_voltage(adc1_cali_handle, adc_reading, &voltage));
		adc_readings[i] = voltage;
	}

	job_schedule(&battery_processing_job, &battery_processing_job_fn, JOB_ASAP);
}

void battery_stop_meas(void)
{
	if (!measurement_ongoing) {
		return;
	}

	adc_deinit();
	system_unlock_max_state(STATE_SLEEP_S1, &state_lock);
	measurement_ongoing = 0;
}

static void battery_processing_job_fn(job_t *job)
{
	uint32_t battery_v = 0;
	uint8_t i;

	for (i = 0; i < ADC_DATA_SIZE; i++) {
		battery_v += adc_readings[i];
	}

	battery_v = (battery_v * BOARD_VBATT_ANA_RATIO) / ADC_DATA_SIZE;

	battery_stop_meas();

	if (battery_cb != NULL) {
		battery_cb(battery_v);
	}
}