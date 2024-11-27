idf_component_register(
    SRCS
        "main.c"
    REQUIRES
        nvs_flash
        sx1280-driver-c
        esp_timer
)
