// #include "datetime.h"
#include "header.h"

void time_sync_notification_cb(struct timeval *tv)
{
    ESP_LOGI(TAG_DATE, "Notification of a time synchronization event");
}

void Get_current_date_time(char *date_time, char *date_time_raw){
	char strftime_buf[64];
	char strftime_buf_raw[64];

	time_t now;
	    struct tm timeinfo;
	    time(&now);
	    localtime_r(&now, &timeinfo);

	    	// Set timezone to Indian Standard Time
	    	    setenv("TZ", "UTC-07", 1);
	    	    tzset();
	    	    localtime_r(&now, &timeinfo);

	    	    strftime(strftime_buf, sizeof(strftime_buf), "%G%m%dT%H%M%S000Z", &timeinfo);
	    	    strftime(strftime_buf_raw, sizeof(strftime_buf_raw), "%FT%T.000Z", &timeinfo);

	    	    // ESP_LOGI(TAG_DATE, "The current date/time in Delhi is: %s", strftime_buf);
                strcpy(date_time,strftime_buf);
                strcpy(date_time_raw,strftime_buf_raw);

}


void initialize_sntp(void)
{
    ESP_LOGI(TAG_DATE, "Initializing SNTP");
    sntp_setoperatingmode(SNTP_OPMODE_POLL);
    sntp_setservername(0, "pool.ntp.org");
    sntp_set_time_sync_notification_cb(time_sync_notification_cb);
#ifdef CONFIG_SNTP_TIME_SYNC_METHOD_SMOOTH
    sntp_set_sync_mode(SNTP_SYNC_MODE_SMOOTH);
#endif
    sntp_init();
}
void obtain_time(void)
{


    initialize_sntp();
    // wait for time to be set
    time_t now = 0;
    struct tm timeinfo = { 0 };
    int retry = 0;
    const int retry_count = 10;
    while (sntp_get_sync_status() == SNTP_SYNC_STATUS_RESET && ++retry < retry_count) {
        ESP_LOGI(TAG_DATE, "Waiting for system time to be set... (%d/%d)", retry, retry_count);
        vTaskDelay(2000 / portTICK_PERIOD_MS);
    }
    time(&now);
    localtime_r(&now, &timeinfo);
}
 void Set_SystemTime_SNTP()  {

	 time_t now;
	    struct tm timeinfo;
	    time(&now);
	    localtime_r(&now, &timeinfo);
	    // Is time set? If not, tm_year will be (1970 - 1900).
	    if (timeinfo.tm_year < (2016 - 1900)) {
	        ESP_LOGI(TAG_DATE, "Time is not set yet. Connecting to WiFi and getting time over NTP.");
	        obtain_time();
	        // update 'now' variable with current time
	        time(&now);
	    }
}
