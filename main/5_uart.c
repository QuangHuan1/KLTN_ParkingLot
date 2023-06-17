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

    // esp_log_level_set(TAG_UART, ESP_LOG_INFO);
    uint8_t* rx_data = (uint8_t*) malloc(RX_BUF_SIZE+1);
    
    while (1) {

        if((PREP_CHECKIN == TRUE || PREP_CHECKOUT == TRUE) && allow_reader == TRUE && readtag_done == false){
            gpio_set_level(gpio0.reader_trigger_pin, 1);
            vTaskDelay(20/portTICK_PERIOD_MS);
            gpio_set_level(gpio0.reader_trigger_pin, 0);
            const int rxBytes = uart_read_bytes(uart0.uart_num, rx_data, RX_BUF_SIZE, 30 / portTICK_RATE_MS);
            ESP_LOGW(TAG_UART, "Read %d bytes\n", rxBytes);

            if (rxBytes > 0 && rxBytes < 25) {
                rx_data[rxBytes] = 0;
                ESP_LOG_BUFFER_HEXDUMP(TAG_UART, rx_data, rxBytes, ESP_LOG_INFO);

                int j = 0;
                uint8_t end = rxBytes - 2;
                uint8_t start = end - 12;
                for (int i = start; i < end; i++)
                {
                    sprintf(hexStr + j, "%02X", rx_data[i]);
                    j += 2;
                }

                hexStr[j] = '\0';
                ESP_LOGI(TAG_UART, "Hexa String: %s", hexStr);
                readtag_done = true;

            }
            vTaskDelay(DELAY_TIME / portTICK_PERIOD_MS);

        } else if ((DONE_CHECKIN == TRUE || DONE_CHECKOUT == TRUE) && postimage_done == true && readtag_done == true){
        // } else if (postimage_done == true && readtag_done == true){

            ESP_LOGI(TAG_CAM, "Uploading E-TAG data!");

            // #ifdef POSITION == GATE 
            //     #ifdef TYPE == CHECKIN
            //         if(DONE_CHECKIN == TRUE){
            //             http_post_tagdata(hexStr, server_infor.post_checkin_path);
            //         }
            //     #elif TYPE == CHECKOUT
            //         if(DONE_CHECKOUT == TRUE){
            //             http_post_tagdata(hexStr, server_infor.post_checkout_path);
            //         }
            //     #elif TYPE == CHECKIN_OUT
            //         if(DONE_CHECKIN == TRUE){
            //             http_post_tagdata(hexStr, server_infor.post_checkin_path);
            //         }
            //         if(DONE_CHECKOUT == TRUE){
            //             http_post_tagdata(hexStr, server_infor.post_checkout_path);
            //         }
            //     #endif
            // #elif POSITION == AREA
            //         if(DONE_CHECKIN == TRUE){
            //             http_post_tagdata(hexStr, server_infor.post_checkin_area_path);
            //         }
            //         if(DONE_CHECKOUT == TRUE){
            //             http_post_tagdata(hexStr, server_infor.post_checkout_area_path);
            //         }
            // #endif

                if(DONE_CHECKIN == TRUE){
                    http_post_tagdata(hexStr, server_infor.post_checkin_path);
                }
                if(DONE_CHECKOUT == TRUE){
                    http_post_tagdata(hexStr, server_infor.post_checkout_path);
                }

                allow_reader = OFF;
                readtag_done = FALSE;
                postimage_done = FALSE;
                postetag_done = TRUE;
        }
        vTaskDelay(DELAY_TIME / portTICK_PERIOD_MS);
    }
}

void http_post_tagdata(char *tagID, char *path)
{   
    const struct addrinfo hints = {
        .ai_family = AF_INET,
        .ai_socktype = SOCK_STREAM,
    };
    uint8_t count_loop = 0;
    while(count_loop <= 5) {
        count_loop++;
        int err = getaddrinfo(server_infor.web_server, server_infor.web_port, &hints, &res);
        if(err != 0 || res == NULL) {
            ESP_LOGE(TAG_POST, "DNS lookup failed err=%d res=%p", err, res);
            vTaskDelay(1000 / portTICK_PERIOD_MS);
            continue;
        }

        /* Code to print the resolved IP.
           Note: inet_ntoa is non-reentrant, look at ipaddr_ntoa_r for "real" code */
        addr = &((struct sockaddr_in *)res->ai_addr)->sin_addr;
        ESP_LOGI(TAG_POST, "DNS lookup succeeded. IP=%s", inet_ntoa(*addr));

        status = socket(res->ai_family, res->ai_socktype, 0);
        if(status < 0) {
            ESP_LOGE(TAG_POST, "... Failed to allocate socket.");
            freeaddrinfo(res);
            vTaskDelay(1000 / portTICK_PERIOD_MS);
            continue;
        }
        ESP_LOGI(TAG_POST, "... allocated socket");

        if(connect(status, res->ai_addr, res->ai_addrlen) != 0) {
            ESP_LOGE(TAG_POST, "... socket connect failed errno=%d", errno);
            close(status);
            freeaddrinfo(res);
            vTaskDelay(4000 / portTICK_PERIOD_MS);
            continue;
        }

        ESP_LOGI(TAG_POST, "... connected");
        freeaddrinfo(res);

        if(path == server_infor.post_checkin_path){
            sprintf(request_content, "{\"eTag\":\"%s\", \
                                       \"checkinTime\":\"%s\", \
                                       \"gateCode\":\"%s\", \
                                       \"areaCode\":\"%s\", \
                                       \"realAreaCode\":\"%s\", \
                                       \"imageCode\":\"%s\"}", 
                                   tagID, Current_Date_Time_Raw, 
                                   GATECODE_CHECKIN, AREACODE, REAL_AREACODE, Current_Date_Time);
        }
        else if(path == server_infor.post_checkout_path){
            sprintf(request_content, "{\"eTag\":\"%s\", \
                                       \"checkoutTime\":\"%s\", \
                                       \"gateCode\":\"%s\", \
                                       \"realAreaCode\":\"%s\", \
                                       \"imageCode\":\"%s\"}", 
                                   tagID, Current_Date_Time_Raw, 
                                   GATECODE_CHECKOUT, REAL_AREACODE, Current_Date_Time);
        }
        else if(path == server_infor.post_checkin_area_path){
            sprintf(request_content, "{\"eTag\":\"%s\", \
                                       \"areaCode\":\"%s\"}", 
                                       tagID, AREACODE);
        }
        else if(path == server_infor.post_checkout_area_path){
            sprintf(request_content, "{\"eTag\":\"%s\", \
                                       \"areaCode\":\"%s\"}", 
                                       tagID, AREACODE);
        }

        printf("%s\n",request_content);
        sprintf(request_msg, "POST %s HTTP/1.1\r\n"
                        "Host: %s:%s\r\n"
                        "Connection: close\r\n"
                        "Content-Type: application/json\r\n"
                        "Content-Length:%d\r\n"
                        "\n%s\r\n", path, 
                        server_infor.web_server, server_infor.web_port, 
                        strlen(request_content), 
                        request_content);

        if (write(status, request_msg, strlen(request_msg)) < 0) {
            ESP_LOGE(TAG_POST, "... socket send failed");
            close(status);
            vTaskDelay(4000 / portTICK_PERIOD_MS);
            continue;
        }
        ESP_LOGI(TAG_POST, "... socket send success");
        vTaskDelay(100 / portTICK_PERIOD_MS);
        break;
    }
}