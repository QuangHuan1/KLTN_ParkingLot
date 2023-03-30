/* HTTP GET Example using plain POSIX sockets

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/

#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "protocol_examples_common.h"

#include "lwip/err.h"
#include "lwip/sockets.h"
#include "lwip/sys.h"
#include "lwip/netdb.h"
#include "lwip/dns.h"

#include "ultrasonic.h"

#include "esp_err.h"
#include "esp_intr_alloc.h"
#include "soc/soc_caps.h"
#include "freertos/semphr.h"
#include "freertos/queue.h"
#include "freertos/ringbuf.h"
#include "hal/uart_types.h"


// #include <ultrasonic.h>
/* Constants that aren't configurable in menuconfig */
// #define WEB_SERVER "api.thingspeak.com"
// #define WEB_PORT "80"
// #define WEB_PATH "/"


#define WEB_SERVER "192.168.87.7"
#define WEB_PORT "3000"

#define MAX_DISTANCE_CM 450 // 5m max // 450
#define GPIO_TRIGGER	13
#define GPIO_ECHO	12

static const char *TAG = "HTTP POST Wating";
static const char *DATA = "DATA";
static const char *TAG2 = "UltraSonic Wating";


static const int RX_BUF_SIZE = 1024;

#define UART_NUM_2             (2) /*!< UART port 2 */
#define TXD_PIN (GPIO_NUM_17)
#define RXD_PIN (GPIO_NUM_16)

// // static const char *REQUEST = "GET " WEB_PATH " HTTP/1.0\r\n"
// //     "Host: "WEB_SERVER":"WEB_PORT"\r\n"
// //     "User-Agent: esp-idf/1.0 esp32\r\n"
// //     "\r\n";

char REQUEST[512];
char SUBREQUEST[100];
char recv_buf[512];

int tagValue;
int gateID;
bool trigger = 0;
QueueHandle_t  q = NULL;

struct addrinfo *res;
struct in_addr *addr;
int s, r;

void init(void) {
    const uart_config_t uart_config = {
        .baud_rate = 115200,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        .source_clk = UART_SCLK_APB,
    };
    // We won't use a buffer for sending data.
    uart_driver_install(UART_NUM_2, RX_BUF_SIZE * 2, 0, 0, NULL, 0);
    uart_param_config(UART_NUM_2, &uart_config);
    uart_set_pin(UART_NUM_2, TXD_PIN, RXD_PIN, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);
}

int sendData(const char* logName, const char* data)
{
    const int len = strlen(data);
    const int txBytes = uart_write_bytes(UART_NUM_2, data, len);
    ESP_LOGI(logName, "Wrote %d bytes", txBytes);
    return txBytes;
}

static void tx_task(void *arg)
{
    static const char *TX_TASK_TAG = "TX_TASK";
    esp_log_level_set(TX_TASK_TAG, ESP_LOG_INFO);
    while (1) {
        sendData(TX_TASK_TAG, "Hello world\n");
        vTaskDelay(2000 / portTICK_PERIOD_MS);
    }
}

static void rx_task(void *arg)
{
    static const char *RX_TASK_TAG = "RX_TASK";
    esp_log_level_set(RX_TASK_TAG, ESP_LOG_INFO);
    uint8_t* data = (uint8_t*) malloc(RX_BUF_SIZE+1);
    while (1) {
        const int rxBytes = uart_read_bytes(UART_NUM_2, data, RX_BUF_SIZE, 1000 / portTICK_RATE_MS);

        if (rxBytes > 0) {
            data[rxBytes] = 0;
            ESP_LOGI(RX_TASK_TAG, "Read %d bytes: '%s'", rxBytes, data);
            ESP_LOG_BUFFER_HEXDUMP(RX_TASK_TAG, data, rxBytes, ESP_LOG_INFO);


        }
    }
    free(data);
}


static void ultrasonic(void *pvParamters)
{
	ultrasonic_sensor_t sensor = {
		.trigger_pin = GPIO_TRIGGER,
		.echo_pin = GPIO_ECHO
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
                        printf("Cannot ping (device is in invalid state)\n");
                        break;
                    case ESP_ERR_ULTRASONIC_PING_TIMEOUT:
                        printf("Ping timeout (no device found)\n");
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

        distance = avg_distance / 10;
        printf("distance %d", distance);
        if( distance < 20 ){
            trigger = 1;
            printf("Average Measurement Distance in %d times: %d cm\n", 10, distance);
        }


        // esp_err_t res = ultrasonic_measure_cm(&sensor, MAX_DISTANCE_CM, &distance);
        // if (res == ESP_OK) 
        // {
        //     avg_distance = avg_distance / 10;
        //     distance  = avg_distance;
        //     xQueueSend(q,(void *)&distance,(TickType_t )0); // add the value to the queue
        // }
        for(int countdown = 10; countdown >= 0; countdown--) 
        {
            ESP_LOGI(TAG2, "%d... ", countdown);
            vTaskDelay(1000 / portTICK_PERIOD_MS);
        }


	}
}



void connect_server(void *pvParameters){
    const struct addrinfo hints = {
        .ai_family = AF_INET,
        .ai_socktype = SOCK_STREAM,
    };
    struct addrinfo *res;
    struct in_addr *addr;
    int s, r;


    while(1) {
        int err = getaddrinfo(WEB_SERVER, WEB_PORT, &hints, &res);

        if(err != 0 || res == NULL) {
            ESP_LOGE(TAG, "DNS lookup failed err=%d res=%p", err, res);
            vTaskDelay(1000 / portTICK_PERIOD_MS);
            continue;
        }

        /* Code to print the resolved IP.

           Note: inet_ntoa is non-reentrant, look at ipaddr_ntoa_r for "real" code */
        addr = &((struct sockaddr_in *)res->ai_addr)->sin_addr;
        ESP_LOGI(TAG, "DNS lookup succeeded. IP=%s", inet_ntoa(*addr));

        s = socket(res->ai_family, res->ai_socktype, 0);
        if(s < 0) {
            ESP_LOGE(TAG, "... Failed to allocate socket.");
            freeaddrinfo(res);
            vTaskDelay(1000 / portTICK_PERIOD_MS);
            continue;
        }
        ESP_LOGI(TAG, "... allocated socket");

        if(connect(s, res->ai_addr, res->ai_addrlen) != 0) {
            ESP_LOGE(TAG, "... socket connect failed errno=%d", errno);
            close(s);
            freeaddrinfo(res);
            vTaskDelay(4000 / portTICK_PERIOD_MS);
            continue;
        }

        ESP_LOGI(TAG, "... connected");
        freeaddrinfo(res);

    }
}

static void http_get_task(void *pvParameters)
{

        tagValue = 0 + (int)(rand()*(100-0+1.0)/(1.0+RAND_MAX));
        gateID = 0 + (int)(rand()*(10-0+1.0)/(1.0+RAND_MAX));

        sprintf(SUBREQUEST, "{\"eTag\":\"%d\",\"gateCode\":\"%d\",\"image\":\"\"}", tagValue, gateID);
        printf("%s\n",SUBREQUEST);
        sprintf(REQUEST, "POST /check-in HTTP/1.1\nHost: 192.168.87.7:3000\nConnection: close\nContent-Type: application/json\nContent-Length:%d\n\n%s\n",strlen(SUBREQUEST), SUBREQUEST);

        //sprintf(REQUEST, "POST /check-in HTTP/1.1\nHost: 192.168.87.7:3000\n{api_key: \"api\", sensor_name: \"name\", temperature: value1, humidity: value2, pressure: value3}\nContent-Type: application/json");
        if(trigger == 1){
            if (write(s, REQUEST, strlen(REQUEST)) < 0) {
                ESP_LOGE(TAG, "... socket send failed");
                close(s);
                vTaskDelay(4000 / portTICK_PERIOD_MS);
                continue;
            }
            ESP_LOGI(TAG, "... socket send success");
            trigger = 0;
        }

        // struct timeval receiving_timeout;
        // receiving_timeout.tv_sec = 5;
        // receiving_timeout.tv_usec = 0;
        // if (setsockopt(s, SOL_SOCKET, SO_RCVTIMEO, &receiving_timeout,
        //         sizeof(receiving_timeout)) < 0) {
        //     ESP_LOGE(TAG, "... failed to set socket receiving timeout");
        //     close(s);
        //     vTaskDelay(4000 / portTICK_PERIOD_MS);
        //     continue;
        // }
        // ESP_LOGI(TAG, "... set socket receiving timeout success");
        

        // /* Read HTTP response */
        // do {
        //     bzero(recv_buf, sizeof(recv_buf));
        //     r = read(s, recv_buf, sizeof(recv_buf)-1);
        //     // ESP_LOGD(DATA, "Respone data length: %d", r);
        //     for(int i = 0; i < r; i++) {
        //         // if(recv_buf[i] == 10){
        //         //     break;
        //         // }
        //         putchar(recv_buf[i]);

        //     }
        // } while(r > 0);

        // ESP_LOGD(TAG, "... done reading from socket. Last read return=%d errno=%d.", r, errno);
        close(s);
        for(int countdown = 5; countdown >= 0; countdown--) {
            ESP_LOGI(TAG, "%d... ", countdown);
            vTaskDelay(1000 / portTICK_PERIOD_MS);
        }
        ESP_LOGI(TAG, "Starting again!");
    }
}

void app_main(void)
{
    ESP_ERROR_CHECK( nvs_flash_init() );
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    /* This helper function configures Wi-Fi or Ethernet, as selected in menuconfig.
     * Read "Establishing Wi-Fi or Ethernet Connection" section in
     * examples/protocols/README.md for more information about this function.
     */
    ESP_ERROR_CHECK(example_connect());
    // xTaskCreate(&http_get_task, "http_get_task", 4096, NULL, 5, NULL);
    xTaskCreate(&ultrasonic, "ultrasonic", 2048, NULL, 4, NULL);
    xTaskCreate(rx_task, "uart_rx_task", 1024*2, NULL, 4, NULL);


    
}
