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
#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"

#include "job.h"
#include "board.h"
#include "gpio.h"
#include "input.h"

#define INPUT_NUM					5

#define DEBOUNCE_DURATION				100 //ms
#define LONG_PRESS_DURATION				1000 //ms

typedef struct {
	input_state_t state;
	job_t debounce_job;
	job_t long_press_job;
	gpio_pin_t pin;
	uint8_t long_press_enabled;
} input_data_t;

static input_data_t inputs[INPUT_NUM];
static QueueHandle_t gpio_evt_queue = NULL;
static void (*input_handler)(input_t, input_state_t, uint8_t) = NULL;

static void IRAM_ATTR gpio_isr_handler(void* arg)
{
	uint32_t gpio_num = (uint32_t) arg;
	xQueueSendFromISR(gpio_evt_queue, &gpio_num, NULL);
}

static input_state_t get_input_hw_state(input_t input)
{
	return (gpio_get(0, inputs[input].pin) ? INPUT_STATE_HIGH : INPUT_STATE_LOW);
}

static void long_press_job_fn(job_t *job);
static void debounce_job_fn(job_t *job);

static void gpio_task(void* arg)
{
	uint32_t io_num;
	input_t input = INPUT_BTN_LEFT;

	for(;;) {
		if(xQueueReceive(gpio_evt_queue, &io_num, portMAX_DELAY)) {
			for (int i = 0; i < INPUT_NUM; i++) {
				if (inputs[i].pin == io_num) {
					input = i;
					break;
				}
			}
			
			if (input < INPUT_NUM) {
				job_schedule(&(inputs[input].debounce_job), &debounce_job_fn, time_get() + MS_TO_MCU_TIME(DEBOUNCE_DURATION));
			}
		}
	}
}

void input_init(void)
{
	gpio_config_t io_conf = {};
	
	inputs[INPUT_BTN_LEFT].pin = BOARD_LEFT_BTN_PIN;
	inputs[INPUT_BTN_LEFT].state = get_input_hw_state(INPUT_BTN_LEFT);
	inputs[INPUT_BTN_LEFT].long_press_enabled = 1;

	inputs[INPUT_BTN_MIDDLE].pin = BOARD_MIDDLE_BTN_PIN;
	inputs[INPUT_BTN_MIDDLE].state = get_input_hw_state(INPUT_BTN_MIDDLE);
	inputs[INPUT_BTN_MIDDLE].long_press_enabled = 1;

	inputs[INPUT_BTN_RIGHT].pin = BOARD_RIGHT_BTN_PIN;
	inputs[INPUT_BTN_RIGHT].state = get_input_hw_state(INPUT_BTN_RIGHT);
	inputs[INPUT_BTN_RIGHT].long_press_enabled = 1;

	io_conf.intr_type = GPIO_INTR_ANYEDGE;
	io_conf.mode = GPIO_MODE_INPUT;
	io_conf.pin_bit_mask = (1ULL << BOARD_LEFT_BTN_PIN) | 
						   (1ULL << BOARD_MIDDLE_BTN_PIN) | 
						   (1ULL << BOARD_RIGHT_BTN_PIN);
	io_conf.pull_down_en = 0;
	io_conf.pull_up_en = 1;
	gpio_config(&io_conf);

	gpio_evt_queue = xQueueCreate(10, sizeof(uint32_t));
	xTaskCreate(gpio_task, "gpio_task", 2048, NULL, 10, NULL);

	gpio_install_isr_service(0);
	gpio_isr_handler_add(BOARD_LEFT_BTN_PIN, gpio_isr_handler, (void*) BOARD_LEFT_BTN_PIN);
	gpio_isr_handler_add(BOARD_MIDDLE_BTN_PIN, gpio_isr_handler, (void*) BOARD_MIDDLE_BTN_PIN);
	gpio_isr_handler_add(BOARD_RIGHT_BTN_PIN, gpio_isr_handler, (void*) BOARD_RIGHT_BTN_PIN);
}

input_state_t input_get_state(input_t input)
{
	return inputs[input].state;
}

void input_register_handler(void (*handler)(input_t, input_state_t, uint8_t))
{
	input_handler = handler;
}

static void long_press_job_fn(job_t *job)
{
	input_t input = INPUT_NUM;

	for (int i = 0; i < INPUT_NUM; i++) {
		if (job == &(inputs[i].long_press_job)) {
			input = i;
			break;
		}
	}

	if (input < INPUT_NUM && input_handler != NULL) {
		input_handler(input, inputs[input].state, 1);
	}
}

static void debounce_job_fn(job_t *job)
{
	input_t input = INPUT_NUM;

	for (int i = 0; i < INPUT_NUM; i++) {
		if (job == &(inputs[i].debounce_job)) {
			input = i;
			break;
		}
	}

	if (input >= INPUT_NUM) {
		return;
	}

	input_state_t new_state = get_input_hw_state(input);

	if (inputs[input].state == INPUT_STATE_LOW && new_state == INPUT_STATE_HIGH) {
		if (inputs[input].long_press_enabled) {
			job_schedule(&(inputs[input].long_press_job), &long_press_job_fn, time_get() + MS_TO_MCU_TIME(LONG_PRESS_DURATION));
		}
	} else if (inputs[input].state == INPUT_STATE_HIGH && new_state == INPUT_STATE_LOW) {
		if (inputs[input].long_press_enabled) {
			job_cancel(&(inputs[input].long_press_job));
		}
	} else if (inputs[input].state == new_state) {
		return;
	}

	inputs[input].state = new_state;

	if (input_handler != NULL) {
		input_handler(input, inputs[input].state, 0);
	}
}