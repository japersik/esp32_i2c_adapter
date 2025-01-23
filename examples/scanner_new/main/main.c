#include <esp_log.h>

#include "japersik/esp32_i2c_adapter/i2c_master.h"

static const char *TAG = "SCANNER";

#define SDA_PORT GPIO_NUM_21
#define SCL_PORT GPIO_NUM_22
#define I2C_PORT I2C_NUM_0
#define I2C_FREQUENCY 400000

void app_main(void) {
  ESP_LOGI(TAG, "Scan for devices");
  I2CMasterAdapterConfig config = {
      .port = I2C_NUM_0,
      .scl_io = SCL_PORT,
      .sda_io = SDA_PORT,
  };
  I2CMasterAdapter *master = i2c_master_adapter_new(config);

  while (1) {
    esp_err_t res;
    printf("HEX  0  1  2  3  4  5  6  7  8  9  A  B  C  D  E  F\n");
    printf("00:         ");
    for (uint8_t i = 3; i < 0x78; i++) {
      if (i % 16 == 0) {
	printf("\n%.2x:", i);
      }

      res = i2c_master_adapter_ping(master, i);

      if (res == 0)
	printf(" %.2x", i);
      else
	printf(" ..");
    }
    printf("\n\n");
    vTaskDelay(pdMS_TO_TICKS(3000));
  }
}

