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
#include <stdbool.h>

#ifdef ESP32
#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_timer.h"
#include "rom/ets_sys.h"
#include "esp_log.h"
#endif

#include "time.h"
#include "i2c.h"
#include "gpio.h"
#include "board.h"
#include "ssd1306.h"

uint8_t ssd1306_i2c_addr = SSD1306_I2C_ADDR_DEFAULT;

static bool ssd1306_detect_i2c_address(void)
{
#ifdef BOARD_SSD1306_I2C
	if (i2c_device_detect(SSD1306_I2C_ADDR_DEFAULT)) {
		ssd1306_i2c_addr = SSD1306_I2C_ADDR_DEFAULT;
		return true;
	}
	if (i2c_device_detect(SSD1306_I2C_ADDR_ALT)) {
		ssd1306_i2c_addr = SSD1306_I2C_ADDR_ALT;
		return true;
	}
#endif
	return false;
}

void ssd1306_init(void)
{
#ifdef BOARD_SSD1306_I2C
	i2c_init();
	
	if (!ssd1306_detect_i2c_address()) {
#ifdef ESP32
		ESP_LOGE("SSD1306", "SSD1306 device not found on I2C bus");
#endif
		return;
	}
#else
	spi_init();
#endif

#ifdef ESP32
	gpio_config_t io_conf = {};
	io_conf.intr_type = GPIO_INTR_DISABLE;
	io_conf.mode = GPIO_MODE_OUTPUT;
	io_conf.pin_bit_mask = (1ULL << BOARD_SCREEN_DC_PIN) | 
						   (1ULL << BOARD_SCREEN_RST_PIN);
	io_conf.pull_down_en = 0;
	io_conf.pull_up_en = 0;
	gpio_config(&io_conf);

	gpio_set_level(BOARD_SCREEN_DC_PIN, 0);
	gpio_set_level(BOARD_SCREEN_RST_PIN, 0);
	vTaskDelay(pdMS_TO_TICKS(1));
	gpio_set_level(BOARD_SCREEN_RST_PIN, 1);
	vTaskDelay(pdMS_TO_TICKS(1));
	gpio_set_level(BOARD_SCREEN_RST_PIN, 0);
	vTaskDelay(pdMS_TO_TICKS(1));
	gpio_set_level(BOARD_SCREEN_RST_PIN, 1);
	gpio_set_level(BOARD_SCREEN_DC_PIN, 1);
	vTaskDelay(pdMS_TO_TICKS(10));
#else
	/* Power-up sequence */
#ifndef BOARD_SSD1306_I2C
	gpio_clear(BOARD_SCREEN_DC_PORT, BOARD_SCREEN_DC_PIN);
	gpio_clear(BOARD_SCREEN_NSS_PORT, BOARD_SCREEN_NSS_PIN);
#endif
	gpio_clear(BOARD_SCREEN_RST_PORT, BOARD_SCREEN_RST_PIN);
	time_delay(MS_TO_MCU_TIME(1));
	gpio_set(BOARD_SCREEN_RST_PORT, BOARD_SCREEN_RST_PIN);
	time_delay(MS_TO_MCU_TIME(1));
	gpio_clear(BOARD_SCREEN_RST_PORT, BOARD_SCREEN_RST_PIN);
	time_delay(MS_TO_MCU_TIME(1));
	gpio_set(BOARD_SCREEN_RST_PORT, BOARD_SCREEN_RST_PIN);
#ifndef BOARD_SSD1306_I2C
	gpio_set(BOARD_SCREEN_DC_PORT, BOARD_SCREEN_DC_PIN);
	gpio_set(BOARD_SCREEN_NSS_PORT, BOARD_SCREEN_NSS_PIN);
#endif
	time_delay(MS_TO_MCU_TIME(10));
#endif

	/* Configuration */
	ssd1306_send_cmd_2b(REG_MUX_RATIO, 0x3F);
	ssd1306_send_cmd_2b(REG_DISP_OFFSET, 0x00);
	ssd1306_send_cmd_1b(REG_DISP_START_LINE, 0);
	ssd1306_send_cmd_1b(REG_SEG_REMAP, 0);
	ssd1306_send_cmd_2b(REG_COM_PINS_CFG, 0x12);
	ssd1306_send_cmd_1b(REG_COM_SCAN_DIR, 0);

	ssd1306_send_cmd_2b(REG_CONTRAST, 0x7F);
	ssd1306_send_cmd_2b(REG_MEM_ADDR_MODE, MEM_ADDR_MODE_H);
	ssd1306_send_cmd_3b(REG_COL_ADDR, 0x00, 0x7F);
	ssd1306_send_cmd_3b(REG_PAGE_ADDR, 0x00, 0x07);

	ssd1306_send_cmd_1b(REG_DISP_MODE, 0);
	ssd1306_send_cmd_2b(REG_VCOMH_LVL, 0x00);

	ssd1306_send_cmd_1b(REG_DISP_ON, 0);
	ssd1306_send_cmd_2b(REG_DISP_CLK_CFG, 0x80);
	ssd1306_send_cmd_2b(REG_CHRG_PUMP, 0x14);
	ssd1306_send_cmd_1b(REG_DISP_EN, 1);
}

void ssd1306_set_display_mode(disp_mode_t mode)
{
	ssd1306_send_cmd_1b(REG_DISP_MODE, (mode == DISP_MODE_NORMAL) ? 0 : 1);
}

void ssd1306_set_power_mode(pwr_mode_t mode)
{
	switch (mode) {
		case PWR_MODE_SLEEP:
			ssd1306_send_cmd_1b(REG_DISP_EN, 0);
			ssd1306_send_cmd_2b(REG_CHRG_PUMP, 0x10);
			break;

		case PWR_MODE_ON:
			ssd1306_send_cmd_2b(REG_CHRG_PUMP, 0x14);
			ssd1306_send_cmd_1b(REG_DISP_EN, 1);
			break;
	}
}

void ssd1306_send_cmd_1b(uint8_t reg, uint8_t data)
{
#ifdef BOARD_SSD1306_I2C
	uint8_t cmd = reg | data;
	if (!i2c_write_byte(ssd1306_i2c_addr, 0x00) || 
		!i2c_write_byte(ssd1306_i2c_addr, cmd)) {
#ifdef ESP32
		ESP_LOGW("SSD1306", "Failed to send 1-byte command: 0x%02X", cmd);
#endif
	}
#elif defined(ESP32)
	gpio_set_level(BOARD_SCREEN_DC_PIN, 0);
	spi_write(reg | data);
	ets_delay_us(1);
#else
#ifndef BOARD_SSD1306_I2C
	gpio_clear(BOARD_SCREEN_DC_PORT, BOARD_SCREEN_DC_PIN);
	gpio_clear(BOARD_SCREEN_NSS_PORT, BOARD_SCREEN_NSS_PIN);

	spi_write(reg | data);
	time_delay(US_TO_MCU_TIME(1));

	gpio_set(BOARD_SCREEN_NSS_PORT, BOARD_SCREEN_NSS_PIN);
#endif
#endif
}

void ssd1306_send_cmd_2b(uint8_t reg, uint8_t data)
{
#ifdef BOARD_SSD1306_I2C
	uint8_t cmd_data[3] = {0x00, reg, data};
	if (!i2c_write_data(ssd1306_i2c_addr, cmd_data, 3)) {
#ifdef ESP32
		ESP_LOGW("SSD1306", "Failed to send 2-byte command: 0x%02X 0x%02X", reg, data);
#endif
	}
#elif defined(ESP32)
	gpio_set_level(BOARD_SCREEN_DC_PIN, 0);
	spi_write(reg);
	spi_write(data);
	ets_delay_us(1);
#else
#ifndef BOARD_SSD1306_I2C
	gpio_clear(BOARD_SCREEN_DC_PORT, BOARD_SCREEN_DC_PIN);
	gpio_clear(BOARD_SCREEN_NSS_PORT, BOARD_SCREEN_NSS_PIN);

	spi_write(reg);
	spi_write(data);
	time_delay(US_TO_MCU_TIME(1));

	gpio_set(BOARD_SCREEN_NSS_PORT, BOARD_SCREEN_NSS_PIN);
#endif
#endif
}

void ssd1306_send_cmd_3b(uint8_t reg, uint8_t data1, uint8_t data2)
{
#ifdef BOARD_SSD1306_I2C
	uint8_t cmd_data[4] = {0x00, reg, data1, data2};
	if (!i2c_write_data(ssd1306_i2c_addr, cmd_data, 4)) {
#ifdef ESP32
		ESP_LOGW("SSD1306", "Failed to send 3-byte command: 0x%02X 0x%02X 0x%02X", reg, data1, data2);
#endif
	}
#elif defined(ESP32)
	gpio_set_level(BOARD_SCREEN_DC_PIN, 0);
	spi_write(reg);
	spi_write(data1);
	spi_write(data2);
	ets_delay_us(1);
#else
#ifndef BOARD_SSD1306_I2C
	gpio_clear(BOARD_SCREEN_DC_PORT, BOARD_SCREEN_DC_PIN);
	gpio_clear(BOARD_SCREEN_NSS_PORT, BOARD_SCREEN_NSS_PIN);

	spi_write(reg);
	spi_write(data1);
	spi_write(data2);
	time_delay(US_TO_MCU_TIME(1));

	gpio_set(BOARD_SCREEN_NSS_PORT, BOARD_SCREEN_NSS_PIN);
#endif
#endif
}

void ssd1306_send_data(uint8_t *data, uint16_t length)
{
	uint16_t i;

#ifdef BOARD_SSD1306_I2C
	if (!i2c_write_byte(ssd1306_i2c_addr, 0x40)) {
#ifdef ESP32
		ESP_LOGW("SSD1306", "Failed to send data header");
#endif
		return;
	}
	
	for (i = 0; i < length; i++) {
		if (!i2c_write_byte(ssd1306_i2c_addr, data[i])) {
#ifdef ESP32
			ESP_LOGW("SSD1306", "Failed to send data byte %d/%d", i+1, length);
#endif
			break;
		}
	}
#elif defined(ESP32)
	gpio_set_level(BOARD_SCREEN_DC_PIN, 1);
	for (i = 0; i < length; i++) {
		spi_write(data[i]);
	}
	ets_delay_us(1);
#else
#ifndef BOARD_SSD1306_I2C
	gpio_set(BOARD_SCREEN_DC_PORT, BOARD_SCREEN_DC_PIN);
	gpio_clear(BOARD_SCREEN_NSS_PORT, BOARD_SCREEN_NSS_PIN);

	for (i = 0; i < length; i++) {
		spi_write(data[i]);
	}
	time_delay(US_TO_MCU_TIME(1));

	gpio_set(BOARD_SCREEN_NSS_PORT, BOARD_SCREEN_NSS_PIN);
#endif
#endif
}
