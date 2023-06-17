/**
 * @file ultrasonic.c
 *
 * ESP-IDF driver for ultrasonic range meters, e.g. HC-SR04, HY-SRF05 and so on
 *
 * Ported from esp-open-rtos
 * Copyright (C) 2016, 2018 Ruslan V. Uss <unclerus@gmail.com>
 * BSD Licensed as described in the file LICENSE
 */
// #include "ultrasonic.h"
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <sys/time.h>

#include "header.h" 

#define TRIGGER_LOW_DELAY 4
#define TRIGGER_HIGH_DELAY 10
#define PING_TIMEOUT 6000
#define ROUNDTRIP 58.8

static portMUX_TYPE mux = portMUX_INITIALIZER_UNLOCKED;

static inline uint32_t get_time_us()
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return tv.tv_usec;
}

#define timeout_expired(start, len) ((uint32_t)(get_time_us() - (start)) >= (len))

#define RETURN_CRTCAL(MUX, RES) do { portEXIT_CRITICAL(&MUX); return RES; } while(0)

void ultrasonic_init(const ultrasonic_sensor_t *dev)
{
    gpio_pad_select_gpio( dev->trigger_pin );
    gpio_pad_select_gpio( dev->echo_pin );
    gpio_set_direction(dev->trigger_pin, GPIO_MODE_OUTPUT);
    gpio_set_direction(dev->echo_pin, GPIO_MODE_INPUT);

    gpio_set_level(dev->trigger_pin, 0);
}

esp_err_t ultrasonic_measure_cm(const ultrasonic_sensor_t **dev, uint32_t max_distance, uint32_t *distance)
{
    if (!distance)
        return ESP_ERR_INVALID_ARG;

    portENTER_CRITICAL(&mux);

    // Ping: Low for 2..4 us, then high 10 us
    gpio_set_level((*dev)->trigger_pin, 0);
    ets_delay_us(TRIGGER_LOW_DELAY);
    gpio_set_level((*dev)->trigger_pin, 1);
    ets_delay_us(TRIGGER_HIGH_DELAY);
    gpio_set_level((*dev)->trigger_pin, 0);

    // Previous ping isn't ended
    if (gpio_get_level((*dev)->echo_pin))
        RETURN_CRTCAL(mux, ESP_ERR_ULTRASONIC_PING);

    // Wait for echo
    uint32_t start = get_time_us();
    while (!gpio_get_level((*dev)->echo_pin))
    {
        if (timeout_expired(start, PING_TIMEOUT))
            RETURN_CRTCAL(mux, ESP_ERR_ULTRASONIC_PING_TIMEOUT);
    }

    // got echo, measuring
    uint32_t echo_start = get_time_us();
    uint32_t time = echo_start;
    uint32_t meas_timeout = echo_start + max_distance * ROUNDTRIP;
    while (gpio_get_level((*dev)->echo_pin))
    {
        time = get_time_us();
        if (timeout_expired(echo_start, meas_timeout))
            RETURN_CRTCAL(mux, ESP_ERR_ULTRASONIC_ECHO_TIMEOUT);
    }
    portEXIT_CRITICAL(&mux);

    *distance = (time - echo_start) / ROUNDTRIP;

    return ESP_OK;
}

esp_err_t get_distance_value(const ultrasonic_sensor_t *dev, uint32_t *avg_distance){

    uint32_t distance = 0;
    uint8_t index_loop = 1;
    esp_err_t res = ESP_OK;
    while(index_loop <= 10){
            esp_err_t res = ultrasonic_measure_cm(&dev, MAX_DISTANCE_CM, &distance);
            if (res != ESP_OK) {
                // do nothing
            } else {

                *avg_distance +=  distance;
                index_loop++;
            }
            vTaskDelay(10/portTICK_PERIOD_MS);
        }

        *avg_distance = *avg_distance / 10;
    return res;
}

void ultrasonic(void *pvParamters)
{
	ultrasonic_sensor_t sensor_front = {
		.trigger_pin = sensor0.sensor_trigger_pin,
		.echo_pin = sensor0.sensor_echo_pin,
	};

	ultrasonic_init(&sensor_front);

    NO_CHECKIN = TRUE;
    NO_CHECKOUT = TRUE;
    uint32_t distance_later;
    uint32_t distance_front;

    while (true) {

        NO_CHECKIN = TRUE;
        SHALL_CHECKIN = FALSE;
        PREP_CHECKIN = FALSE;
        DONE_CHECKIN = FALSE;

        NO_CHECKOUT = TRUE;
        SHALL_CHECKOUT = FALSE;
        PREP_CHECKOUT = FALSE;
        DONE_CHECKOUT = FALSE;


        capture_done = FALSE;
        readtag_done = FALSE;
		uint32_t distance;
        esp_err_t res = get_distance_value(&sensor_front, &distance);
        sensor_err = res;
        ESP_LOGW(TAG_SENSOR, "Distance FIRST: %"PRIu32" cm, %.02f m ", distance, distance / 100.0);

        distance_later = distance;
		if (res != ESP_OK) {
			switch (res) {
				case ESP_ERR_ULTRASONIC_PING:
                    ESP_LOGE(TAG_SENSOR, "Cannot ping (device is in invalid state) ");
					break;
				case ESP_ERR_ULTRASONIC_PING_TIMEOUT:
                    ESP_LOGE(TAG_SENSOR, "Ping timeout (no device found) ");
					break;
				case ESP_ERR_ULTRASONIC_ECHO_TIMEOUT:
                    ESP_LOGE(TAG_SENSOR, "Echo timeout (i.e. distance too big) ");
					break;
				default:
                    ESP_LOGE(TAG_SENSOR, "%d ", res);
                    break;
			}
		} else {
            if(distance > FAR_THRESHOLD - THRESHOLD_OFFSET && distance < FAR_THRESHOLD + THRESHOLD_OFFSET){
                ESP_LOGW(TAG_SENSOR, "IN CHECKIN STATE ");
                uint8_t ERROR_COUNT = 0;

                if(NO_CHECKIN == TRUE){
                    SHALL_CHECKIN = TRUE;
                    NO_CHECKIN = FALSE;
                }
                else{
                    //do nothing
                }
                
                while (true)
                {   
                    esp_err_t res_temp = get_distance_value(&sensor_front, &distance);


                    if(distance >= FAR_THRESHOLD + THRESHOLD_OFFSET || res_temp != ESP_OK){
                        ERROR_COUNT++;
                        if(ERROR_COUNT == ERROR_THRESHOLD){
                            NO_CHECKIN = TRUE;
                            SHALL_CHECKIN = FALSE;
                            PREP_CHECKIN = FALSE;
                            DONE_CHECKIN = FALSE;
                            break;
                        }
                    }
                    else {
                        ERROR_COUNT = 0;

                        if(distance <= distance_later){
                            distance_later = distance;
                            if(distance >= CAPTURE_THRESHOLD - THRESHOLD_OFFSET && distance <= CAPTURE_THRESHOLD + THRESHOLD_OFFSET){
                                NO_CHECKIN = FALSE;
                                SHALL_CHECKIN = FALSE;
                                PREP_CHECKIN = TRUE;   
                                DONE_CHECKIN = FALSE;                     
                            }
                            else if (distance <= NEAR_THRESHOLD + THRESHOLD_OFFSET){
                                if(PREP_CHECKIN == TRUE){
                                    NO_CHECKIN = FALSE;
                                    SHALL_CHECKIN = FALSE;
                                    PREP_CHECKIN = FALSE;   
                                    DONE_CHECKIN = TRUE;
                                }else{
                                    //do nothing
                                }
                            }
                            else{
                                // do nothing
                            }
                        }
                        else if (distance > CAPTURE_THRESHOLD + THRESHOLD_OFFSET){
                            
                            distance_later = distance;

                            NO_CHECKIN = FALSE;
                            SHALL_CHECKIN = TRUE;
                            PREP_CHECKIN = FALSE;   
                            DONE_CHECKIN = FALSE;

                            capture_done = FALSE;
                            readtag_done = FALSE;
                            // break;
                        }
                    }

                    ESP_LOGI(TAG_SENSOR, "Distance CHECK IN: %"PRIu32" cm, %.02f m", distance, distance / 100.0);
                    ESP_LOGI(TAG_SENSOR, "NO_CHECKIN state %d ", NO_CHECKIN);
                    ESP_LOGI(TAG_SENSOR, "SHALL_CHECKIN state %d ", SHALL_CHECKIN);
                    ESP_LOGI(TAG_SENSOR, "PREP_CHECKIN state %d ", PREP_CHECKIN);
                    ESP_LOGI(TAG_SENSOR, "DONE_CHECKIN state %d ", DONE_CHECKIN);
                    ESP_LOGI(TAG_SENSOR, "--------------------------------------\n");


                    if(PREP_CHECKIN == TRUE){
                        Get_current_date_time(Current_Date_Time, Current_Date_Time_Raw);
                        allow_camera = (capture_done == FALSE) ? TRUE : FALSE;
                        allow_reader = (readtag_done == FALSE) ? TRUE : FALSE;
                        
                        // if(capture_done == FALSE){
                        //     allow_camera = TRUE;
                        // } else {
                        //     allow_camera = FALSE;
                        // }
                        // if(readtag_done == FALSE){
                        //     allow_reader = TRUE;
                        // } else {
                        //     allow_reader = FALSE;
                        // }
                    } else if (DONE_CHECKIN == TRUE){
                        uint8_t count = 0;
                        while(!(postetag_done || count == 10)){
                            count++;
                            vTaskDelay(500/portTICK_PERIOD_MS);
                        }
                        postetag_done = FALSE;
                        break;
                    }
                    else
                    {
                        capture_done = FALSE;
                        readtag_done = FALSE;
                    }
                    
                    vTaskDelay(DELAY_TIME / portTICK_PERIOD_MS);
                }

            }else if (distance <= NEAR_THRESHOLD + THRESHOLD_OFFSET){
                ESP_LOGW(TAG_SENSOR, "IN CHECKOUT STATE");
                if(NO_CHECKOUT == TRUE){
                    SHALL_CHECKOUT = TRUE;
                    NO_CHECKOUT = FALSE;
                }
                else{
                    //do nothing
                }
                uint8_t ERROR_COUNT = 0;

                while (true)
                {
                    esp_err_t res_temp = get_distance_value(&sensor_front, &distance);

                    if(distance > FAR_THRESHOLD + THRESHOLD_OFFSET || res_temp != ESP_OK){
                        ERROR_COUNT++;
                        if(ERROR_COUNT == ERROR_THRESHOLD){
                            NO_CHECKOUT = TRUE;
                            SHALL_CHECKOUT = FALSE;
                            PREP_CHECKOUT = FALSE;
                            DONE_CHECKOUT = FALSE;
                            break;
                        }
                    }
                    else {
                        ERROR_COUNT = 0;
                        if(distance >= distance_later){
                            distance_later = distance;

                            if(distance >= CAPTURE_THRESHOLD - THRESHOLD_OFFSET && distance <= CAPTURE_THRESHOLD + THRESHOLD_OFFSET){
                                NO_CHECKOUT = FALSE;
                                SHALL_CHECKOUT = FALSE;
                                PREP_CHECKOUT = TRUE;
                                DONE_CHECKOUT = FALSE;

                            }
                            else if (distance >= FAR_THRESHOLD - THRESHOLD_OFFSET && distance <= FAR_THRESHOLD + THRESHOLD_OFFSET){
                                if(PREP_CHECKOUT == TRUE){
                                    NO_CHECKOUT = FALSE;
                                    SHALL_CHECKOUT = FALSE;
                                    PREP_CHECKOUT = FALSE;
                                    DONE_CHECKOUT = TRUE;

                                }else{
                                    //do nothing
                                }

                            }
                            else{
                                // do nothing
                            }
                        }
                        else if(distance <= CAPTURE_THRESHOLD - THRESHOLD_OFFSET){
                            distance_later = distance;

                            NO_CHECKOUT = FALSE;
                            SHALL_CHECKOUT = TRUE;
                            PREP_CHECKOUT = FALSE;
                            DONE_CHECKOUT = FALSE;

                            capture_done = FALSE;
                            readtag_done = FALSE;
                        }
                    }

                    
                    ESP_LOGI(TAG_SENSOR, "Distance CHECK OUT: %"PRIu32" cm, %.02f m ", distance, distance / 100.0);
                    ESP_LOGI(TAG_SENSOR, "NO_CHECKOUT state %d ", NO_CHECKOUT);
                    ESP_LOGI(TAG_SENSOR, "SHALL_CHECKOUT state %d ", SHALL_CHECKOUT);
                    ESP_LOGI(TAG_SENSOR, "PREP_CHECKOUT state %d ", PREP_CHECKOUT);
                    ESP_LOGI(TAG_SENSOR, "DONE_CHECKOUT state %d ", DONE_CHECKOUT);
                    ESP_LOGI(TAG_SENSOR, "-------------------------------------- ");

                    //condition for check-out

                    if(PREP_CHECKOUT == TRUE){
                        Get_current_date_time(Current_Date_Time, Current_Date_Time_Raw);
                        allow_camera = (capture_done == FALSE) ? TRUE : FALSE;
                        allow_reader = (readtag_done == FALSE) ? TRUE : FALSE;

                        // if(capture_done == false){
                        //     allow_camera = TRUE;
                        // } else {
                        //     allow_camera = FALSE;
                        // }

                        // if(readtag_done == false){
                        //     allow_reader = TRUE;
                        // } else {
                        //     allow_reader = FALSE;
                        // } 

                        // vTaskDelay(1000/portTICK_PERIOD_MS);

                    } 
                    else if (DONE_CHECKOUT == TRUE)
                    {
                        uint8_t count = 0;
                        while(!(postetag_done || count == 10)){
                            count++;
                            vTaskDelay(500/portTICK_PERIOD_MS);
                        }
                        postetag_done = FALSE;
                        break;
                    }
                    else
                    {
                        capture_done = FALSE;
                        readtag_done = FALSE;
                    }
                    vTaskDelay(DELAY_TIME / portTICK_PERIOD_MS);
                }                
            }   
            else{
                ESP_LOGW(TAG_SENSOR, "Distance : %"PRIu32" cm, %.02f m ", distance, distance / 100.0);

            }
		}
        vTaskDelay(DELAY_TIME / portTICK_PERIOD_MS);
	}
}
