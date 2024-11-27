#ifndef __HW_SPI_H__
#define __HW_SPI_H__

#include "hw-common.h"
#include "hw-config.h"
#include <stdint.h>


esp_err_t SpiInit(const lora_hw_config_t *config);

esp_err_t SpiDeinit(const lora_hw_config_t *config);

void SpiInOut(uint8_t *txBuffer, uint8_t *rxBuffer, const uint16_t size);

void SpiIn(uint8_t *txBuffer, const uint16_t size);

#endif  // __HW_SPI_H__
