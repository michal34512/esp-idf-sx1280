#ifndef __HW_COMMON_H__
#define __HW_COMMON_H__

#include "esp_err.h"

#define RET_ON_ERR(X)        \
    do {                     \
        esp_err_t err = (X); \
        if (err != ESP_OK) { \
            return err;      \
        }                    \
    } while (0)

#endif  // __HW_COMMON_H__
