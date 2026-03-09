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

void disable_wifi();

void hidd_callback(esp_hidd_cb_event_t event, esp_hidd_cb_param_t *param);

void init_bluetooth(esp_bt_gap_cb_t callback);

void init_hidd();

void set_cod();
