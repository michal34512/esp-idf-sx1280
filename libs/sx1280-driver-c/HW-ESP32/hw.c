#include "hw.h"

#include "esp_check.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "sx1280-hal.h"
#include "sx1280.h"

#define TAG "LORA"

static lora_hw_config_t config;

esp_err_t LoraHwInit(lora_hw_config_t *_config) {
    config = *_config;
    SX1280SetHwConfig(&config);
    ESP_RETURN_ON_ERROR(GpioInit(&config), TAG, "Gpio init");
    ESP_RETURN_ON_ERROR(SpiInit(&config), TAG, "Spi init");
    return ESP_OK;
}

esp_err_t LoraHwDeinit() {
    ESP_RETURN_ON_ERROR(GpioDeinit(&config), TAG, "Gpio deinit");
    ESP_RETURN_ON_ERROR(SpiDeinit(&config), TAG, "Spi deinit");
    return ESP_OK;
}

void HAL_Delay(uint32_t Delay) {
    vTaskDelay(pdMS_TO_TICKS(Delay));
}