
set(
    BLE_SRCS 
    "ble_client.c"
    "ble_gap.c"
    "ble_gattc.c"
    "service_callbacks/example_client_callback.c"
)

set(
    BLE_INCLUDES
    .
    "service_callbacks"
)

idf_component_register(
    SRCS  ${BLE_SRCS}
    INCLUDE_DIRS ${BLE_INCLUDES}
    REQUIRES bt mbedtls esp_timer
)

