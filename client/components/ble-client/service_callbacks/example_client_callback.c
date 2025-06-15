
#include "esp_log.h"
#include "esp_err.h"

#include "ble_client.h"

#define TAG "Example Client Service"

void ble_gattc_example_service_callback(esp_gattc_cb_event_t event, esp_gatt_if_t gattc_if, esp_ble_gattc_cb_param_t *param)
{
    switch (event)
    {
    case ESP_GATTC_CONNECT_EVT:
    {
        break;
    }
    case ESP_GATTC_DISCONNECT_EVT:
    {
        break;
    }
    case ESP_GATTC_SEARCH_CMPL_EVT:
    {
        /**
         *  Post search complete operations can be implemented here, for example, if any of the characteristics requires 
         *  register for notification, service callback can register it using `esp_ble_gattc_register_for_notify()` function
         */
        break;
    }
    case ESP_GATTC_WRITE_CHAR_EVT:
    {
        ESP_LOGI(TAG, "ESP_GATTC_WRITE_CHAR_EVT");
        break;
    }
    case ESP_GATTC_READ_CHAR_EVT:
    {
        ESP_LOGI(TAG, "ESP_GATTC_READ_CHAR_EVT");
        break;
    }
    case ESP_GATTC_READ_DESCR_EVT:
    {
        ESP_LOGI(TAG, "ESP_GATTC_READ_DESCR_EVT");
        break;
    }
    default:
    {
        ESP_LOGE(TAG, "Unhandled example service callback event(event: %d)", event);
        break;
    }
    }
}