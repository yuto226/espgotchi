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
#include "esp_spiffs.h"
#include "esp_log.h"

#include "storage.h"

#define TAG "storage"
#define STORAGE_FILE "/spiffs/storage.bin"

static bool spiffs_initialized = false;

static int8_t storage_init(void)
{
	if (spiffs_initialized) {
		return 0;
	}

	esp_vfs_spiffs_conf_t conf = {
		.base_path = "/spiffs",
		.partition_label = NULL,
		.max_files = 5,
		.format_if_mount_failed = true
	};

	esp_err_t ret = esp_vfs_spiffs_register(&conf);
	if (ret == ESP_ERR_INVALID_STATE) {
    	ESP_LOGW(TAG, "SPIFFS already mounted");
    	spiffs_initialized = true;
    	return 0;
	}
	if (ret != ESP_OK) {
		ESP_LOGE(TAG, "Failed to initialize SPIFFS (%s)", esp_err_to_name(ret));
		return -1;
	}

	size_t total = 0, used = 0;
	ret = esp_spiffs_info(NULL, &total, &used);
	if (ret != ESP_OK) {
		ESP_LOGE(TAG, "Failed to get SPIFFS partition information (%s)", esp_err_to_name(ret));
		return -1;
	}

	ESP_LOGI(TAG, "Partition size: total: %d, used: %d", total, used);
	spiffs_initialized = true;
	return 0;
}

int8_t storage_read(uint32_t offset, uint32_t *data, uint32_t length)
{
	if (storage_init() != 0) {
		return -1;
	}

	FILE *f = fopen(STORAGE_FILE, "rb");
	if (f == NULL) {
		ESP_LOGW(TAG, "Storage file not found, creating empty storage");
		memset(data, 0xFF, length * sizeof(uint32_t));
		return 0;
	}

	if (fseek(f, offset * sizeof(uint32_t), SEEK_SET) != 0) {
		fclose(f);
		return -1;
	}

	size_t bytes_read = fread(data, sizeof(uint32_t), length, f);
	fclose(f);

	if (bytes_read < length) {
		memset(&data[bytes_read], 0xFF, (length - bytes_read) * sizeof(uint32_t));
	}

	return 0;
}

int8_t storage_write(uint32_t offset, uint32_t *data, uint32_t length)
{
	if (storage_init() != 0) {
		return -1;
	}

	FILE *f = fopen(STORAGE_FILE, "r+b");
	if (f == NULL) {
		f = fopen(STORAGE_FILE, "w+b");
		if (f == NULL) {
			ESP_LOGE(TAG, "Failed to create storage file");
			return -1;
		}
	}

	if (fseek(f, offset * sizeof(uint32_t), SEEK_SET) != 0) {
		fclose(f);
		return -1;
	}

	size_t bytes_written = fwrite(data, sizeof(uint32_t), length, f);
	fclose(f);

	if (bytes_written != length) {
		ESP_LOGE(TAG, "Failed to write all data");
		return -1;
	}

	return 0;
}

int8_t storage_erase(void)
{
	if (storage_init() != 0) {
		return -1;
	}

	if (remove(STORAGE_FILE) != 0) {
		ESP_LOGW(TAG, "Storage file doesn't exist or failed to remove");
	}

	return 0;
}