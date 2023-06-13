#include "nvs_flash.h"
#include "driver/uart.h"
#include "ultrasonic.h"
#include "camera_header.h"
#include "protocol_common.h"
#include "datetime.h"
#include "stdbool.h"

// #define ESP32
#define ESP32CAM            // Define for Project using esp32cam as main MCU

//Where the MCU is used
#define GATE        0
#define AREA        1

//Determine whether the vehicle is entering or exiting
#define CHECKIN     0
#define CHECKOUT    1
#define CHECKIN_OUT    2


#define POSITION    GATE       //Place at GATE
#define TYPE        CHECKIN_OUT        //Checkin gate

// Declare Gate and Area Code Khu E
#define GATECODE_CHECKIN    "MC00"
#define GATECODE_CHECKOUT    "MC01"
#define AREACODE    "MK00"
#define REAL_AREACODE    "MK01"


// // Declare Gate and Area Code Khu B
// #define GATECODE_CHECKIN    "MC00"
// #define GATECODE_CHECKOUT    "MC01"
// #define AREACODE    "MK00"
// #define REAL_AREACODE    "MK02"


//Define distance for tracking car
#define MAX_DISTANCE_CM     450 // 5m max
#define FAR_THRESHOLD       60  //300
#define CAPTURE_THRESHOLD   30   ///175
#define NEAR_THRESHOLD      10      //50
#define THRESHOLD_OFFSET    5       //30

#define BOUNDARY "X-ESPIDF_MULTIPART"

#define OFF     0
#define ON      1

#define TRUE true
#define FALSE false

#define ERROR_THRESHOLD 10


// // For checkin state
// #define NO_CHECKIN      0
// #define SHALL_CHECKIN   1
// #define PREP_CHECKIN    2
// #define DONE_CHECKIN    3

// // For checkout state

// #define NO_CHECKOUT     0
// #define SHALL_CHECKOUT  1
// #define PREP_CHECKOUT   2
// #define DONE_CHECKOUT   3





#define TAG_WIFI    "FROM WIFI"
#define TAG_POST    "FROM HTTP POST"
#define TAG_SENSOR  "FROM UltraSonic"
#define TAG_UART    "FROM RX UART"
#define TAG_DATE    "FROM DATETIME"
#define TAG_CAM     "FROM CAMERA"

#define RX_BUF_SIZE 1024

// Web server infomation related
typedef struct {
    char *web_server;
    char *web_port;
    char *post_image_checkin_path;
    char *post_image_checkout_path;
    char *post_checkin_path;
    char *post_checkout_path;
    char *post_checkin_area_path;
    char *post_checkout_area_path;

} server;

// Declare UART PIN
typedef struct {
    uart_port_t uart_num;
    gpio_num_t txd_pin;
    gpio_num_t rxd_pin;
} uart_pin;

// Declare SENSOR PIN
typedef struct {
    gpio_num_t sensor_trigger_pin;
    gpio_num_t sensor_echo_pin;
} sensor_pin;

// Appendable PIN......
typedef struct {
    gpio_num_t reader_trigger_pin;
    gpio_num_t LED_Wifi_Status;
    gpio_num_t LED_Sensor_Status;
} gpio_serveral;


char request_msg[1024];
char request_content[512];
char recv_buf[512];
char hexStr[40];
uint8_t checkin_state;
uint8_t checkout_state;
uint8_t allow_reader;
uint8_t allow_camera;
uint8_t capture_done;
uint8_t readtag_done;
uint8_t postimage_done;
uint8_t postetag_done;
uint8_t ERROR_COUNT;


uint8_t NO_CHECKIN;
uint8_t SHALL_CHECKIN;
uint8_t PREP_CHECKIN;
uint8_t DONE_CHECKIN;

uint8_t NO_CHECKOUT;
uint8_t SHALL_CHECKOUT;
uint8_t PREP_CHECKOUT;
uint8_t DONE_CHECKOUT;



server server_infor;
uart_pin uart0;
sensor_pin sensor0;
gpio_serveral gpio0;


struct addrinfo *res;
struct in_addr *addr;

int status;

/**
 * @brief Post JSON data from MCU node to Server using HTTP POST method.
 *  
 * @param tagID Char pointer variable store EPC code that read from ETAG 
 * attached on CAR by UHF Reader.
 * E.g: E2 80 69 95 00 00 40 03 75 57 41 70 (in Hexa form)
 * 
 * @param path Char pointer variable store ROUTE in Server that determine 
 * JSON file will be POST using this PATH
 * E.g: "/check-in/check-in-area"
 */
void http_post_tagdata(char *tagID, char *path);





