#include "init.h"
#include "driver/gpio.h"
#include "host.c"
#include "device.c"

void init_gpio(void);
void app_main(void) {
    nvs_flash_init();
    disable_wifi();

    init_gpio();
    bool client_mode = gpio_get_level(GPIO_NUM_4) == 0;
    if (!client_mode) {
        ESP_LOGI("MAIN", "Starting in client mode");
        host();
    } else {
        ESP_LOGI("MAIN", "Starting in device mode");
        device();
    }
}

void init_gpio(void) {
    gpio_config_t io_conf = {
      .pin_bit_mask = (1ULL << GPIO_NUM_4),
      .mode = GPIO_MODE_INPUT,
      .pull_up_en = GPIO_PULLUP_ENABLE,
      .pull_down_en = GPIO_PULLDOWN_DISABLE,
      .intr_type = GPIO_INTR_DISABLE
    };

    gpio_config(&io_conf);
}
