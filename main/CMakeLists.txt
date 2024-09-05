idf_component_register(SRCS "BLE.c" "main.c" "WiFi_app.c" "rgb_led.c" "app_NVS.c" "toggle_sleep_button.c" "aws_iot.c" "AHT20.c" "moisture_sensor.c" "global_event_group.c"
                    INCLUDE_DIRS ".")

target_add_binary_data(${COMPONENT_TARGET} "certs/aws_root_ca_pem" TEXT)
target_add_binary_data(${COMPONENT_TARGET} "certs/certificate_pem_crt" TEXT)
target_add_binary_data(${COMPONENT_TARGET} "certs/private_pem_key" TEXT)