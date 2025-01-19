#ifndef JAPERSIK_ESP32_I2C_ADAPTER_I2C_ADAPTER
#define JAPERSIK_ESP32_I2C_ADAPTER_I2C_ADAPTER

#include <stddef.h>
#include <stdint.h>

#include "driver/i2c.h"
#include "esp_err.h"
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "soc/gpio_num.h"

typedef struct I2CAdapter I2CAdapter;

typedef struct I2CAdapterConfig {
  i2c_port_t port;
  gpio_num_t sda_io;
  gpio_num_t scl_io;
  uint32_t clk_speed;

} I2CAdapterConfig;

I2CAdapter *i2c_adapter_new(I2CAdapterConfig config);
void i2c_adapter_free(I2CAdapter *adapter);

esp_err_t i2c_adapter_write(I2CAdapter *self, uint8_t address, uint8_t *data,
                            size_t data_len);
esp_err_t i2c_adapter_read(I2CAdapter *self, uint8_t address, uint8_t *data,
                           size_t data_len);
esp_err_t i2c_adapter_ping(I2CAdapter *self, uint8_t address);
#endif // !JAPERSIK_ESP32_I2C_ADAPTER_I2C_ADAPTER
