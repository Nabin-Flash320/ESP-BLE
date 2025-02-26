
#include <string.h>
#include <stdlib.h>

#include "esp_log.h"
#include "esp_err.h"

#include "ble.h"

#define TAG "Example Service"

static uint8_t data[1024];
static size_t data_size = 0;

void ble_example_service_callback(esp_gatts_cb_event_t event, esp_gatt_if_t gatts_if, esp_ble_gatts_cb_param_t *param)
{
    switch (event)
    {
    case ESP_GATTS_CONNECT_EVT:
    {
        ESP_LOGI(TAG, "Example service connect event");
        break;
    }
    case ESP_GATTS_DISCONNECT_EVT:
    {
        ESP_LOGI(TAG, "Example service disconnect event");
        break;
    }
    case ESP_GATTS_READ_EVT:
    {
        ESP_LOGI(TAG, "ESP_GATTS_READ_EVT");
        s_gatts_service_inst_t *service_instance = ble_gap_get_service_instance_by_gatts_if(gatts_if);
        if (service_instance && service_instance->characteristics_len > 0 && service_instance->characteristics)
        {
            if (param->write.handle == service_instance->characteristics[EXAMPLE_CHAR_ID_READ].char_handle)
            {
                esp_gatt_rsp_t response = {
                    .attr_value = {
                        .len = data_size,
                        .handle = param->read.handle,
                        .offset = 0,
                        .auth_req = ESP_GATT_AUTH_REQ_NONE,
                    },
                };

                memcpy(response.attr_value.value, data, data_size);
                ESP_ERROR_CHECK(
                    esp_ble_gatts_send_response(
                        gatts_if,
                        param->read.conn_id,
                        param->read.trans_id,
                        ESP_GATT_OK,
                        &response));
            }
        }
        break;
    }
    case ESP_GATTS_WRITE_EVT:
    {
        ESP_LOGI(TAG, "ESP_GATTS_WRITE_EVT for handle: 0x%02x(conn-id: %d)", param->write.handle, param->write.conn_id);
        s_gatts_service_inst_t *service_instance = ble_gap_get_service_instance_by_gatts_if(gatts_if);
        if (service_instance && service_instance->characteristics_len > 0 && service_instance->characteristics)
        {
            if (param->write.handle == service_instance->characteristics[EXAMPLE_CHAR_ID_WRITE].char_handle)
            {
                memset(data, 0, sizeof(data));
                data_size = param->write.len;
                snprintf((char *)data, sizeof(data), "%s", (const char *)param->write.value);
                ESP_LOGI(TAG, "Incoming data: \n%s", data);
            }
        }
        break;
    }
    default:
    {
        ESP_LOGE(TAG, "Unknown example service callback event(event: %d)", event);
        break;
    }
    }
}