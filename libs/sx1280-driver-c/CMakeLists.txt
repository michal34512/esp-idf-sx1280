idf_component_register(
    SRCS "SX1280/sx1280-hal.c" "SX1280/sx1280.c" "HW-ESP32/hw-gpio.c" "HW-ESP32/hw-gpio-isr-daemon.c" "HW-ESP32/hw-spi.c" "HW-ESP32/hw.c"
    INCLUDE_DIRS "." "rangingCorrection" "HW-ESP32" "SX1280"
    REQUIRES
        driver
    )