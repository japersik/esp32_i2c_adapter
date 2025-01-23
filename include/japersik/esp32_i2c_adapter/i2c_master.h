#ifndef JAPERSIK_ESP32_I2C_ADAPTER_I2C_MASTER
#define JAPERSIK_ESP32_I2C_ADAPTER_I2C_MASTER

#include <stddef.h>
#include <stdint.h>

#include "driver/i2c_master.h"
#include "esp_err.h"
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "soc/gpio_num.h"

typedef struct I2CMasterAdapter I2CMasterAdapter;
typedef struct I2CMasterDevice I2CMasterDevice;

typedef struct I2CMasterAdapterConfig {
  i2c_port_t port;
  gpio_num_t sda_io;
  gpio_num_t scl_io;
} I2CMasterAdapterConfig;

typedef struct I2CMasterDeviceConfig {
  I2CMasterAdapter *adapter;
  uint16_t address;
  uint32_t scl_speed;
} I2CMasterDeviceConfig;

I2CMasterAdapter *i2c_master_adapter_new(I2CMasterAdapterConfig config);
void i2c_master_adapter_free(I2CMasterAdapter *adapter);

esp_err_t i2c_master_adapter_ping(I2CMasterAdapter *self, uint16_t address);

I2CMasterDevice *i2c_master_device_new(I2CMasterDeviceConfig config);
void i2c_master_device_free(I2CMasterDevice *adapter);

esp_err_t i2c_master_device_write(I2CMasterDevice *self, uint8_t *data, size_t data_len);
esp_err_t i2c_master_device_read(I2CMasterDevice *self, uint8_t *data, size_t data_len);
esp_err_t i2c_master_device_ping(I2CMasterDevice *self);
#endif	// !JAPERSIK_ESP32_I2C_ADAPTER_I2C_MASTER
