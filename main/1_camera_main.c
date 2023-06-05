#include "header.h"

esp_err_t init_camera(void)
{
    camera_config_t camera_config = {
        .pin_pwdn  = CAM_PIN_PWDN,
        .pin_reset = CAM_PIN_RESET,
        .pin_xclk = CAM_PIN_XCLK,
        .pin_sccb_sda = CAM_PIN_SIOD,
        .pin_sccb_scl = CAM_PIN_SIOC,

        .pin_d7 = CAM_PIN_D7,
        .pin_d6 = CAM_PIN_D6,
        .pin_d5 = CAM_PIN_D5,
        .pin_d4 = CAM_PIN_D4,
        .pin_d3 = CAM_PIN_D3,
        .pin_d2 = CAM_PIN_D2,
        .pin_d1 = CAM_PIN_D1,
        .pin_d0 = CAM_PIN_D0,
        .pin_vsync = CAM_PIN_VSYNC,
        .pin_href = CAM_PIN_HREF,
        .pin_pclk = CAM_PIN_PCLK,

        .xclk_freq_hz = CONFIG_XCLK_FREQ,
        .ledc_timer = LEDC_TIMER_0,
        .ledc_channel = LEDC_CHANNEL_0,

        .pixel_format = PIXFORMAT_JPEG,
        .frame_size = FRAMESIZE_VGA,

        .jpeg_quality = 10,
        .fb_count = 1,
        .grab_mode = CAMERA_GRAB_WHEN_EMPTY};//CAMERA_GRAB_LATEST. Sets when buffers should be filled
    esp_err_t err = esp_camera_init(&camera_config);
    if (err != ESP_OK)
    {
        return err;
    }
    return ESP_OK;
}


void http_post_image(camera_fb_t *fb, char *path)
{   

    const struct addrinfo hints = {
        .ai_family = AF_INET,
        .ai_socktype = SOCK_STREAM,
    };

    //while(1){
    uint8_t count_loop = 0;

    while(count_loop <= 3) {
        count_loop++;
        int err = getaddrinfo(server_infor.web_server, server_infor.web_port, &hints, &res);
        if(err != 0 || res == NULL) {
            ESP_LOGE(TAG_CAM, "DNS lookup failed err=%d res=%p", err, res);
            vTaskDelay(1000 / portTICK_PERIOD_MS);
            continue;
        }

        /* Code to print the resolved IP.
        Note: inet_ntoa is non-reentrant, look at ipaddr_ntoa_r for "real" code */
        addr = &((struct sockaddr_in *)res->ai_addr)->sin_addr;
        ESP_LOGI(TAG_CAM, "DNS lookup succeeded. IP=%s", inet_ntoa(*addr));

        status = socket(res->ai_family, res->ai_socktype, 0);
        if(status < 0) {
            ESP_LOGE(TAG_CAM, "... Failed to allocate socket.");
            freeaddrinfo(res);
            vTaskDelay(1000 / portTICK_PERIOD_MS);
            continue;
        }
        ESP_LOGI(TAG_CAM, "... allocated socket");

        if(connect(status, res->ai_addr, res->ai_addrlen) != 0) {
            ESP_LOGE(TAG_CAM, "... socket connect failed errno=%d", errno);
            close(status);
            freeaddrinfo(res);
            vTaskDelay(4000 / portTICK_PERIOD_MS);
            continue;
        }

        ESP_LOGI(TAG_CAM, "... connected");
        freeaddrinfo(res);

        char HEADER[512];
        char header[512];

        sprintf(header, "POST %s HTTP/1.1\r\n", path);
        strcpy(HEADER, header);
        sprintf(header, "Host: %s:%s\r\n",  server_infor.web_server, server_infor.web_port);
        strcat(HEADER, header);
        sprintf(header, "User-Agent: esp-idf/%d.%d.%d esp32\r\n", ESP_IDF_VERSION_MAJOR, ESP_IDF_VERSION_MINOR, ESP_IDF_VERSION_PATCH);
        strcat(HEADER, header);
        sprintf(header, "Accept: */*\r\n");
        strcat(HEADER, header);
        sprintf(header, "Content-Type: multipart/form-data; boundary=%s\r\n", BOUNDARY);
        strcat(HEADER, header);
    
        char BODY[512];
        sprintf(header, "--%s\r\n", BOUNDARY);
        strcpy(BODY, header);
        sprintf(header, "Content-Disposition: form-data; name=\"image\"; filename=\"%s.jpg\"\r\n", Current_Date_Time);
        strcat(BODY, header);
        sprintf(header, "Content-Type: image/jpeg\r\n\r\n");
        strcat(BODY, header);

        char END[128];
        sprintf(header, "\r\n--%s--\r\n\r\n", BOUNDARY);
        strcpy(END, header);
        
        int dataLength = strlen(BODY) + strlen(END) +  fb->len;
        sprintf(header, "Content-Length: %d\r\n\r\n", dataLength);
        strcat(HEADER, header);

        ESP_LOGD(TAG_CAM, "[%s]", HEADER);
        if (write(status, HEADER, strlen(HEADER)) < 0) {
            ESP_LOGE(TAG_CAM, "... socket1 send failed");
            close(status);
            vTaskDelay(4000 / portTICK_PERIOD_MS);
            continue;
        }

        ESP_LOGD(TAG_CAM, "[%s]", BODY);
        if (write(status, BODY, strlen(BODY)) < 0) {
            ESP_LOGE(TAG_CAM, "... socket2 send failed");
            close(status);
            vTaskDelay(4000 / portTICK_PERIOD_MS);
            continue;
        }

        size_t fbLen = fb->len;
        if (write(status,(const char *)fb->buf, fbLen) < 0) {
            ESP_LOGE(TAG_CAM, "... socket3 send failed");
            close(status);
            vTaskDelay(4000 / portTICK_PERIOD_MS);
            continue;
        }

        if (write(status, END, strlen(END)) < 0) {
            ESP_LOGE(TAG_CAM, "... socket6 send failed");
            close(status);
            vTaskDelay(4000 / portTICK_PERIOD_MS);
            continue;
        }

        ESP_LOGI(TAG_CAM, "Starting again!");
        vTaskDelay(2000 / portTICK_PERIOD_MS);
        close(status);
        break;
    }
}


void jpg_capture(){
    while(1){
        // condition for checkin/out state?
        static camera_fb_t * fb = NULL; 

        if((checkin_state == PREP_CHECKIN || checkout_state == PREP_CHECKOUT) && allow_camera == ON){
            fb = esp_camera_fb_get();
            if (!fb) {
                ESP_LOGE(TAG_CAM, "Camera capture failed");
            }
            else{
                capture_done = true;
                ESP_LOGI(TAG_CAM, "Camera capture success!");
            }
        }else if((checkin_state == DONE_CHECKIN || checkout_state == DONE_CHECKOUT) && capture_done == true){
                ESP_LOGI(TAG_CAM, "Posting Image!");
                #ifdef POSITION == GATE 
                    #ifdef TYPE == CHECKIN
                        if(checkin_state == DONE_CHECKIN){
                            http_post_image(fb, server_infor.post_image_checkin_path);
                        }
                    #elif TYPE == CHECKOUT
                        if(checkout_state == DONE_CHECKOUT){
                            http_post_image(fb, server_infor.post_image_checkout_path);
                        }
                    #endif
                #endif

                // if(checkin_state == DONE_CHECKIN){
                //     http_post_image(fb, server_infor.post_image_checkin_path);
                // }
                // if(checkout_state == DONE_CHECKOUT){
                //     http_post_image(fb, server_infor.post_image_checkout_path);
                // }
                // http_post_image(fb, server_infor.post_image_path);

                postimage_done = true;
                allow_camera = OFF;
                capture_done = false;
                esp_camera_fb_return(fb);
        }
        else{
            esp_camera_fb_return(fb);
        }


        ESP_LOGE(TAG_CAM, "Vehicle Not Detected!");
        vTaskDelay(2000 / portTICK_PERIOD_MS);
    }
}


