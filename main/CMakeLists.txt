idf_component_register(
    SRCS
        "config_store.cpp"
        "loom.cpp"
        "loom_info.cpp"
        "main.cpp"
        "web_server.cpp"
        "wifi_info.cpp"
    PRIV_REQUIRES
        button_handler
        circular_deque
        dns_server
        esp_driver_gpio
        esp_event
        esp_http_server
        esp_wifi
        json
        mbedtls
        nvs_flash
        ssd1306
    INCLUDE_DIRS
        "."
)

# Note: you must have a partition named the first argument (here it's "littlefs")
# in your partition table csv file.
littlefs_create_partition_image(littlefs ../flash_data FLASH_IN_PROJECT)
