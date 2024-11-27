#include "hw-spi.h"
#include "esp_log.h"
#include "freertos/semphr.h"

/* Private variables ---------------------------------------------------------*/
static spi_device_handle_t _lora_spi_handle;
static SemaphoreHandle_t mutex;

esp_err_t SpiInit(const lora_hw_config_t *config) {
    spi_bus_config_t bus = {.miso_io_num     = config->miso_pin,
                            .mosi_io_num     = config->mosi_pin,
                            .sclk_io_num     = config->sclk_pin,
                            .quadwp_io_num   = -1,
                            .quadhd_io_num   = -1,
                            .max_transfer_sz = 0};

    RET_ON_ERR(spi_bus_initialize(config->spi_host, &bus, SPI_DMA_CH_AUTO));

    spi_device_interface_config_t dev = {.clock_speed_hz = 9000000,  // Up to 18 MHz
                                         .mode           = 0,        // CPOL = 0, CPHA = 0
                                         .queue_size     = 3,
                                         .flags          = 0,
                                         .pre_cb         = NULL,
                                         .post_cb        = NULL};
    RET_ON_ERR(spi_bus_add_device(config->spi_host, &dev, &_lora_spi_handle));
    mutex = xSemaphoreCreateMutex();
    return ESP_OK;
}

esp_err_t SpiDeinit(const lora_hw_config_t *config) {
    RET_ON_ERR(spi_bus_remove_device(_lora_spi_handle));
    RET_ON_ERR(spi_bus_free(config->spi_host));
    vSemaphoreDelete(mutex);
    return ESP_OK;
}

/*!
 * @brief Sends txBuffer and receives rxBuffer
 *
 * @param [IN] txBuffer Byte to be sent
 * @param [OUT] rxBuffer Byte to be sent
 * @param [IN] size Byte to be sent
 */
void SpiInOut(uint8_t *txBuffer, uint8_t *rxBuffer, uint16_t size) {
    spi_transaction_t t = {
      .flags     = 0,
      .length    = 8 * size,
      .tx_buffer = txBuffer,
      .rx_buffer = rxBuffer,
    };
    if (xSemaphoreTake(mutex, portMAX_DELAY) == pdFALSE)
        return;
    spi_device_transmit(_lora_spi_handle, &t);
    xSemaphoreGive(mutex);
}

void SpiIn(uint8_t *txBuffer, uint16_t size) {
    spi_transaction_t t = {
      .flags     = 0,
      .length    = 8 * size,
      .tx_buffer = txBuffer,
      .rx_buffer = NULL,
    };
    if (xSemaphoreTake(mutex, portMAX_DELAY) == pdFALSE)
        return;
    spi_device_transmit(_lora_spi_handle, &t);
    xSemaphoreGive(mutex);
}