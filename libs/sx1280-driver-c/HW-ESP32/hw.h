#ifndef __HW_H__
#define __HW_H__

#include "esp_err.h"
#include "hw-config.h"
#include "hw-gpio.h"
#include "hw-spi.h"
#include "stdio.h"
#include <stdbool.h>
#include <stdint.h>

/*!
 * \brief Inits lora SX1280 hardware
 */
esp_err_t LoraHwInit(lora_hw_config_t *_config);

/*!
 * \brief Deinits lora SX1280 hardware
 */
esp_err_t LoraHwDeinit();

void HAL_Delay(uint32_t Delay);

#endif  // __HW_H__
