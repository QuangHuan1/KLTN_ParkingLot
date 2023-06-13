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

#include "esp_camera.h"
#include "protocol_common.h"

#include "esp_http_server.h"
#include "esp_timer.h"
#include "esp_system.h"
#include "esp_wifi.h"

#include "lwip/sockets.h"
#include "lwip/netdb.h"
#include "lwip/dns.h"

#include "esp_intr_alloc.h"
#include "soc/soc_caps.h"
#include "freertos/semphr.h"
#include "freertos/queue.h"
#include "freertos/ringbuf.h"
#include "hal/uart_types.h"


#define CONFIG_XCLK_FREQ 20000000 

#define CAM_PIN_PWDN 32
#define CAM_PIN_RESET -1 //software reset will be performed
#define CAM_PIN_XCLK 0
#define CAM_PIN_SIOD 26
#define CAM_PIN_SIOC 27

#define CAM_PIN_D7 35
#define CAM_PIN_D6 34
#define CAM_PIN_D5 39
#define CAM_PIN_D4 36
#define CAM_PIN_D3 21
#define CAM_PIN_D2 19
#define CAM_PIN_D1 18
#define CAM_PIN_D0 5
#define CAM_PIN_VSYNC 25
#define CAM_PIN_HREF 23
#define CAM_PIN_PCLK 22


// #define PART_BOUNDARY "123456789000000000000987654321"
// static const char* _STREAM_CONTENT_TYPE = "multipart/x-mixed-replace;boundary=" PART_BOUNDARY;
// static const char* _STREAM_BOUNDARY = "\r\n--" PART_BOUNDARY "\r\n";
// static const char* _STREAM_PART = "Content-Type: image/jpeg\r\nContent-Length: %u\r\n\r\n";


/**
 * @brief Function call to INITIATE ESP32 CAMERA
 * 
 * @return ESP_OK on successful initiation
 */
esp_err_t initiate_camera(void);

/**
 * @brief Post IMAGE captured from MCU node to Server using HTTP POST method.
 *  
 * @param fb Object store camera frame buffer captured from ESP32CAM.
 * E.g: This is an image.
 * 
 * @param path Char pointer variable store ROUTE in Server that determine 
 * IMAGE will be POST using this PATH
 * E.g: "/check-in-out-image/check-in"
 */
void http_post_image(camera_fb_t *fb, char *path);

/**
 * @brief One of Main tasks that control Image capture task and OPLOAD 
 * this image to Server.
 */
void jpg_capture();


#ifdef __cplusplus
}
#endif

#endif /* __CAMERA_H__ */
