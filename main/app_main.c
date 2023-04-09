/* 
    KLTN designed by Quyen DQ & Huan TQ
*/
#include "header.h"

void http_post_task(char *paraA, int paraB);
esp_err_t init_uart(void) {
    const uart_config_t uart_config = {
        .baud_rate = 115200,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        .source_clk = UART_SCLK_APB,
    };
    // We won't use a buffer for sending data.
    ESP_ERROR_CHECK(uart_driver_install(gpio_infor.uart_num, RX_BUF_SIZE * 2, 0, 0, NULL, 0));
    ESP_ERROR_CHECK(uart_param_config(gpio_infor.uart_num, &uart_config));
    ESP_ERROR_CHECK(uart_set_pin(gpio_infor.uart_num, gpio_infor.txd_pin, gpio_infor.rxd_pin, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE));
    return ESP_OK;
}

// int sendData(const char* logName, const char* data)
// {
//     const int len = strlen(data);
//     const int txBytes = uart_write_bytes(UART_NUM_2, data, len);
//     ESP_LOGI(logName, "Wrote %d bytes", txBytes);
//     return txBytes;
// }

// static void tx_task(void *arg)
// {
//     static const char *TX_TASK_TAG = "TX_TASK";
//     esp_log_level_set(TX_TASK_TAG, ESP_LOG_INFO);
//     while (1) {
//         sendData(TX_TASK_TAG, "Hello world\n");
//         vTaskDelay(2000 / portTICK_PERIOD_MS);
//     }
// }

static void rx_task(void *arg)
{   
    static const char *RX_TASK_TAG = "RX_TASK";
    esp_log_level_set(RX_TASK_TAG, ESP_LOG_INFO);
    uint8_t* rx_data = (uint8_t*) malloc(RX_BUF_SIZE+1);
    while (1) {
        const int rxBytes = uart_read_bytes(gpio_infor.uart_num, rx_data, RX_BUF_SIZE, 1000 / portTICK_RATE_MS);
        printf("bytes %d, Allow reader %d\n", rxBytes, allow_reader);
        if (rxBytes > 0 && allow_reader == reader_on) {
            rx_data[rxBytes] = 0;
            ESP_LOGI(RX_TASK_TAG, "Read %d bytes: '%s'", rxBytes, rx_data);
            ESP_LOG_BUFFER_HEXDUMP(RX_TASK_TAG, rx_data, rxBytes, ESP_LOG_INFO);

            int j = 0;
            for (int i = 0; i < rxBytes; i++)
            {
                sprintf(hexStr + j, "%02X", rx_data[i]);
                j += 2;
            }

            hexStr[j] = '\0';
            ESP_LOGI(RX_TASK_TAG, "Hexa String: %s", hexStr);
            http_post_task(hexStr, GATE_ID);
            allow_reader = reader_off;

        }
    }
    free(rx_data);
}


static void ultrasonic(void *pvParamters)
{
	ultrasonic_sensor_t sensor = {
		.trigger_pin = gpio_infor.sensor_trigger_pin,
		.echo_pin = gpio_infor.sensor_echo,
	};

	ultrasonic_init(&sensor);
    uint32_t distance = 0;
	while (true) {
    uint32_t avg_distance = 0;
    int index_loop = 1;
    while(index_loop <= 10){
            esp_err_t res = ultrasonic_measure_cm(&sensor, MAX_DISTANCE_CM, &distance);
            if (res != ESP_OK) {
                printf("Error: ");
                switch (res) {
                    case ESP_ERR_ULTRASONIC_PING:
                        ESP_LOGW(TAG2, "Cannot ping (device is in invalid state)\n");
                        break;
                    case ESP_ERR_ULTRASONIC_PING_TIMEOUT:
                        ESP_LOGW(TAG2, "Ping timeout (no device found)\n");
                        break;
                    case ESP_ERR_ULTRASONIC_ECHO_TIMEOUT:
                        printf("Echo timeout (i.e. distance too big)\n");
                        break;
                    default:
                        printf("%d\n", res);
                }
            } else {

        avg_distance +=  distance;
        index_loop++;
            }
        }

        avg_distance = avg_distance / 10;
        ESP_LOGE(TAG2, "distance %d", avg_distance);
        if( avg_distance < 20 ){
            allow_reader = reader_on;
            ESP_LOGI(TAG2, "Average Measurement Distance in %d times: %d cm\n", 10, distance);
            gpio_set_level(gpio_infor.reader_trigger_pin, 0);
            vTaskDelay(1000/portTICK_PERIOD_MS);
            gpio_set_level(gpio_infor.reader_trigger_pin, 1);

        }
        vTaskDelay(2000 / portTICK_PERIOD_MS);
	}
}


 void http_post_task(char *tagID, int gateID)
{   
    printf("server\n");
    const struct addrinfo hints = {
        .ai_family = AF_INET,
        .ai_socktype = SOCK_STREAM,
    };

    while(1) {
        int err = getaddrinfo(server_infor.web_server, server_infor.web_port, &hints, &res);
        if(err != 0 || res == NULL) {
            ESP_LOGE(TAG, "DNS lookup failed err=%d res=%p", err, res);
            vTaskDelay(1000 / portTICK_PERIOD_MS);
            continue;
        }

        /* Code to print the resolved IP.
           Note: inet_ntoa is non-reentrant, look at ipaddr_ntoa_r for "real" code */
        addr = &((struct sockaddr_in *)res->ai_addr)->sin_addr;
        ESP_LOGI(TAG, "DNS lookup succeeded. IP=%s", inet_ntoa(*addr));

        status = socket(res->ai_family, res->ai_socktype, 0);
        if(status < 0) {
            ESP_LOGE(TAG, "... Failed to allocate socket.");
            freeaddrinfo(res);
            vTaskDelay(1000 / portTICK_PERIOD_MS);
            continue;
        }
        ESP_LOGI(TAG, "... allocated socket");

        if(connect(status, res->ai_addr, res->ai_addrlen) != 0) {
            ESP_LOGE(TAG, "... socket connect failed errno=%d", errno);
            close(status);
            freeaddrinfo(res);
            vTaskDelay(4000 / portTICK_PERIOD_MS);
            continue;
        }

        ESP_LOGI(TAG, "... connected");
        freeaddrinfo(res);

        sprintf(request_content, "{\"eTag\":\"%s\",\"gateCode\":\"%d\",\"image\":\"\"}", tagID, gateID);
        printf("%s\n",request_content);
        sprintf(request_msg, "POST /check-in HTTP/1.1\r\n"
                        "Host: 192.168.190.7:3000\r\n"
                        "Connection: close\r\n"
                        "Content-Type: application/json\r\n"
                        "Content-Length:%d\r\n"
                        "\n%s\r\n", strlen(request_content), request_content);

        if (write(status, request_msg, strlen(request_msg)) < 0) {
            ESP_LOGE(TAG, "... socket send failed");
            close(status);
            vTaskDelay(4000 / portTICK_PERIOD_MS);
            continue;
        }
        ESP_LOGI(TAG, "... socket send success");
        vTaskDelay(5000 / portTICK_PERIOD_MS);
        ESP_LOGI(TAG, "Starting again!");
        break;
    }
}

void app_main(void)
{
    ESP_ERROR_CHECK(nvs_flash_init() );
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    gpio_pad_select_gpio (gpio_infor.reader_trigger_pin);
    gpio_set_direction(gpio_infor.reader_trigger_pin, GPIO_MODE_OUTPUT);
    gpio_set_level(gpio_infor.reader_trigger_pin, 1);

    gpio_pad_select_gpio (gpio_infor.gnd_extend);
    gpio_set_direction(gpio_infor.gnd_extend, GPIO_MODE_OUTPUT);
    gpio_set_level(gpio_infor.gnd_extend, 0);


    ESP_ERROR_CHECK(wifi_connect());
    // ESP_ERROR_CHECK(example_connect());

    ESP_ERROR_CHECK(init_uart());
    xTaskCreate(&ultrasonic, "ultrasonic", 2048, NULL, 5, NULL);
    xTaskCreate(&rx_task, "rx_task", 1024*2, NULL, 4, NULL);

}
