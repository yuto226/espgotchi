#include "u8g2.h"
#include "freertos/FreeRTOS.h"
#include "driver/i2c_master.h"
#include "esp_log.h"
#include <string.h>
#include "esp_err.h"

#define I2C_MASTER_PORT       I2C_NUM_0
#define I2C_MASTER_SDA_IO     26
#define I2C_MASTER_SCL_IO     25
#define I2C_MASTER_FREQ_HZ    400000
#define I2C_TIMEOUT_MS        2000

static const char *TAG = "u8x8_hw_i2c";

// 転送用ハンドル（開始～終了まで維持）
static i2c_master_dev_handle_t i2c_dev = NULL;
static i2c_master_bus_handle_t i2c_bus = NULL;

// 初期化処理
static void i2c_master_hw_init(uint8_t i2c_address) {
    if (i2c_bus == NULL) {
        i2c_master_bus_config_t bus_cfg = {
            .clk_source = I2C_CLK_SRC_DEFAULT,
            .i2c_port = I2C_MASTER_PORT,
            .sda_io_num = I2C_MASTER_SDA_IO,
            .scl_io_num = I2C_MASTER_SCL_IO,
            .flags.enable_internal_pullup = true,
        };
        ESP_ERROR_CHECK(i2c_new_master_bus(&bus_cfg, &i2c_bus));
    }

    if (i2c_dev == NULL) {
        i2c_device_config_t dev_cfg = {
            .dev_addr_length = I2C_ADDR_BIT_LEN_7,
            .device_address = i2c_address,
            .scl_speed_hz = I2C_MASTER_FREQ_HZ,
        };
        ESP_ERROR_CHECK(i2c_master_bus_add_device(i2c_bus, &dev_cfg, &i2c_dev));
    }
}

// U8g2向け I2C送信関数
uint8_t u8x8_byte_hw_i2c(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr) {
    static uint8_t buffer[128];  // 最大128バイト分保持
    static size_t buffer_pos = 0;

    switch (msg) {
        case U8X8_MSG_BYTE_INIT:
            i2c_master_hw_init(0x3C);
            break;

        case U8X8_MSG_BYTE_START_TRANSFER:
            buffer_pos = 0;  // 転送バッファのリセット
            break;

        case U8X8_MSG_BYTE_SEND:
            if ((buffer_pos + arg_int) <= sizeof(buffer)) {
                memcpy(&buffer[buffer_pos], arg_ptr, arg_int);
                buffer_pos += arg_int;
            } else {
                ESP_LOGE(TAG, "I2C buffer overflow");
                return 0;
            }
            break;

        case U8X8_MSG_BYTE_END_TRANSFER: {
            esp_err_t err = i2c_master_transmit(i2c_dev, buffer, buffer_pos, pdMS_TO_TICKS(I2C_TIMEOUT_MS));
            if (err != ESP_OK) {
                ESP_LOGE(TAG, "I2C transmit failed: %s", esp_err_to_name(err));
                return 0;
            }
            break;
        }

        default:
            return 0;
    }

    return 1;
}