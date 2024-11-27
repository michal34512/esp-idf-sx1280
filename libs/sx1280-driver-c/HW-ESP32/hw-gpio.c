#include "hw-gpio.h"

static void IRAM_ATTR gpio_isr_handler(void *arg) {
    HwGpioHandleInterruptFromIsr((uint32_t)arg);
}

esp_err_t GpioInit(lora_hw_config_t *config) {
    RET_ON_ERR(gpio_reset_pin(config->nreset_pin));
    RET_ON_ERR(gpio_set_direction(config->nreset_pin, GPIO_MODE_OUTPUT));
    RET_ON_ERR(gpio_reset_pin(config->nss_pin));
    RET_ON_ERR(gpio_set_direction(config->nss_pin, GPIO_MODE_OUTPUT));
    RET_ON_ERR(gpio_set_level(config->nss_pin, 1));
    HwGpioIsrDaemonInit();
    return ESP_OK;
}

esp_err_t GpioDeinit(lora_hw_config_t *config) {
    RET_ON_ERR(gpio_reset_pin(config->nreset_pin));
    RET_ON_ERR(gpio_reset_pin(config->nss_pin));
    HwGpioIsrDaemonDeinit();
    return ESP_OK;
}

esp_err_t GpioSetIrq(gpio_num_t gpio_num, uint32_t prio, GpioIrqHandler irqHandler) {
    if (irqHandler != NULL) {
        HwGpioSetupInterruptHandler(irqHandler, gpio_num);
        gpio_config_t io_conf = {.pin_bit_mask = (1ULL << gpio_num),
                                 .mode         = GPIO_MODE_INPUT,
                                 .pull_up_en   = GPIO_PULLUP_DISABLE,
                                 .pull_down_en = GPIO_PULLDOWN_ENABLE,
                                 .intr_type    = GPIO_INTR_POSEDGE};
        RET_ON_ERR(gpio_config(&io_conf));
        RET_ON_ERR(gpio_install_isr_service(ESP_INTR_FLAG_IRAM));
        RET_ON_ERR(gpio_isr_handler_add(gpio_num, gpio_isr_handler, (void *)gpio_num));
    }
    return ESP_OK;
}

/*!
 * @brief Writes the given value to the GPIO output
 *
 * @param  GPIO_Pin: specifies the port bit to be written.
 *                   This parameter can be one of GPIO_PIN_x where x can be (0..15).
 *                   All port bits are not necessarily available on all GPIOs.
 * @param [IN] value New GPIO output value
 * @retval none
 */
esp_err_t GpioWrite(uint16_t GPIO_Pin, uint32_t value) {
    RET_ON_ERR(gpio_set_level((gpio_num_t)GPIO_Pin, value));
    return ESP_OK;
}

/*!
 * @brief Reads the current GPIO input value
 *
 * @param  GPIOx: where x can be (A..E and H)
 * @param  GPIO_Pin: specifies the port bit to be written.
 *                   This parameter can be one of GPIO_PIN_x where x can be (0..15).
 *                   All port bits are not necessarily available on all GPIOs.
 * @retval value   Current GPIO input value
 */
uint32_t GpioRead(uint16_t GPIO_Pin) {
    return gpio_get_level((gpio_num_t)GPIO_Pin);
}
