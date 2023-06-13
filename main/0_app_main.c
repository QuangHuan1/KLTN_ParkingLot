/* 
    KLTN designed by Quyen DQ & Huan TQ
*/
#include "header.h"

server server_infor = {
    .web_server = "192.168.1.3",
    .web_port = "3000",
    .post_image_checkin_path = "/check-in-out-image/check-in",
    .post_image_checkout_path = "/check-in-out-image/check-out",
    .post_checkin_path = "/check-in",
    .post_checkout_path = "/check-out",
    .post_checkin_area_path = "/check-in/check-in-area",
    .post_checkout_area_path = "/check-out/check-out-area"
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


gpio_serveral gpio0 = {
    .reader_trigger_pin = GPIO_NUM_12,
    .LED_Wifi_Status = GPIO_NUM_14,
    .LED_Sensor_Status = GPIO_NUM_2
};
#endif

void disable_mcu_pin(){

}


void app_main(void)
{
    ESP_ERROR_CHECK(nvs_flash_init());
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    
    gpio_pad_select_gpio (gpio0.reader_trigger_pin);
    gpio_set_direction(gpio0.reader_trigger_pin, GPIO_MODE_OUTPUT);
    gpio_set_level(gpio0.reader_trigger_pin, 0);

    gpio_pad_select_gpio (gpio0.LED_Wifi_Status);
    gpio_set_direction(gpio0.LED_Wifi_Status, GPIO_MODE_OUTPUT);
    gpio_set_level(gpio0.LED_Wifi_Status, 1);

    gpio_pad_select_gpio (gpio0.LED_Sensor_Status);
    gpio_set_direction(gpio0.LED_Sensor_Status, GPIO_MODE_OUTPUT);
    gpio_set_level(gpio0.LED_Sensor_Status, 1);


    allow_reader = OFF;
    allow_camera = OFF;
    capture_done = false;
    readtag_done = false;
    postetag_done = false;
    postimage_done = false;


    NO_CHECKIN = OFF;
    SHALL_CHECKIN = OFF;
    PREP_CHECKIN = OFF;
    DONE_CHECKIN = OFF;

    NO_CHECKOUT = OFF;
    SHALL_CHECKOUT = OFF;
    PREP_CHECKOUT = OFF;
    DONE_CHECKOUT = OFF;

    ERROR_COUNT = 0;

    if (wifi_connect() == ESP_OK)
    {
        gpio_set_level(gpio0.LED_Wifi_Status, 0);

        if (initiate_camera() != ESP_OK)
        {
            printf("error while init camera.\n");
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
    else{
        gpio_set_level(gpio0.LED_Wifi_Status, 1);
        ESP_LOGI(TAG_CAM, "Failed to connected with Wi-Fi\n");
    }
}
