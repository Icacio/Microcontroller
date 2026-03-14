#include "nvs_flash.h"
#include "esp_bt_main.h"
#include "esp_bt_device.h"
#include "esp_hidd_api.h"
#include "esp_gap_bt_api.h"
#include "nvs_flash.h"
#include "esp_mac.h"
#include "esp_log.h"
#include "init.h"

static void gapd_callback(esp_bt_gap_cb_event_t event, esp_bt_gap_cb_param_t *param);
static esp_hidd_app_param_t app_param;

void set_cod() {
    esp_bt_cod_t cod = {0};
    cod.major = ESP_BT_COD_MAJOR_DEV_PERIPHERAL;
    cod.minor = ESP_BT_COD_MINOR_PERIPHERAL_JOYSTICK;
    esp_bt_gap_set_cod(cod, ESP_BT_SET_COD_MAJOR_MINOR);
}

void init_hidd() {

    esp_bt_hid_device_init();

    esp_bt_pin_type_t pin_type = ESP_BT_PIN_TYPE_VARIABLE;
    esp_bt_pin_code_t pin_code;

    esp_bt_hid_device_register_callback(hidd_callback);
    esp_bt_gap_set_pin(pin_type, 0, pin_code);
}

static esp_hidd_qos_param_t qos_param = {
    .service_type = 0x01,
    .token_rate = 0,
    .token_bucket_size = 0,
    .peak_bandwidth = 0,
    .access_latency = 0,
    .delay_variation = 0
};

void device(void) {
    //esp_log_level_set("*", ESP_LOG_NONE);
    esp_log_level_set("HID", ESP_LOG_INFO);
    esp_log_level_set("Evento", ESP_LOG_DEBUG);
    esp_log_level_set("datos", ESP_LOG_DEBUG);
    init_bluetooth(gapd_callback);
    init_hidd();

    esp_bt_gap_set_device_name("Nintendo RVL-CNT-01");

    set_cod();

    esp_bt_gap_set_scan_mode(ESP_BT_CONNECTABLE, ESP_BT_LIMITED_DISCOVERABLE);
    while (1) {
        vTaskDelay(portMAX_DELAY);
    }
}

static const char *TAG = "HID";
void hidd_callback(esp_hidd_cb_event_t event, esp_hidd_cb_param_t *param) {
    ESP_LOGI(TAG, "HIDD EVENT: %d", event);
    switch (event) {
    case ESP_HIDD_INIT_EVT:
        ESP_LOGI(TAG, "HID INIT");
        esp_bt_hid_device_register_app(&app_param,&qos_param,&qos_param);
    break;
    case ESP_HIDD_SET_REPORT_EVT:
        char buf[256];
        for (int i = 0; i < param->set_report.len; i++) {
            buf[i] = param->set_report.data[i];
        }
        ESP_LOGI("datos","datos: %s", buf);
    break;
    case ESP_HIDD_REGISTER_APP_EVT:
        ESP_LOGI(TAG, "HID REGISTERED");
    break;

    case ESP_HIDD_OPEN_EVT:
        ESP_LOGI(TAG, "HOST CONNECTED");
    break;

    case ESP_HIDD_CLOSE_EVT:
        ESP_LOGI(TAG, "HOST DISCONNECTED");
    break;
    case ESP_HIDD_DEINIT_EVT:
        ESP_LOGI(TAG, "HID DEINIT");
    break;
    default:
        ESP_LOGI(TAG, "OTHER EVENT");
    break;
    }
}

static void gapd_callback(esp_bt_gap_cb_event_t event, esp_bt_gap_cb_param_t *param) {
    ESP_LOGI("Evento", "evento GAP: %d", event);
    switch (event) {
    case ESP_BT_GAP_ACL_CONN_CMPL_STAT_EVT:
        ESP_LOGI("Evento", "ESP_BT_GAP_ACL_CONN_CMPL_STAT_EVT");
        if (param->acl_conn_cmpl_stat.stat == ESP_BT_STATUS_SUCCESS) {
            ESP_LOGI("Evento", "ACL connection complete.");
        } else {
            ESP_LOGE("Evento", "ACL connection failed, status: %d", param->acl_conn_cmpl_stat.stat);
        }
    break;
    case ESP_BT_GAP_PIN_REQ_EVT://legacy pairing
        ESP_LOGI("Evento", "Legacy pairing: %d", event);
        esp_bt_pin_code_t pin;

        uint8_t *bda = param->pin_req.bda;

        for (int i = 0; i < 6; i++)
            pin[i] = bda[i];

        esp_bt_gap_pin_reply(param->pin_req.bda,true,6,pin);
    break;
    case ESP_BT_GAP_AUTH_CMPL_EVT: //authentication complete

        ESP_LOGI("BT", "status: %d", param->auth_cmpl.stat);
        ESP_LOGI("BT", "device: %s", param->auth_cmpl.device_name);

        ESP_LOG_BUFFER_HEX("BT", param->auth_cmpl.bda, ESP_BD_ADDR_LEN);

        ESP_LOGI("BT", "success: %s",
        param->auth_cmpl.stat == ESP_BT_STATUS_SUCCESS ? "true" : "false");

        if (param->auth_cmpl.stat == ESP_BT_STATUS_SUCCESS) {
            ESP_LOGI("Evento", "Authentication successful: %s", param->auth_cmpl.device_name);
        } else {
            ESP_LOGE("Evento", "Authentication failed, status: %d", param->auth_cmpl.stat);
        }
    break;
    case ESP_BT_GAP_CFM_REQ_EVT:
        ESP_LOGI("Evento", "Authenticating");
        esp_bt_gap_ssp_confirm_reply(param->cfm_req.bda, true);
    break;
    case ESP_BT_GAP_CONFIG_EIR_DATA_EVT:
        if (param->config_eir_data.stat == ESP_BT_STATUS_SUCCESS) {
            ESP_LOGI("Evento", "EIR data configured successfully");
        } else if (param->config_eir_data.stat == ESP_BT_STATUS_EIR_TOO_LARGE) {
            ESP_LOGW("Evento", "EIR data is too large, may not contain the whole data");
        } else {
            ESP_LOGE("Evento", "Failed to configure EIR data, status: %d", param->config_eir_data.stat);
        }
    break;
    case ESP_BT_GAP_ACL_DISCONN_CMPL_STAT_EVT:
        ESP_LOGI("Evento", "ACL disconnected");
    break;
    default:
        ESP_LOGI("Evento", "OTHER EVENT");
    break;
    }
}

static uint8_t hid_descriptor[] = {
    0x05, 0x01, 0x09, 0x05, 0xA1, 0x01,
    0x85, 0x10, 0x15, 0x00, 0x26, 0xFF,
    0x00, 0x75, 0x08, 0x95, 0x01, 0x06,
    0x00, 0xFF, 0x09, 0x01, 0x91, 0x00,
    0x85, 0x11, 0x95, 0x01, 0x09, 0x01,
    0x91, 0x00, 0x85, 0x12, 0x95, 0x02,
    0x09, 0x01, 0x91, 0x00, 0x85, 0x13,
    0x95, 0x01, 0x09, 0x01, 0x91, 0x00,
    0x85, 0x14, 0x95, 0x01, 0x09, 0x01,
    0x91, 0x00, 0x85, 0x15, 0x95, 0x01,
    0x09, 0x01, 0x91, 0x00, 0x85, 0x16,
    0x95, 0x15, 0x09, 0x01, 0x91, 0x00,
    0x85, 0x17, 0x95, 0x06, 0x09, 0x01,
    0x91, 0x00, 0x85, 0x18, 0x95, 0x15,
    0x09, 0x01, 0x91, 0x00, 0x85, 0x19,
    0x95, 0x01, 0x09, 0x01, 0x91, 0x00,
    0x85, 0x1A, 0x95, 0x01, 0x09, 0x01,
    0x91, 0x00, 0x85, 0x20, 0x95, 0x06,
    0x09, 0x01, 0x81, 0x00, 0x85, 0x21,
    0x95, 0x15, 0x09, 0x01, 0x81, 0x00,
    0x85, 0x22, 0x95, 0x04, 0x09, 0x01,
    0x81, 0x00, 0x85, 0x30, 0x95, 0x02,
    0x09, 0x01, 0x81, 0x00, 0x85, 0x31,
    0x95, 0x05, 0x09, 0x01, 0x81, 0x00,
    0x85, 0x32, 0x95, 0x0A, 0x09, 0x01,
    0x81, 0x00, 0x85, 0x33, 0x95, 0x11,
    0x09, 0x01, 0x81, 0x00, 0x85, 0x34,
    0x95, 0x15, 0x09, 0x01, 0x81, 0x00,
    0x85, 0x35, 0x95, 0x15, 0x09, 0x01,
    0x81, 0x00, 0x85, 0x36, 0x95, 0x15,
    0x09, 0x01, 0x81, 0x00, 0x85, 0x37,
    0x95, 0x15, 0x09, 0x01, 0x81, 0x00,
    0x85, 0x3D, 0x95, 0x15, 0x09, 0x01,
    0x81, 0x00, 0x85, 0x3E, 0x95, 0x15,
    0x09, 0x01, 0x81, 0x00, 0x85, 0x3F,
    0x95, 0x15, 0x09, 0x01, 0x81, 0x00,
    0xC0
};

static esp_hidd_app_param_t app_param = {
    .name = "Nintendo RVL-CNT-01",
    .description = "Nintendo RVL-CNT-01",
    .provider = "Nintendo",
    .subclass = 0x08,
    .desc_list = hid_descriptor,
    .desc_list_len = sizeof(hid_descriptor)
};
