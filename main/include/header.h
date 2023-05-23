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

#define IDLE 0
#define CHECKIN 1
#define CHEKCOUT 2

#define NO_CHECKIN 0
#define SHALL_CHECKIN 1
#define PREP_CHECKIN 2
#define DONE_CHECKIN 3


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


typedef struct {
    uart_port_t uart_num;
    gpio_num_t txd_pin;
    gpio_num_t rxd_pin;
} uart_pin;

typedef struct {
    gpio_num_t sensor_trigger_pin;
    gpio_num_t sensor_echo_pin;
} sensor_pin;

// Appendable......
typedef struct {
    gpio_num_t reader_trigger_pin;
} gpio_serveral;


static const char *TAG = "HTTP POST";
static const char *TAG2 = "Ultra Sonic";
static const char *RX_TASK_TAG = "RX_TASK";
static const int RX_BUF_SIZE = 1024;

char request_msg[1024];
char request_content[512];
char recv_buf[512];
char hexStr[128];
uint8_t carCounter;
uint8_t car_entry;
uint8_t checkin_state;


server server_infor;
uart_pin uart0;
sensor_pin sensor0;
sensor_pin sensor1;
gpio_serveral gpio0;

bool allow_reader;
bool allow_camera;
bool read_tag_ok;
bool car_comein;
bool car_status;
bool checkin_status;



struct addrinfo *res;
struct in_addr *addr;
int status;

void http_post_task(char *paraA);





