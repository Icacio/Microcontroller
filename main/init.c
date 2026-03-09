#include "nvs_flash.h"
#include "esp_bt_main.h"
#include "esp_bt_device.h"
#include "esp_hidd_api.h"
#include "esp_gap_bt_api.h"
#include "nvs_flash.h"
#include "esp_mac.h"
#include "esp_log.h"
#include "esp_wifi.h"
#include "esp_bt.h"

void disable_wifi() {
    esp_wifi_stop();
    esp_wifi_deinit();
}


void init_bluetooth(esp_bt_gap_cb_t callback) {
    esp_bt_controller_config_t bt_cfg = BT_CONTROLLER_INIT_CONFIG_DEFAULT();
    esp_bt_controller_init(&bt_cfg);
    esp_bt_controller_enable(ESP_BT_MODE_CLASSIC_BT);

    esp_bluedroid_init();
    esp_bluedroid_enable();

    esp_bt_gap_register_callback(callback);
}
