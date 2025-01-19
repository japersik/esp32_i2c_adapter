
#include "japersik/esp32_i2c_adapter/i2c_adapter.h"

#include <stdlib.h>

#include "esp_log.h"

static const char* TAG = "I2C_ADAPTER";

struct I2CAdapter {
  i2c_port_t i2c_port;
  SemaphoreHandle_t mu;
};

I2CAdapter* i2c_adapter_new(I2CAdapterConfig config) {
  i2c_config_t conf = {
      .mode = I2C_MODE_MASTER,
      .sda_io_num = config.sda_io,
      .sda_pullup_en = GPIO_PULLUP_ENABLE,
      .scl_io_num = config.scl_io,
      .scl_pullup_en = GPIO_PULLUP_ENABLE,
      .master.clk_speed = config.clk_speed,
  };

  esp_err_t err = i2c_param_config(config.port, &conf);
  if (err != ESP_OK) {
    ESP_LOGE(TAG, "Failed to configure I2C: %s", esp_err_to_name(err));
    return NULL;
  }

  err = i2c_driver_install(config.port, conf.mode, 0, 0, 0);
  if (err != ESP_OK) {
    ESP_LOGE(TAG, "Failed to install I2C driver: %s", esp_err_to_name(err));
    return NULL;
  }

  I2CAdapter* adapter = calloc(1, sizeof(I2CAdapter));
  if (!adapter) {
    ESP_LOGE(TAG, "Failed to allocate memory for I2C adapter");
    i2c_driver_delete(config.port);
    return NULL;
  }

  adapter->i2c_port = config.port;
  adapter->mu = xSemaphoreCreateMutex();
  if (!adapter->mu) {
    ESP_LOGE(TAG, "Failed to create mu");
    free(adapter);
    i2c_driver_delete(config.port);
    return NULL;
  }

  ESP_LOGI(TAG, "I2C initialized on port %d", config.port);
  return adapter;
}

void i2c_adapter_free(I2CAdapter* self) {
  if (!self)
    return;

  if (self->mu) {
    vSemaphoreDelete(self->mu);
  }
  i2c_driver_delete(self->i2c_port);
  free(self);

  ESP_LOGI(TAG, "I2C adapter destroyed");
}

esp_err_t i2c_adapter_write(I2CAdapter* self, uint8_t address, uint8_t* data, size_t data_len) {
  if (!self) {
    return ESP_ERR_INVALID_ARG;
  }
  if (xSemaphoreTake(self->mu, portMAX_DELAY) == pdTRUE) {
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (address << 1) | I2C_MASTER_WRITE, true);
    i2c_master_write(cmd, data, data_len, true);
    i2c_master_stop(cmd);

    esp_err_t ret = i2c_master_cmd_begin(self->i2c_port, cmd, pdMS_TO_TICKS(1000));
    i2c_cmd_link_delete(cmd);
    xSemaphoreGive(self->mu);

    if (ret != ESP_OK) {
      ESP_LOGE(TAG, "I2C write failed: %s", esp_err_to_name(ret));
    }
    return ret;
  } else {
    ESP_LOGE(TAG, "Failed to take mu for I2C write");
    return ESP_ERR_TIMEOUT;
  }
}

esp_err_t i2c_adapter_read(I2CAdapter* self, uint8_t address, uint8_t* data, size_t data_len) {
  if (!self) {
    return ESP_ERR_INVALID_ARG;
  }
  if (xSemaphoreTake(self->mu, portMAX_DELAY) == pdTRUE) {
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (address << 1) | I2C_MASTER_READ, true);
    i2c_master_read(cmd, data, data_len, I2C_MASTER_LAST_NACK);
    i2c_master_stop(cmd);

    esp_err_t err = i2c_master_cmd_begin(self->i2c_port, cmd, pdMS_TO_TICKS(1000));
    i2c_cmd_link_delete(cmd);
    xSemaphoreGive(self->mu);

    if (err != ESP_OK) {
      ESP_LOGE(TAG, "I2C read failed: %s", esp_err_to_name(err));
    }
    return err;
  } else {
    ESP_LOGE(TAG, "Failed to take mu for I2C read");
    return ESP_ERR_TIMEOUT;
  }
}
esp_err_t i2c_adapter_ping(I2CAdapter* self, uint8_t address) {
  if (!self) {
    return ESP_ERR_INVALID_ARG;
  }
  if (xSemaphoreTake(self->mu, portMAX_DELAY) == pdTRUE) {
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, address << 1 | I2C_MASTER_WRITE, I2C_MASTER_NACK);
    i2c_master_stop(cmd);
    esp_err_t err = i2c_master_cmd_begin(self->i2c_port, cmd, pdMS_TO_TICKS(1000));
    i2c_cmd_link_delete(cmd);
    xSemaphoreGive(self->mu);
    return err;
  } else {
    ESP_LOGE(TAG, "Failed to take mu for I2C read");
    return ESP_ERR_TIMEOUT;
  }
}

