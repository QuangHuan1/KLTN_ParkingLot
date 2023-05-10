#include "nvs_flash.h"
#include "driver/uart.h"
#include "ultrasonic.h"
#include "camera_header.h"
#include "protocol_common.h"
#include "datetime.h"

// #define ESP32
#define ESP32CAM

#define gateCode "MC00"
#define areaCode "MK00"

#define MAX_DISTANCE_CM 450 // 5m max // 450
#define UART_NUM_2  (2) /*!< UART port 2 */
#define BOUNDARY "X-ESPIDF_MULTIPART"

#define OFF 0
#define ON 1

typedef struct {
    char *web_server;
    char *web_port;
    char *web_path;
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
char hexStr[128];
uint8_t carCounter;


server server_infor;
bool allow_reader;
bool allow_camera;




struct addrinfo *res;
struct in_addr *addr;
int status;




