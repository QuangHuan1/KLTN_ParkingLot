idf_component_register(SRCS "app_main.c"
                    INCLUDE_DIRS "."
                    REQUIRES camera sensor wifi
                    PRIV_REQUIRES nvs_flash esp32-camera-master esp_http_server)