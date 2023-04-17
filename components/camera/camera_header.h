/**
 * @file ultrasonic.h
 *
 * ESP-IDF driver for ultrasonic range meters, e.g. HC-SR04, HY-SRF05 and so on
 *
 * Ported from esp-open-rtos
 * Copyright (C) 2016, 2018 Ruslan V. Uss <unclerus@gmail.com>
 * BSD Licensed as described in the file LICENSE
 */
#ifndef __CAMERA_H__
#define __CAMERA_H__

#ifdef __cplusplus
extern "C" {
#endif

// #include <esp_system.h>
// #include <string.h>
// #include "freertos/FreeRTOS.h"
// #include "freertos/task.h"
// #include "driver/gpio.h"
// #include "esp_event.h"

// #include "esp_log.h"

#include "nvs_flash.h"
#include "esp_camera.h"
#include "esp_http_server.h"
#include "esp_timer.h"


#include "esp_system.h"
#include "esp_wifi.h"
#include "protocol_common.h"

// #include "lwip/err.h"
#include "lwip/sockets.h"
// #include "lwip/sys.h"
#include "lwip/netdb.h"
#include "lwip/dns.h"

// #include "esp_err.h"
#include "esp_intr_alloc.h"
#include "soc/soc_caps.h"
#include "freertos/semphr.h"
#include "freertos/queue.h"
#include "freertos/ringbuf.h"
#include "hal/uart_types.h"

static const char *TAG_CAM = "esp32-cam Webserver";

#define PART_BOUNDARY "123456789000000000000987654321"
static const char* _STREAM_CONTENT_TYPE = "multipart/x-mixed-replace;boundary=" PART_BOUNDARY;
static const char* _STREAM_BOUNDARY = "\r\n--" PART_BOUNDARY "\r\n";
static const char* _STREAM_PART = "Content-Type: image/jpeg\r\nContent-Length: %u\r\n\r\n";



esp_err_t init_camera(void);
httpd_handle_t setup_server(void);

#ifdef __cplusplus
}
#endif

#endif /* __CAMERA_H__ */
