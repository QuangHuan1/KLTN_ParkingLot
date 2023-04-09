#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"
// #include "protocol_examples_common.h"
#include "protocol_common.h"

#include "lwip/err.h"
#include "lwip/sockets.h"
#include "lwip/sys.h"
#include "lwip/netdb.h"
#include "lwip/dns.h"

#include "esp_err.h"
#include "esp_intr_alloc.h"
#include "soc/soc_caps.h"
#include "freertos/semphr.h"
#include "freertos/queue.h"
#include "freertos/ringbuf.h"
#include "hal/uart_types.h"

#include "driver/uart.h"
#include "driver/gpio.h"
#include "ultrasonic.h"


typedef struct {
    char *web_server;
    char *web_port;
} server;

typedef struct {
    uart_port_t uart_num;
    gpio_num_t txd_pin;
    gpio_num_t rxd_pin;
    gpio_num_t reader_trigger_pin;
    gpio_num_t sensor_trigger_pin;
    gpio_num_t sensor_echo;
    gpio_num_t gnd_extend;
} gpio;


static const char *TAG = "HTTP POST";
static const char *TAG2 = "Ultra Sonic";

static const int RX_BUF_SIZE = 1024;
char request_msg[1024];
char request_content[512];
char recv_buf[512];
char hexStr[512];


#define GATE_ID 1
#define MAX_DISTANCE_CM 450 // 5m max // 450
#define reader_off 0
#define reader_on 1
#define UART_NUM_2  (2) /*!< UART port 2 */


bool allow_reader = reader_off;
struct addrinfo *res;
struct in_addr *addr;
int status;

server server_infor = {
    .web_server = "192.168.190.7",
    .web_port = "3000",
};

gpio gpio_infor = {
    .uart_num = UART_NUM_2,
    .txd_pin = GPIO_NUM_17,
    .rxd_pin = GPIO_NUM_16,
    .reader_trigger_pin = GPIO_NUM_33,
    .sensor_trigger_pin = GPIO_NUM_13,
    .sensor_echo = GPIO_NUM_12,
    .gnd_extend = GPIO_NUM_33,
};
