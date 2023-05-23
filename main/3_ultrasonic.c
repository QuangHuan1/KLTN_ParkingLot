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

esp_err_t ultrasonic_measure_cm(const ultrasonic_sensor_t *dev, uint32_t max_distance, uint32_t *distance)
{
    if (!distance)
        return ESP_ERR_INVALID_ARG;

    portENTER_CRITICAL(&mux);

    // Ping: Low for 2..4 us, then high 10 us
    gpio_set_level(dev->trigger_pin, 0);
    ets_delay_us(TRIGGER_LOW_DELAY);
    gpio_set_level(dev->trigger_pin, 1);
    ets_delay_us(TRIGGER_HIGH_DELAY);
    gpio_set_level(dev->trigger_pin, 0);

    // Previous ping isn't ended
    if (gpio_get_level(dev->echo_pin))
        RETURN_CRTCAL(mux, ESP_ERR_ULTRASONIC_PING);

    // Wait for echo
    uint32_t start = get_time_us();
    while (!gpio_get_level(dev->echo_pin))
    {
        if (timeout_expired(start, PING_TIMEOUT))
            RETURN_CRTCAL(mux, ESP_ERR_ULTRASONIC_PING_TIMEOUT);
    }

    // got echo, measuring
    uint32_t echo_start = get_time_us();
    uint32_t time = echo_start;
    uint32_t meas_timeout = echo_start + max_distance * ROUNDTRIP;
    while (gpio_get_level(dev->echo_pin))
    {
        time = get_time_us();
        if (timeout_expired(echo_start, meas_timeout))
            RETURN_CRTCAL(mux, ESP_ERR_ULTRASONIC_ECHO_TIMEOUT);
    }
    portEXIT_CRITICAL(&mux);

    *distance = (time - echo_start) / ROUNDTRIP;

    return ESP_OK;
}

void ultrasonic(void *pvParamters)
{
	ultrasonic_sensor_t sensor_front = {
		.trigger_pin = sensor0.sensor_trigger_pin,
		.echo_pin = sensor0.sensor_echo_pin,
	};

    ultrasonic_sensor_t sensor_rear = {
		.trigger_pin = sensor1.sensor_trigger_pin,
		.echo_pin = sensor1.sensor_echo_pin,
	};


	ultrasonic_init(&sensor_front);
	ultrasonic_init(&sensor_rear);

    uint32_t distance = 0;
    uint8_t isCarCnt = 0;
    car_entry = 0;
    checkin_state = NO_CHECKIN;
    car_status = IDLE;
    
	while (true) {
        uint16_t avg_distance = 0;
        int index_loop = 1;
        while(index_loop <= 10 && isCarCnt <= 3){
                esp_err_t res = ultrasonic_measure_cm(&sensor_front, MAX_DISTANCE_CM, &distance);
                if (res != ESP_OK) {
                    printf("Error Ultranonic 1\n");
                    car_status = IDLE;
                    break;
                } else {
                    if (distance < 100){
                        if (car_status == IDLE || car_status == CHECKIN){
                            car_status = CHECKIN;
                            checkin_state = SHALL_CHECKIN;
                        }
                        else { //if (car_status == CHECKOUT)
                            //Checkout_status = SHALL_CHECKOUT;
                        }   
                        isCarCnt++;
                    } else{
                        // car_status = IDLE;
                        break;
                    }
                    ESP_LOGI(TAG2, "Average Measurement Distance in %d times: %d cm\n", 10, distance);
                    vTaskDelay(500 / portTICK_PERIOD_MS);      
                    index_loop++;
                }
        }

        index_loop = 0;
        isCarCnt = 0;

                while(index_loop <= 10 && isCarCnt <= 3){
                esp_err_t res = ultrasonic_measure_cm(&sensor_front, MAX_DISTANCE_CM, &distance);
                if (res != ESP_OK) {
                    printf("Error Ultranonic 1\n");
                    car_status = IDLE;
                    break;
                } else {
                    if (distance < 100){
                        if (car_status == IDLE || car_status == CKECKOUT){
                            car_status = CKECKOUT;
                            // Checkout_status = SHALL_CHECKOUT;
                        }
                        else { //if (car_status == CHECKIN)
                            //Checkout_status = SHALL_CHECKOUT;
                             checkin_state = SHALL_CHECKIN;
                        }   
                        isCarCnt++;
                    } else{
                        // car_status = IDLE;
                        break;
                    }
                    ESP_LOGI(TAG2, "Average Measurement Distance in %d times: %d cm\n", 10, distance);
                    vTaskDelay(500 / portTICK_PERIOD_MS);      
                    index_loop++;
                }
            isCarCnt++;
        }

        switch (car_status)
        {
        case IDLE:
            /* code */
            break;
        case CHECKIN:
            /* code */


            break;
        case CHEKCOUT:
            /* code */
            break;
        default:
            break;
        }

        if(car_status == SHALL_CHECKIN){
            allow_camera = ON;
            allow_reader = ON;

            Get_current_date_time(Current_Date_Time, Current_Date_Time_Raw);

            gpio_set_level(gpio0.reader_trigger_pin, 1);
            vTaskDelay(1000/portTICK_PERIOD_MS);
            gpio_set_level(gpio0.reader_trigger_pin, 0);
        } else{

        }

        printf("sensor_next = %d \n", checkin_state);

       
        vTaskDelay(1000 / portTICK_PERIOD_MS);

        // avg_distance = avg_distance / 10;
        
        // ESP_LOGE(TAG2, "distance = %d and carCounter = %d\n", avg_distance, isCarCnt);
        // // distance with in 80cm.
        // if( avg_distance < 80){
        //     isCarCnt++;
        //     if (isCarCnt == 3){
        //         isCarCnt = 0;
        //         allow_camera = ON;
        //         allow_reader = ON;

        //         Get_current_date_time(Current_Date_Time, Current_Date_Time_Raw);

        //         ESP_LOGI(TAG2, "Average Measurement Distance in %d times: %d cm\n", 10, distance);
        //         gpio_set_level(gpio0.reader_trigger_pin, 0);
        //         vTaskDelay(1000/portTICK_PERIOD_MS);
        //         gpio_set_level(gpio0.reader_trigger_pin, 1);
        //     }
        // }
        // else{
        //     isCarCnt = 0;
        // }
        // vTaskDelay(1000 / portTICK_PERIOD_MS);
	}

    free(&sensor);
}