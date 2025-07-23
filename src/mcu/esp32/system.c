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
#include "driver/gpio.h"
#include "esp_intr_alloc.h"

#include "system.h"
#include "board.h"

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

static void IRAM_ATTR gpio_isr_handler(void* arg) {
    uint32_t gpio_num = (uint32_t)arg;
    // ISR処理（必要に応じて実装）
}

void board_init(void)
{
    // ESP32 GPIO初期化
    
    /* Buttons - 基本的なGPIO設定（割り込みはboard_init_irqで設定） */
    gpio_set_direction(BOARD_LEFT_BTN_PIN, GPIO_MODE_INPUT);
    gpio_set_pull_mode(BOARD_LEFT_BTN_PIN, GPIO_PULLUP_ONLY);  // ESP32では通常プルアップ
    
    gpio_set_direction(BOARD_MIDDLE_BTN_PIN, GPIO_MODE_INPUT);
    gpio_set_pull_mode(BOARD_MIDDLE_BTN_PIN, GPIO_FLOATING);   // プルなし
    
    gpio_set_direction(BOARD_RIGHT_BTN_PIN, GPIO_MODE_INPUT);
    gpio_set_pull_mode(BOARD_RIGHT_BTN_PIN, GPIO_PULLUP_ONLY);
    
    /* Screen control pins */
    // Reset pin for SSD1306 I2C mode
    gpio_set_direction(BOARD_SCREEN_RST_PIN, GPIO_MODE_OUTPUT);
    gpio_set_level(BOARD_SCREEN_RST_PIN, 1);  // リセット初期値HIGH
    
    /* RGB LED pins - これらはLEDCで制御されるため出力モードに設定 */
    gpio_set_direction(BOARD_LED_RED_PIN, GPIO_MODE_OUTPUT);
    gpio_set_level(BOARD_LED_RED_PIN, 0);
    
    gpio_set_direction(BOARD_LED_GREEN_PIN, GPIO_MODE_OUTPUT);
    gpio_set_level(BOARD_LED_GREEN_PIN, 0);
    
    gpio_set_direction(BOARD_LED_BLUE_PIN, GPIO_MODE_OUTPUT);
    gpio_set_level(BOARD_LED_BLUE_PIN, 0);
    
    /* Speaker pin */
    gpio_set_direction(BOARD_SPEAKER_PIN, GPIO_MODE_OUTPUT);
    gpio_set_level(BOARD_SPEAKER_PIN, 0);
    
    /* Battery voltage measurement pin - アナログ入力 */
    gpio_set_direction(BOARD_VBATT_ANA_PIN, GPIO_MODE_DISABLE);  // アナログモード
    gpio_set_pull_mode(BOARD_VBATT_ANA_PIN, GPIO_FLOATING);
    
    /* USB pins - 入力モード */
    gpio_set_direction(BOARD_USB_DP_PIN, GPIO_MODE_INPUT);
    gpio_set_pull_mode(BOARD_USB_DP_PIN, GPIO_FLOATING);
    
    gpio_set_direction(BOARD_USB_DM_PIN, GPIO_MODE_INPUT);
    gpio_set_pull_mode(BOARD_USB_DM_PIN, GPIO_FLOATING);
    
    // 注意: SPIピン（SCLK, MOSI）はSPIドライバが管理するため、ここでは設定しない
    // 注意: I2Cピン（SCL, SDA）はI2Cドライバが管理するため、ここでは設定しない
    // 注意: バックライトピンはLEDCドライバが管理するため、ここでは設定しない
}

void board_init_irq(void)
{
    // --- ボタン GPIO (既存のピン定義を使用) ---
    gpio_set_direction(BOARD_LEFT_BTN_PIN, GPIO_MODE_INPUT);
    gpio_set_pull_mode(BOARD_LEFT_BTN_PIN, GPIO_PULLUP_ONLY);
    gpio_set_intr_type(BOARD_LEFT_BTN_PIN, GPIO_INTR_NEGEDGE);
    gpio_isr_handler_add(BOARD_LEFT_BTN_PIN, gpio_isr_handler, (void*)BOARD_LEFT_BTN_PIN);

    gpio_set_direction(BOARD_MIDDLE_BTN_PIN, GPIO_MODE_INPUT);
    gpio_set_pull_mode(BOARD_MIDDLE_BTN_PIN, GPIO_PULLUP_ONLY);
    gpio_set_intr_type(BOARD_MIDDLE_BTN_PIN, GPIO_INTR_NEGEDGE);
    gpio_isr_handler_add(BOARD_MIDDLE_BTN_PIN, gpio_isr_handler, (void*)BOARD_MIDDLE_BTN_PIN);

    gpio_set_direction(BOARD_RIGHT_BTN_PIN, GPIO_MODE_INPUT);
    gpio_set_pull_mode(BOARD_RIGHT_BTN_PIN, GPIO_PULLUP_ONLY);
    gpio_set_intr_type(BOARD_RIGHT_BTN_PIN, GPIO_INTR_NEGEDGE);
    gpio_isr_handler_add(BOARD_RIGHT_BTN_PIN, gpio_isr_handler, (void*)BOARD_RIGHT_BTN_PIN);

    // --- USB検出用GPIO (適切なピンが定義されていればUSB関連も追加可能) ---
    // 今回はVBUSピンが明確でないため一時的にコメントアウト
    /*
    gpio_set_direction(VBUS_GPIO, GPIO_MODE_INPUT);
    gpio_set_intr_type(VBUS_GPIO, GPIO_INTR_ANYEDGE);
    gpio_isr_handler_add(VBUS_GPIO, gpio_isr_handler, (void*)VBUS_GPIO);
    */
}