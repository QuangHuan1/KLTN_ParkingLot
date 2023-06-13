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
#define ROUNDTRIP 58

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

    // uint16_t avg_distance = 0;
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

    NO_CHECKIN = ON;
    NO_CHECKOUT = ON;
    uint32_t distance_later;
    uint32_t distance_front;

    while (true) {
        NO_CHECKIN = ON;
        SHALL_CHECKIN = OFF;
        PREP_CHECKIN = OFF;
        DONE_CHECKIN = OFF;

        NO_CHECKOUT = ON;
        SHALL_CHECKOUT = OFF;
        PREP_CHECKOUT = OFF;
        DONE_CHECKOUT = OFF;

        capture_done = false;
        readtag_done = false;
		uint32_t distance;
		// esp_err_t res = ultrasonic_measure_cm(&sensor_front, MAX_DISTANCE_CM, &distance);
        esp_err_t res = get_distance_value(&sensor_front, &distance);
        printf("Distance FIRST: %"PRIu32" cm, %.02f m\n", distance, distance / 100.0);
        distance_later = distance;
		if (res != ESP_OK) {
            gpio_set_level(gpio0.LED_Sensor_Status, 1);
			printf("Error: ");
			switch (res) {
				case ESP_ERR_ULTRASONIC_PING:
					printf("Cannot ping (device is in invalid state)\n");
					break;
				case ESP_ERR_ULTRASONIC_PING_TIMEOUT:
					printf("Ping timeout (no device found)\n");
					break;
				case ESP_ERR_ULTRASONIC_ECHO_TIMEOUT:
					printf("Echo timeout (i.e. distance too big)\n");
					break;
				default:
					printf("%d\n", res);
			}
		} else {
            gpio_set_level(gpio0.LED_Sensor_Status, 0);


            if(distance > FAR_THRESHOLD - THRESHOLD_OFFSET && distance < FAR_THRESHOLD + THRESHOLD_OFFSET){
                printf("IN CHECKIN STATE\n");

                if(NO_CHECKIN == ON){
                    SHALL_CHECKIN = ON;
                    NO_CHECKIN = OFF;
                }
                else{
                    //do nothing
                }
                
                while (true)
                {   
                    // esp_err_t res_temp = ultrasonic_measure_cm(&sensor_front, MAX_DISTANCE_CM, &distance);
                    esp_err_t res_temp = get_distance_value(&sensor_front, &distance);

                    if(distance > FAR_THRESHOLD + THRESHOLD_OFFSET || res_temp != ESP_OK){
                        ERROR_COUNT++;
                        if(ERROR_COUNT == ERROR_THRESHOLD){
                            NO_CHECKIN = ON;
                            SHALL_CHECKIN = OFF;
                            PREP_CHECKIN = OFF;
                            DONE_CHECKIN = OFF;
                            break;
                        }
                    }
                    else {
                        ERROR_COUNT = 0;

                        if(distance <= distance_later){
                            distance_later = distance;
                            if(distance > CAPTURE_THRESHOLD - THRESHOLD_OFFSET && distance < CAPTURE_THRESHOLD + THRESHOLD_OFFSET){
                                NO_CHECKIN = OFF;
                                SHALL_CHECKIN = OFF;
                                PREP_CHECKIN = ON;   
                                DONE_CHECKIN = OFF;                     
                            }
                            else if (distance <= NEAR_THRESHOLD){
                                if(PREP_CHECKIN == ON){
                                    NO_CHECKIN = OFF;
                                    SHALL_CHECKIN = OFF;
                                    PREP_CHECKIN = OFF;   
                                    DONE_CHECKIN = ON;
                                    // checkin_state = DONE_CHECKIN;
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

                            NO_CHECKIN = OFF;
                            SHALL_CHECKIN = ON;
                            PREP_CHECKIN = OFF;   
                            DONE_CHECKIN = OFF;

                            capture_done = false;
                            readtag_done = false;
                            // break;
                        }
                    }

                    
			        printf("Distance CHECK IN: %"PRIu32" cm, %.02f m\n", distance, distance / 100.0);
                    printf("NO_CHECKIN state %d\n", NO_CHECKIN);
                    printf("SHALL_CHECKIN state %d\n", SHALL_CHECKIN);
                    printf("PREP_CHECKIN state %d\n", PREP_CHECKIN);
                    printf("DONE_CHECKIN state %d\n", DONE_CHECKIN);
                    printf("--------------------------------------\n");


                    if(PREP_CHECKIN == ON){
                        Get_current_date_time(Current_Date_Time, Current_Date_Time_Raw);

                        if(capture_done == false){
                            allow_camera = ON;
                        } else {
                            //do nothing
                            allow_camera = OFF;

                        }
                        if(readtag_done == false){
                            allow_reader = ON;
                            // gpio_set_level(gpio0.reader_trigger_pin, 1);
                            // vTaskDelay(200/portTICK_PERIOD_MS);
                            // gpio_set_level(gpio0.reader_trigger_pin, 0);


                        } else {
                            //do nothing
                            allow_reader = OFF;
                        }
                    } else if (DONE_CHECKIN == ON){
                        uint8_t count_loop = 0;
                        while(count_loop <= 10 || postetag_done == true){
                            count_loop++;
                            vTaskDelay(100/portTICK_PERIOD_MS);
                        }
                        postetag_done = false;
                        break;
                    }
                    else
                    {
                        capture_done = false;
                        readtag_done = false;
                    }
                    
                    vTaskDelay(100 / portTICK_PERIOD_MS);
                }

            }else if (distance <= NEAR_THRESHOLD){
                printf("IN CHECKOUT STATE\n");
                if(NO_CHECKOUT == ON){
                    SHALL_CHECKOUT = ON;
                    NO_CHECKOUT = OFF;
                }
                else{
                    //do nothing
                }
                // checkout_state = SHALL_CHECKOUT;
                while (true)
                {
                    // esp_err_t res_temp = ultrasonic_measure_cm(&sensor_front, MAX_DISTANCE_CM, &distance);
                    esp_err_t res_temp = get_distance_value(&sensor_front, &distance);

                    if(distance > FAR_THRESHOLD + THRESHOLD_OFFSET || res_temp != ESP_OK){
                        ERROR_COUNT++;
                        if(ERROR_COUNT == ERROR_THRESHOLD){
                            NO_CHECKOUT = ON;
                            SHALL_CHECKOUT = OFF;
                            PREP_CHECKOUT = OFF;
                            DONE_CHECKOUT = OFF;
                            break;
                        }
                    }
                    else {
                        // distance = 1;
                        // distance_later = distance;
                        ERROR_COUNT = 0;


                        if(distance >= distance_later){
                            distance_later = distance;

                            if(distance > CAPTURE_THRESHOLD - THRESHOLD_OFFSET && distance < CAPTURE_THRESHOLD + THRESHOLD_OFFSET){
                                NO_CHECKOUT = OFF;
                                SHALL_CHECKOUT = OFF;
                                PREP_CHECKOUT = ON;
                                DONE_CHECKOUT = OFF;
                                // PREP_CHECKOUT = ON;
                                // checkout_state = PREP_CHECKOUT;
                            }
                            else if (distance > FAR_THRESHOLD - THRESHOLD_OFFSET && distance < FAR_THRESHOLD + THRESHOLD_OFFSET){
                                if(PREP_CHECKOUT == ON){
                                    NO_CHECKOUT = OFF;
                                    SHALL_CHECKOUT = OFF;
                                    PREP_CHECKOUT = OFF;
                                    DONE_CHECKOUT = ON;
                                    // DONE_CHECKOUT = ON;
                                    // checkout_state = DONE_CHECKOUT;
                                }else{
                                    //do nothing
                                }

                            }
                            else{
                                // do nothing
                            }
                        }
                        else if(distance <= CAPTURE_THRESHOLD - THRESHOLD_OFFSET){
                            // SHALL_CHECKOUT = ON;
                            // checkout_state = SHALL_CHECKOUT;
                            distance_later = distance;

                            NO_CHECKOUT = OFF;
                            SHALL_CHECKOUT = ON;
                            PREP_CHECKOUT = OFF;
                            DONE_CHECKOUT = OFF;


                            capture_done = false;
                            readtag_done = false;
                        }
                    }

                    

			        printf("Distance CHECK OUT: %"PRIu32" cm, %.02f m\n", distance, distance / 100.0);
                    printf("NO_CHECKOUT state %d\n", NO_CHECKOUT);
                    printf("SHALL_CHECKOUT state %d\n", SHALL_CHECKOUT);
                    printf("PREP_CHECKOUT state %d\n", PREP_CHECKOUT);
                    printf("DONE_CHECKOUT state %d\n", DONE_CHECKOUT);
                    printf("--------------------------------------\n");
                    //condition for check-out

                    if(PREP_CHECKOUT == ON){
                        Get_current_date_time(Current_Date_Time, Current_Date_Time_Raw);

                        if(capture_done == false){
                            allow_camera = ON;
                        } else {
                            allow_camera = OFF;
                        }

                        if(readtag_done == false){
                            allow_reader = ON;
                            // gpio_set_level(gpio0.reader_trigger_pin, 1);
                            // vTaskDelay(200/portTICK_PERIOD_MS);
                            // gpio_set_level(gpio0.reader_trigger_pin, 0);
                        } else {
                            allow_reader = OFF;
                        } 
                    } 
                    else if (DONE_CHECKOUT == ON)
                    {
                        uint8_t count_loop = 0;
                        while(count_loop <= 10 || postetag_done == true){
                            count_loop++;
                            vTaskDelay(100/portTICK_PERIOD_MS);
                        }
                        postetag_done = false;
                        break;
                    }
                    else
                    {
                        capture_done = false;
                        readtag_done = false;
                    }
                    vTaskDelay(100 / portTICK_PERIOD_MS);
                }                
            }   
            else{
                    printf("Distance TOTALLLL   : %"PRIu32" cm, %.02f m\n", distance, distance / 100.0);
            }
		}
        vTaskDelay(200 / portTICK_PERIOD_MS);
	}
}
