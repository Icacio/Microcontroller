#include "nvs_flash.h"
#include "esp_bt_main.h"
#include "esp_bt_device.h"
#include "esp_hidh.h"
#include "esp_gap_bt_api.h"
#include "nvs_flash.h"
#include "esp_mac.h"
#include "esp_system.h"
#include "esp_log.h"
#include "init.h"

static void gaph_callback(esp_bt_gap_cb_event_t event, esp_bt_gap_cb_param_t *param);
void hidh_callback(void *handler_args,esp_event_base_t base,int32_t id,void *event_data);
void host(void) {
    //esp_log_level_set("*", ESP_LOG_NONE);
    init_bluetooth(gaph_callback);
    esp_hidh_config_t config = {
        .callback = hidh_callback,
    };

    esp_hidh_init(&config);

    esp_bt_gap_set_device_name("huele");

    set_cod();
    esp_bt_gap_start_discovery(ESP_BT_INQ_MODE_GENERAL_INQUIRY,5,0);
    while (1) {
        vTaskDelay(portMAX_DELAY);
    }
}
/*
static void parse_eir(uint8_t *eir) {
    uint8_t len;
    uint8_t *data;

    data = esp_bt_gap_resolve_eir_data(eir,ESP_BT_EIR_TYPE_CMPL_LOCAL_NAME,&len);
    if (data) {
        char name[64];
        memcpy(name, data, len);
        name[len] = 0;

        ESP_LOGI("SCAN", "EIR Name: %s", name);
    }
}
    */
static esp_bd_addr_t wiimote_addr;
static bool wiimote_found = false;
static void gaph_callback(esp_bt_gap_cb_event_t event, esp_bt_gap_cb_param_t *param) {
    ESP_LOGI("Evento", "evento GAP: %d", event);
    switch (event) {
    case ESP_BT_GAP_DISC_RES_EVT:
        for (int i = 0; i < param->disc_res.num_prop; i++) {
            esp_bt_gap_dev_prop_t *p = &param->disc_res.prop[i];
            if (p->type == ESP_BT_GAP_DEV_PROP_COD) {
                uint32_t cod = *(uint32_t *)p->val;
                if ((cod == 0x2504) && !wiimote_found) {
                    ESP_LOGI("SCAN", "Found a Wiimote device");
                    wiimote_found = true;
                    memcpy(wiimote_addr, param->disc_res.bda,sizeof(esp_bd_addr_t));
                } else {
                    ESP_LOGI("SCAN", "Found device with COD: %08x", cod);
                }
            }
        }
    break;
    case ESP_BT_GAP_RMT_SRVCS_EVT:
        ESP_LOGI("SCAN", "Found %d services", param->rmt_srvcs.num_uuids);
        for (int i = 0; i < param->disc_res.num_prop; i++) {/*
        esp_bt_gap_dev_prop_t *p = &param->disc_res.prop[i];
        switch(p->type) {
            case ESP_BT_GAP_DEV_PROP_BDNAME:
                ESP_LOGI("SCAN", "Device name: %s", (char *)p->val);
            break;
            case ESP_BT_GAP_DEV_PROP_COD:
                ESP_LOGI("SCAN", "Class of Device: %08x", *(uint32_t *)p->val);
            break;
            case ESP_BT_GAP_DEV_PROP_RSSI:
                ESP_LOGI("SCAN", "RSSI: %d", *(int8_t *)p->val);
            break;
            case ESP_BT_GAP_DEV_PROP_EIR:
                parse_eir((uint8_t *)p->val);
            break;
        }*/
    }

    break;
    case ESP_BT_GAP_PIN_REQ_EVT:
        uint8_t mac[6];
        esp_bt_pin_code_t pin = {0};
        esp_read_mac(mac, ESP_MAC_BT);
        for (int i = 0; i < 6; i++) {
            pin[i] = mac[5-i];
        }
        esp_bt_gap_pin_reply(param->pin_req.bda, true, 6, pin);
        ESP_LOGI("SCAN", "Pin code requested, replying with %02x%02x%02x%02x%02x%02x",
            pin[0],pin[1],pin[2],pin[3],pin[4],pin[5]);
    break;
    case ESP_BT_GAP_DISC_STATE_CHANGED_EVT:
        if (param->disc_st_chg.state == ESP_BT_GAP_DISCOVERY_STARTED)
            ESP_LOGI("SCAN","Discovery started");

        if (param->disc_st_chg.state == ESP_BT_GAP_DISCOVERY_STOPPED) {
            ESP_LOGI("SCAN","Discovery stopped");
            if (wiimote_found) {
                ESP_LOGI("SCAN", "Wiimote address: %02x:%02x:%02x:%02x:%02x:%02x",
                    wiimote_addr[0],wiimote_addr[1],wiimote_addr[2],
                    wiimote_addr[3],wiimote_addr[4],wiimote_addr[5]);
                esp_hidh_dev_open(wiimote_addr,ESP_HID_TRANSPORT_BT,true);
                esp_bt_gap_get_remote_services(wiimote_addr);
            }
        }
    break;
    case ESP_BT_GAP_ACL_CONN_CMPL_STAT_EVT:
        ESP_LOGI("SCAN", "ACL connection complete");
    break;
    case ESP_BT_GAP_ENC_CHG_EVT:
        ESP_LOGI("SCAN", "Encryption changed");
    break;
    default:
        ESP_LOGI("SCAN", "Event: %d", event);
    }
}

void hidh_callback(void *handler_args,esp_event_base_t base,int32_t id,void *event_data) {
    switch (id) {
    case ESP_HIDH_OPEN_EVENT:
        esp_hidh_event_data_t *evt = (esp_hidh_event_data_t *)event_data;
        if (evt->open.status == ESP_OK) {
            ESP_LOGI("HID", "Device opened successfully");
            const esp_hid_device_config_t *config = esp_hidh_dev_config_get(evt->open.dev);
            if (config)
            for (int j = 0; j < config->report_maps_len; j++) {
                const uint8_t *map_data = config->report_maps[j].data;
                size_t map_len = config->report_maps[j].len;
                printf("Report Map (hex): ");
                for (int i = 0; i < map_len; i++) {
                    printf("%02X ", map_data[i]);
                }
                printf("\n");
            }
        } else {
            ESP_LOGI("HID", "Failed to open device");
        }
    break;
    case ESP_HIDH_INPUT_EVENT:{
        esp_hidh_event_data_t *input_evt = (esp_hidh_event_data_t *)event_data;
        ESP_LOGI("HID", "Received input report: usage=%d, report_id=%d, length=%d",
            input_evt->input.usage, input_evt->input.report_id, input_evt->input.length);
            char hex[3*input_evt->input.length+1];
            for (int i = 0; i < input_evt->input.length; i++) {
                sprintf(hex+i*3, "%02x ", input_evt->input.data[i]);
            }
            ESP_LOGI("HID", "Input data (hex): %s", hex);
            printf("Input data (hex): %s\n", hex);
    }
    break;
    default:
        ESP_LOGI("HID", "Event: %d", id);
    }
}
