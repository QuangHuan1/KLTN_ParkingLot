#include "header.h"


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
    ESP_ERROR_CHECK(uart_driver_install(uart0.uart_num, RX_BUF_SIZE * 2, 0, 0, NULL, 0));
    ESP_ERROR_CHECK(uart_param_config(uart0.uart_num, &uart_config));
    ESP_ERROR_CHECK(uart_set_pin(uart0.uart_num, uart0.txd_pin, uart0.rxd_pin, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE));
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

void rx_task(void *arg)
{   

    esp_log_level_set(RX_TASK_TAG, ESP_LOG_INFO);
    uint8_t* rx_data = (uint8_t*) malloc(RX_BUF_SIZE+1);
    
    while (1) {
        const int rxBytes = uart_read_bytes(uart0.uart_num, rx_data, RX_BUF_SIZE, 1000 / portTICK_RATE_MS);
        if (rxBytes > 0 && allow_reader == ON) {
            rx_data[rxBytes] = 0;
            // ESP_LOGI(RX_TASK_TAG, "Read %d bytes: '%s'", rxBytes, rx_data);
            ESP_LOG_BUFFER_HEXDUMP(RX_TASK_TAG, rx_data, rxBytes, ESP_LOG_INFO);

            int j = 0;
            for (int i = 0; i < rxBytes; i++)
            {
                sprintf(hexStr + j, "%02X", rx_data[i]);
                j += 2;
            }

            hexStr[j] = '\0';
            ESP_LOGI(RX_TASK_TAG, "-------Hexa String: %s", hexStr);
            // if(car_entry == 2){
            //     http_post_task(hexStr);
            //     car_entry = 0;
            // }

            // if (checkin_state == DONE_CHECKIN){
            //     http_post_task(hexStr);
            //     checkin_state = NO_CHECKIN;
            // }

            // if (checkout_state == DONE_CHECKOUT){
            //     http_post_task(hexStr);
            //     checkout_state = NO_CHECKOUT;
            // }
            allow_reader = OFF;

        }
    }
}

void http_post_task(char *tagID)
{   
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

        sprintf(request_content, "{\"eTag\":\"%s\",\"checkinTime\":\"%s\",\"gateCode\":\"%s\",\"areaCode\":\"%s\",\"imageCode\":\"%s\"}", tagID, Current_Date_Time_Raw, gateCode, areaCode, Current_Date_Time);
        printf("%s\n",request_content);
        sprintf(request_msg, "POST /check-in HTTP/1.1\r\n"
                        "Host: 192.168.91.7:3000\r\n"
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
        vTaskDelay(2000 / portTICK_PERIOD_MS);
        ESP_LOGI(TAG, "Starting again!");
        break;
    }
}