#include "u8g2.h"
#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/i2c_master.h"
#include "esp_log.h"

uint8_t u8x8_gpio_and_delay_esp32(u8x8_t *u8x8,
                                  uint8_t msg, uint8_t arg_int, void *arg_ptr) {
    switch (msg) {
        case U8X8_MSG_DELAY_MILLI:
            vTaskDelay(pdMS_TO_TICKS(arg_int));
            break;

        case U8X8_MSG_GPIO_AND_DELAY_INIT:
            // GPIOの初期化が必要ならここでやる（SSD1306なら基本不要）
            break;

        case U8X8_MSG_GPIO_RESET:
            // RESETピン使うならここに処理を書く
            break;

        default:
            return 0;
    }
    return 1;
}
