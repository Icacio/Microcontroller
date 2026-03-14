idf_component_register(
    SRCS "main.c" "init.c" "device.c" "host.c"
    INCLUDE_DIRS "."
    PRIV_REQUIRES bt nvs_flash esp_hid
    REQUIRES driver
)

target_include_directories(${COMPONENT_LIB} PRIVATE
    ${IDF_PATH}/components/bt/common/include
    ${IDF_PATH}/components/bt/host/bluedroid/common/include
    ${IDF_PATH}/components/bt/host/bluedroid/stack/include
    ${IDF_PATH}/components/bt/host/bluedroid/stack/include/stack
    ${IDF_PATH}/components/bt/host/bluedroid/stack/sdp
)
