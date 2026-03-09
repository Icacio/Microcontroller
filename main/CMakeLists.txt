idf_component_register(SRCS "main.c" "init.c" "device.c" "host.c"
                    PRIV_REQUIRES bt nvs_flash esp_hid
                    INCLUDE_DIRS "."
    REQUIRES driver)
