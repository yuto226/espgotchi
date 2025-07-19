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
#include "nvs_flash.h"
#include "nvs.h"
#include "esp_log.h"

#include "config.h"

#define TAG "config"
#define NVS_NAMESPACE "tamagotchi"

void config_save(config_t *cfg)
{
	nvs_handle_t nvs_handle;
	esp_err_t err;

	err = nvs_open(NVS_NAMESPACE, NVS_READWRITE, &nvs_handle);
	if (err != ESP_OK) {
		ESP_LOGE(TAG, "Failed to open NVS handle");
		return;
	}

	err = nvs_set_blob(nvs_handle, "config", cfg, sizeof(config_t));
	if (err != ESP_OK) {
		ESP_LOGE(TAG, "Failed to save config to NVS");
	} else {
		err = nvs_commit(nvs_handle);
		if (err != ESP_OK) {
			ESP_LOGE(TAG, "Failed to commit NVS");
		}
	}

	nvs_close(nvs_handle);
}

int8_t config_load(config_t *cfg)
{
	nvs_handle_t nvs_handle;
	esp_err_t err;
	size_t required_size = sizeof(config_t);

	err = nvs_open(NVS_NAMESPACE, NVS_READONLY, &nvs_handle);
	if (err != ESP_OK) {
		ESP_LOGE(TAG, "Failed to open NVS handle");
		return -1;
	}

	err = nvs_get_blob(nvs_handle, "config", cfg, &required_size);
	nvs_close(nvs_handle);

	if (err != ESP_OK || required_size != sizeof(config_t)) {
		ESP_LOGE(TAG, "Failed to load config from NVS");
		return -1;
	}

	return 0;
}