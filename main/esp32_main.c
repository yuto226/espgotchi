/*
 * ESP32 Tamagotchi Emulator - Main Entry Point
 */
#include <stdio.h>
#include "esp_log.h"
#include "nvs_flash.h"
#include "esp_spiffs.h"

extern int main(void);

static const char *TAG = "esp32_main";

void app_main(void)
{
    ESP_LOGI(TAG, "ESP32 Tamagotchi Emulator starting...");
    
    // Initialize NVS
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);
    
    // Call original main function
    main();
}