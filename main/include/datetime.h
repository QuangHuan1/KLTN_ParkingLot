#ifndef __DATETIME_H__
#define __DATETIME_H__

#include <time.h>
#include <sys/time.h>
#include "esp_attr.h"
#include "esp_sleep.h"
#include "esp_sntp.h"

//wifi
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"

#include "lwip/err.h"
#include "lwip/sys.h"



// #define EXAMPLE_ESP_WIFI_SSID      "toh8hc"
// #define EXAMPLE_ESP_WIFI_PASS      "24102001"

// static EventGroupHandle_t s_wifi_event_group;

/* The event group allows multiple bits for each event, but we only care about two events:
 * - we are connected to the AP with an IP
 * - we failed to connect after the maximum amount of retries */
#define WIFI_CONNECTED_BIT BIT0
#define WIFI_FAIL_BIT      BIT1

char Current_Date_Time[50];
char Current_Date_Time_Raw[50];


// void time_sync_notification_cb(struct timeval *tv);
// static void event_handler(void* arg, esp_event_base_t event_base,
//                                 int32_t event_id, void* event_data);
// void wifi_init_sta(void);


void Get_current_date_time(char *date_time, char *date_time_raw);
void initialize_sntp(void);
void obtain_time(void);
void Set_SystemTime_SNTP();

void rx_task(void *arg);
esp_err_t init_uart(void);

#endif /* __DATETIME_H__ */


