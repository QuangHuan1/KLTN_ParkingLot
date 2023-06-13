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



/* The event group allows multiple bits for each event, but we only care about two events:
 * - we are connected to the AP with an IP
 * - we failed to connect after the maximum amount of retries */
#define WIFI_CONNECTED_BIT BIT0
#define WIFI_FAIL_BIT      BIT1

char Current_Date_Time[50];
char Current_Date_Time_Raw[50];

/**
 * @brief Get current Datetime in two types are Raw and Filtered.
 *  
 * @param date_time Char pointer variable that will returned Datetime 
 * value in filtered type.
 * E.g: 2023-06-04T21:26:34.000+00:00
 * 
 * @param date_time_raw Char pointer variable that will returned Datetime 
 * value in raw type.
 * E.g: 20230604T214126000Z
 * 
 */
void Get_current_date_time(char *date_time, char *date_time_raw);
void initialize_sntp(void);
void obtain_time(void);
void Set_SystemTime_SNTP();

void rx_task(void *arg);
esp_err_t init_uart(void);

#endif /* __DATETIME_H__ */


