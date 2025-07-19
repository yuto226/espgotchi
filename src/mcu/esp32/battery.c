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
#include "driver/adc.h"
#include "esp_adc_cal.h"
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

static esp_adc_cal_characteristics_t *adc_chars;
static adc_channel_t channel = BOARD_VBATT_ANA_ADC_CHANNEL;
static adc_atten_t atten = ADC_ATTEN_DB_11;
static adc_unit_t unit = ADC_UNIT_1;

static void (*battery_cb)(uint16_t) = NULL;
static job_t battery_processing_job;
static uint8_t measurement_ongoing = 0;
static uint8_t state_lock = 0;

static uint32_t adc_readings[ADC_DATA_SIZE];

static void adc_init(void)
{
	adc1_config_width(ADC_WIDTH_BIT_12);
	adc1_config_channel_atten(channel, atten);

	adc_chars = calloc(1, sizeof(esp_adc_cal_characteristics_t));
	esp_adc_cal_value_t val_type = esp_adc_cal_characterize(unit, atten, ADC_WIDTH_BIT_12, DEFAULT_VREF, adc_chars);
}

static void adc_deinit(void)
{
	if (adc_chars) {
		free(adc_chars);
		adc_chars = NULL;
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
			adc_reading += adc1_get_raw((adc1_channel_t)channel);
		}
		adc_reading /= NO_OF_SAMPLES;
		adc_readings[i] = esp_adc_cal_raw_to_voltage(adc_reading, adc_chars);
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