#ifndef __HW_GPIO_H__
#define __HW_GPIO_H__

#include "driver/gpio.h"
#include "hw-common.h"
#include "hw-config.h"
#include "hw-gpio-isr-daemon.h"

esp_err_t GpioInit(lora_hw_config_t *config);

esp_err_t GpioDeinit(lora_hw_config_t *config);

esp_err_t GpioWrite(uint16_t GPIO_Pin, uint32_t value);

uint32_t GpioRead(uint16_t GPIO_Pin);

esp_err_t GpioSetIrq(gpio_num_t gpio_num, uint32_t prio, GpioIrqHandler irqHandler);

void GpioLaunchIrqHandler(uint16_t GPIO_Pin);

#endif  // __HW_GPIO_H__
