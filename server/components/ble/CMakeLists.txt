
set(
    SERVICE_SRCS 
    "ble.c" 
    "ble_gatts.c" 
    "ble_gap.c"
    "service_callbacks/example_service_handler.c"
)

set(
    SERVICE_INCLUDES
    "."
    "service_callbacks"
)

idf_component_register(SRCS ${SERVICE_SRCS}
                        INCLUDE_DIRS ${SERVICE_INCLUDES}
                        REQUIRES bt mbedtls esp_event
)

