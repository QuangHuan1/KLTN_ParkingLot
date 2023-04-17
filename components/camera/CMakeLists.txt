idf_component_register(SRCS "camera_main.c"
                    INCLUDE_DIRS "."
                    PRIV_REQUIRES esp32-camera-master esp_http_server wifi main)

