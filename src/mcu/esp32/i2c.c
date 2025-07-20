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
#include <string.h>
#include <stdbool.h>
#include "driver/i2c_master.h"
#include "esp_log.h"

#include "i2c.h"
#include "board.h"

static const char *TAG = "I2C";
static i2c_master_bus_handle_t i2c_bus_handle;

void i2c_init(void)
{
	i2c_master_bus_config_t i2c_mst_config = {
		.clk_source = I2C_CLK_SRC_DEFAULT,
		.i2c_port = BOARD_SCREEN_I2C_NUM,
		.scl_io_num = BOARD_SCREEN_I2C_SCL_PIN,
		.sda_io_num = BOARD_SCREEN_I2C_SDA_PIN,
		.glitch_ignore_cnt = 7,
		.flags.enable_internal_pullup = true,
	};
	
	ESP_ERROR_CHECK(i2c_new_master_bus(&i2c_mst_config, &i2c_bus_handle));
	
	ESP_LOGI(TAG, "I2C master bus initialized on SCL:%d, SDA:%d", BOARD_SCREEN_I2C_SCL_PIN, BOARD_SCREEN_I2C_SDA_PIN);
}

bool i2c_write_byte(uint8_t device_addr, uint8_t data)
{
	i2c_device_config_t dev_cfg = {
		.dev_addr_length = I2C_ADDR_BIT_LEN_7,
		.device_address = device_addr,
		.scl_speed_hz = 400000,
	};
	
	i2c_master_dev_handle_t dev_handle;
	esp_err_t ret = i2c_master_bus_add_device(i2c_bus_handle, &dev_cfg, &dev_handle);
	if (ret != ESP_OK) {
		ESP_LOGW(TAG, "Failed to add I2C device: %s", esp_err_to_name(ret));
		return false;
	}
	
	ret = i2c_master_transmit(dev_handle, &data, 1, I2C_TIMEOUT_MS);
	i2c_master_bus_rm_device(dev_handle);
	
	if (ret != ESP_OK) {
		ESP_LOGW(TAG, "I2C write byte failed: %s", esp_err_to_name(ret));
		return false;
	}
	
	return true;
}

bool i2c_write_data(uint8_t device_addr, uint8_t *data, size_t length)
{
	i2c_device_config_t dev_cfg = {
		.dev_addr_length = I2C_ADDR_BIT_LEN_7,
		.device_address = device_addr,
		.scl_speed_hz = 400000,
	};
	
	i2c_master_dev_handle_t dev_handle;
	esp_err_t ret = i2c_master_bus_add_device(i2c_bus_handle, &dev_cfg, &dev_handle);
	if (ret != ESP_OK) {
		ESP_LOGW(TAG, "Failed to add I2C device: %s", esp_err_to_name(ret));
		return false;
	}
	
	ret = i2c_master_transmit(dev_handle, data, length, I2C_TIMEOUT_MS);
	i2c_master_bus_rm_device(dev_handle);
	
	if (ret != ESP_OK) {
		ESP_LOGW(TAG, "I2C write data failed: %s", esp_err_to_name(ret));
		return false;
	}
	
	return true;
}

bool i2c_read_data(uint8_t device_addr, uint8_t *data, size_t length)
{
	i2c_device_config_t dev_cfg = {
		.dev_addr_length = I2C_ADDR_BIT_LEN_7,
		.device_address = device_addr,
		.scl_speed_hz = 400000,
	};
	
	i2c_master_dev_handle_t dev_handle;
	esp_err_t ret = i2c_master_bus_add_device(i2c_bus_handle, &dev_cfg, &dev_handle);
	if (ret != ESP_OK) {
		ESP_LOGW(TAG, "Failed to add I2C device: %s", esp_err_to_name(ret));
		return false;
	}
	
	ret = i2c_master_receive(dev_handle, data, length, I2C_TIMEOUT_MS);
	i2c_master_bus_rm_device(dev_handle);
	
	if (ret != ESP_OK) {
		ESP_LOGW(TAG, "I2C read data failed: %s", esp_err_to_name(ret));
		return false;
	}
	
	return true;
}

bool i2c_device_detect(uint8_t device_addr)
{
	esp_err_t ret = i2c_master_probe(i2c_bus_handle, device_addr, I2C_TIMEOUT_MS);
	return (ret == ESP_OK);
}