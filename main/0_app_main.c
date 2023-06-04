/* 
    KLTN designed by Quyen DQ & Huan TQ
*/
#include "header.h"

server server_infor = {
    .web_server = "192.168.1.6",
    .web_port = "3000",
    .web_path = "/check-in-out-image/check-in"
};

#ifdef ESP32CAM
uart_pin uart0 = {
    .uart_num = UART_NUM_0,
    .txd_pin = GPIO_NUM_1,
    .rxd_pin = GPIO_NUM_3,
};

sensor_pin sensor0 = {
    .sensor_trigger_pin = GPIO_NUM_13,
    .sensor_echo_pin = GPIO_NUM_15,
};

sensor_pin sensor1 = {
    .sensor_trigger_pin = GPIO_NUM_14,
    .sensor_echo_pin = GPIO_NUM_2,
};


gpio_serveral gpio0 = {
    .reader_trigger_pin = GPIO_NUM_12
};
#endif



void app_main(void)
{
    ESP_ERROR_CHECK(nvs_flash_init());
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    gpio_pad_select_gpio (gpio0.reader_trigger_pin);
    gpio_set_direction(gpio0.reader_trigger_pin, GPIO_MODE_OUTPUT);
    gpio_set_level(gpio0.reader_trigger_pin, 0);

    // gpio_pad_select_gpio (gpio0.gnd_extend);
    // gpio_set_direction(gpio0.gnd_extend, GPIO_MODE_OUTPUT);
    // gpio_set_level(gpio0.gnd_extend, 0);

    allow_reader = OFF;
    allow_camera = OFF;
    capture_done = OFF;
    readtag_done = OFF;
    postetag_done = false;
    postimage_done = false;
    esp_err_t err;
    if (wifi_connect() == ESP_OK)
    {
        if (init_camera() != ESP_OK)
        {
            printf("error while init camera: %s\n", esp_err_to_name(init_camera()));
            return;
        }
        ESP_ERROR_CHECK(init_uart());
        Set_SystemTime_SNTP();

        // ESP_ERROR_CHECK(setup_server());
        // xTaskCreate(&tx_task, "tx_task", 2048, NULL, 5, NULL);
        vTaskDelay(500 / portTICK_PERIOD_MS);
        
        xTaskCreatePinnedToCore(&ultrasonic, "ultrasonic", 1024*3, NULL, 1, NULL, 0);
        xTaskCreatePinnedToCore(&jpg_capture, "jpg_capture", 1024*4, NULL, 3, NULL, 0);
        xTaskCreatePinnedToCore(&rx_task, "rx_task", 1024*2, NULL, 1, NULL, 1);

        ESP_LOGI(TAG_CAM, "Tasks is created and running\n");
    }
    else
    ESP_LOGI(TAG_CAM, "Failed to connected with Wi-Fi\n");
}
