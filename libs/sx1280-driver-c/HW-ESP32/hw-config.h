
#ifndef __HW_CONFIG_H__
#define __HW_CONFIG_H__
#include "driver/spi_master.h"

/*!
 * \brief Struct containing informations about sx1280 module pinout.
 *
 */
typedef struct {
    int nss_pin;
    int mosi_pin;
    int miso_pin;
    int sclk_pin;
    int nreset_pin;
    int busy_pin;
    int dio1_pin;
    int dio2_pin;
    int dio3_pin;
    spi_host_device_t spi_host;
} lora_hw_config_t;

#endif  // __HW_GPIO_H__
