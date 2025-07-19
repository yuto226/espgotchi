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

#ifdef CONFIG_IDF_TARGET_ESP32S2
#include "tinyusb.h"
#include "tusb_msc_storage.h"
#include "esp_log.h"
#endif

#include "usb.h"

#ifdef CONFIG_IDF_TARGET_ESP32S2
#define TAG "usb"

static bool usb_initialized = false;
static bool usb_started = false;

static esp_err_t storage_init_spiflash(wl_handle_t *wl_handle);
static void storage_mount_changed_cb(tinyusb_msc_event_t event, tinyusb_msc_event_data_t *event_data);

static tinyusb_config_t tusb_cfg = {
    .device_descriptor = NULL,
    .string_descriptor = NULL,
    .external_phy = false,
    .configuration_descriptor = NULL,
};

static tinyusb_config_msc_t msc_cfg = {
    .pdrv = 0,
    .callback_mount_changed = storage_mount_changed_cb,
    .callback_premount_changed = NULL,
};

static void storage_mount_changed_cb(tinyusb_msc_event_t event, tinyusb_msc_event_data_t *event_data)
{
    ESP_LOGI(TAG, "MSC event: %d", event);
    if (event == TINYUSB_MSC_EVENT_MOUNT_CHANGED) {
        ESP_LOGI(TAG, "MSC mounted: %s", event_data->mount_changed_data.is_mounted ? "true" : "false");
    }
}

static esp_err_t storage_init_spiflash(wl_handle_t *wl_handle)
{
    ESP_LOGI(TAG, "Initializing wear levelling");

    const esp_vfs_fat_mount_config_t mount_config = {
        .format_if_mount_failed = true,
        .max_files = 4,
        .allocation_unit_size = CONFIG_WL_SECTOR_SIZE
    };

    esp_err_t err = esp_vfs_fat_spiflash_mount_rw_wl("/data", "storage", &mount_config, wl_handle);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to mount FATFS (%s)", esp_err_to_name(err));
        return err;
    }
    return ESP_OK;
}
#endif

void usb_init(void)
{
#ifdef CONFIG_IDF_TARGET_ESP32S2
    if (usb_initialized) {
        return;
    }

    wl_handle_t wl_handle;
    ESP_ERROR_CHECK(storage_init_spiflash(&wl_handle));

    ESP_LOGI(TAG, "USB MSC initialization");
    ESP_ERROR_CHECK(tinyusb_driver_install(&tusb_cfg));
    ESP_ERROR_CHECK(tinyusb_msc_register_callback(TINYUSB_MSC_EVENT_MOUNT_CHANGED, storage_mount_changed_cb));
    ESP_ERROR_CHECK(tinyusb_msc_storage_init_spiflash(&msc_cfg));
    ESP_ERROR_CHECK(tinyusb_msc_storage_mount("/data"));

    usb_initialized = true;
    ESP_LOGI(TAG, "USB MSC initialization DONE");
#endif
}

void usb_deinit(void)
{
#ifdef CONFIG_IDF_TARGET_ESP32S2
    if (!usb_initialized) {
        return;
    }

    tinyusb_msc_storage_unmount();
    tinyusb_driver_uninstall();
    usb_initialized = false;
    ESP_LOGI(TAG, "USB MSC deinitialized");
#endif
}

void usb_start(void)
{
#ifdef CONFIG_IDF_TARGET_ESP32S2
    if (!usb_initialized || usb_started) {
        return;
    }

    usb_started = true;
    ESP_LOGI(TAG, "USB MSC started");
#endif
}

void usb_stop(void)
{
#ifdef CONFIG_IDF_TARGET_ESP32S2
    if (!usb_started) {
        return;
    }

    usb_started = false;
    ESP_LOGI(TAG, "USB MSC stopped");
#endif
}