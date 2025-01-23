#include "japersik/esp32_i2c_adapter/i2c_master.h"

#include <stdlib.h>

#include "driver/i2c.h"
#include "driver/i2c_master.h"
#include "esp_err.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "soc/gpio_num.h"

static const char *TAG = "I2C_MASTER_ADAPTER";

struct I2CMasterAdapter {
  i2c_master_bus_handle_t bus_handle;
};

struct I2CMasterDevice {
  I2CMasterAdapter *adapter;
  i2c_master_dev_handle_t dev_handle;
  uint16_t address;
};

I2CMasterAdapter *i2c_master_adapter_new(I2CMasterAdapterConfig config) {
  i2c_master_bus_handle_t i2c_master_bus_handle;

  i2c_master_bus_config_t i2c_master_bus_config = {.clk_source = I2C_CLK_SRC_DEFAULT,
						   .i2c_port = config.port,
						   .glitch_ignore_cnt = 7,
						   .flags.enable_internal_pullup = true,
						   .scl_io_num = config.scl_io,
						   .sda_io_num = config.sda_io};
  ESP_ERROR_CHECK(i2c_new_master_bus(&i2c_master_bus_config, &i2c_master_bus_handle));

  I2CMasterAdapter *master = calloc(1, sizeof(I2CMasterAdapter));
  if (!master) {
    ESP_LOGE(TAG, "Failed to allocate memory for I2C adapter");
    i2c_del_master_bus(i2c_master_bus_handle);
    return NULL;
  }
  master->bus_handle = i2c_master_bus_handle;

  ESP_LOGI(TAG, "I2C master bus initialized on port %d", config.port);
  return master;
};

void i2c_master_adapter_free(I2CMasterAdapter *self) {
  i2c_del_master_bus(self->bus_handle);
  free(self);
};

esp_err_t i2c_master_adapter_ping(I2CMasterAdapter *self, uint16_t address) {
  return i2c_master_probe(self->bus_handle, address, 1000);
};

I2CMasterDevice *i2c_master_device_new(I2CMasterDeviceConfig config) {
  i2c_device_config_t i2c_device_config = {
      .scl_speed_hz = config.scl_speed,
      .device_address = config.address,
  };
  i2c_master_dev_handle_t dev_handle;
  i2c_master_bus_add_device(config.adapter->bus_handle, &i2c_device_config, &dev_handle);

  I2CMasterDevice *dev = calloc(1, sizeof(I2CMasterDevice));
  if (!dev) {
    ESP_LOGE(TAG, "Failed to allocate memory for I2C adapter");
    i2c_master_bus_rm_device(dev_handle);
    return NULL;
  }
  dev->dev_handle = dev_handle;
  dev->adapter = config.adapter;
  return dev;
};

void i2c_master_device_free(I2CMasterDevice *self) {
  i2c_master_bus_rm_device(self->dev_handle);
  free(self);
};

esp_err_t i2c_master_device_write(I2CMasterDevice *self, uint8_t *data, size_t data_len) {
  return i2c_master_transmit(self->dev_handle, data, data_len, -1);
};

esp_err_t i2c_master_device_read(I2CMasterDevice *self, uint8_t *buffer, size_t buffer_len) {
  uint8_t wrute_for_read[] = {0X00};
  return i2c_master_transmit_receive(self->dev_handle, wrute_for_read, sizeof(wrute_for_read), buffer, buffer_len, -1);
};

esp_err_t i2c_master_device_ping(I2CMasterDevice *self) {
  return i2c_master_adapter_ping(self->adapter, self->address);
};
