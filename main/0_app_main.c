/* 
    KLTN designed by Quyen DQ & Huan TQ
*/
#include "header.h"

server server_infor = {
    .web_server = "192.168.91.7",
    .web_port = "3000",
    .web_path = "/check-in-out-image/check-in"
};

bool allow_reader = OFF;
bool allow_camera = OFF;



// #ifdef ESP32
// uart_pin uart0 = {
//     .uart_num = UART_NUM_2,
//     .txd_pin = GPIO_NUM_17,
//     .rxd_pin = GPIO_NUM_16,
// };

// sensor_pin sensor0 = {
//     .sensor_trigger_pin = GPIO_NUM_13,
//     .sensor_echo_pin = GPIO_NUM_11,
// };

// gpio_serveral gpio0 = {
//     .reader_trigger_pin = GPIO_NUM_33
// };

// #endif

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
    ESP_ERROR_CHECK(nvs_flash_init() );
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    gpio_pad_select_gpio (gpio0.reader_trigger_pin);
    gpio_set_direction(gpio0.reader_trigger_pin, GPIO_MODE_OUTPUT);
    gpio_set_level(gpio0.reader_trigger_pin, 0);

    // gpio_pad_select_gpio (gpio0.gnd_extend);
    // gpio_set_direction(gpio0.gnd_extend, GPIO_MODE_OUTPUT);
    // gpio_set_level(gpio0.gnd_extend, 0);

    
    esp_err_t err;
    if (wifi_connect() == ESP_OK)
    {
        
        err = init_camera();        
        if (err != ESP_OK)
        {
            printf("err: %s\n", esp_err_to_name(err));
            return;
        }
        printf("CAMERA OK\n");
        ESP_ERROR_CHECK(init_uart());
        Set_SystemTime_SNTP();

        // ESP_ERROR_CHECK(setup_server());
        // xTaskCreate(&tx_task, "tx_task", 2048, NULL, 5, NULL);
        vTaskDelay(3000 / portTICK_PERIOD_MS);
        
        // xTaskCreate(&rx_task, "rx_task", 1024*2, NULL, 5, NULL);
        // xTaskCreate(&ultrasonic, "ultrasonic", 1024*3, NULL, 1, NULL);
        // xTaskCreate(&jpg_capture, "jpg_capture", 1024*4, NULL, 3, NULL);


        xTaskCreatePinnedToCore(&rx_task, "rx_task", 1024*2, NULL, 1, NULL, 1);
        xTaskCreatePinnedToCore(&ultrasonic, "ultrasonic", 1024*3, NULL, 1, NULL, 0);
        xTaskCreatePinnedToCore(&jpg_capture, "jpg_capture", 1024*4, NULL, 3, NULL, 0);



        // xTaskCreate(&http_post_image, "http_post_image", 2048*2, NULL, 3, NULL);
        // vTaskDelay(5000 / portTICK_PERIOD_MS);
        ESP_LOGI(TAG_CAM, "Tasks is created and running\n");
    }
    else
    ESP_LOGI(TAG_CAM, "Failed to connected with Wi-Fi\n");
}
