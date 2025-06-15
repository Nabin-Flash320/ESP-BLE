
#include "stdlib.h"
#include "string.h"

#include "esp_log.h"
#include "esp_err.h"
#include "esp_bt.h"
#include "esp_gap_ble_api.h"
#include "esp_gattc_api.h"
#include "esp_gatt_defs.h"
#include "esp_bt_main.h"
#include "esp_gatt_common_api.h"

#include "ble_client.h"

#define BLE_GAP_TAG "BLE GAP"
#define EXPAND_ARRAY_OF_6(arr) arr[0], arr[1], arr[2], arr[3], arr[4], arr[5]

void ble_gap_process_scan_result_event_handler(esp_ble_gap_cb_param_t *param);

static bool ble_gap_scan_stopped = false;
static char *server_name = NULL;

void ble_gap_callbacak(esp_gap_ble_cb_event_t event, esp_ble_gap_cb_param_t *param)
{
    switch (event)
    {
    case ESP_GAP_BLE_SCAN_PARAM_SET_COMPLETE_EVT:
    {
        ESP_LOGI(BLE_GAP_TAG, "ESP_GAP_BLE_SCAN_PARAM_SET_COMPLETE_EVT");
        if (ESP_BT_STATUS_SUCCESS == param->scan_param_cmpl.status)
        {
            ESP_ERROR_CHECK_WITHOUT_ABORT(esp_ble_gap_start_scanning(15));
            ble_gap_scan_stopped = false;
        }
        else
        {
            ESP_LOGE(BLE_GAP_TAG, "Scan param set complete error: 0x%02x", param->scan_param_cmpl.status);
        }
        break;
    }
    case ESP_GAP_BLE_SCAN_RESULT_EVT:
    {
        ESP_LOGI(BLE_GAP_TAG, "ESP_GAP_BLE_SCAN_RESULT_EVT");
        ble_gap_process_scan_result_event_handler(param);
        break;
    }
    case ESP_GAP_BLE_SCAN_STOP_COMPLETE_EVT:
    {
        ESP_LOGI(BLE_GAP_TAG, "ESP_GAP_BLE_SCAN_STOP_COMPLETE_EVT");
        break;
    }
    default:
    {
        ESP_LOGI(BLE_GAP_TAG, "GAP event %d unhandled", event);
        break;
    }
    }
}

void ble_set_server_name(const char *name)
{
    if (!name)
    {
        return;
    }

    if (NULL != server_name)
    {
        free(server_name);
        server_name = NULL;
    }

    server_name = (char *)malloc(32);
    if (server_name)
    {
        memset(server_name, 0, 32);
        snprintf(server_name, 32, "%s", name);
    }

    printf("server name: %s\n", server_name);
}

void ble_gap_process_scan_result_event_handler(esp_ble_gap_cb_param_t *param)
{
    switch (param->scan_rst.search_evt)
    {
    case ESP_GAP_SEARCH_INQ_RES_EVT:
    {
        ESP_LOGI(BLE_GAP_TAG, "ESP_GAP_SEARCH_INQ_RES_EVT");
        ESP_LOGI(BLE_GAP_TAG, "bda: %d.%d.%d.%d.%d.%d, dev type: %d", EXPAND_ARRAY_OF_6(param->scan_rst.bda),
                 param->scan_rst.dev_type);
        uint8_t *manufacturer_data = NULL;
        uint8_t manufacturer_data_len = 0;
        manufacturer_data = esp_ble_resolve_adv_data(param->scan_rst.ble_adv,
                                                     ESP_BLE_AD_TYPE_NAME_CMPL,
                                                     &manufacturer_data_len);

        if (server_name)
        {
            if ((strlen(server_name) == manufacturer_data_len) && (0 == strncmp(server_name, (const char *)manufacturer_data, strlen(server_name))))
            {
                ESP_LOGE(BLE_GAP_TAG, "Device %s discovered", server_name);
                if (!ble_gap_scan_stopped)
                {
                    ble_gap_scan_stopped = true;
                    ESP_ERROR_CHECK(esp_ble_gap_stop_scanning());
                    s_gattc_service_instance_t *service_instance = ble_gattc_get_service_instance_by_id(BLE_PROFILE_ID_EXAMPLE);
                    if (service_instance)
                    {
                        ESP_ERROR_CHECK(esp_ble_gattc_open(service_instance->gattc_if, param->scan_rst.bda, param->scan_rst.ble_addr_type, true));
                    }
                }
            }
        }
        break;
    }
    case ESP_GAP_SEARCH_INQ_CMPL_EVT:
    {
        ESP_LOGI(BLE_GAP_TAG, "ESP_GAP_SEARCH_INQ_CMPL_EVT");
        break;
    }
    case ESP_GAP_SEARCH_DISC_RES_EVT:
    {
        ESP_LOGI(BLE_GAP_TAG, "ESP_GAP_SEARCH_DISC_RES_EVT");
        break;
    }
    case ESP_GAP_SEARCH_DISC_BLE_RES_EVT:
    {
        ESP_LOGI(BLE_GAP_TAG, "ESP_GAP_SEARCH_DISC_BLE_RES_EVT");
        break;
    }
    case ESP_GAP_SEARCH_DISC_CMPL_EVT:
    {
        ESP_LOGI(BLE_GAP_TAG, "ESP_GAP_SEARCH_DISC_CMPL_EVT");
        break;
    }
    case ESP_GAP_SEARCH_DI_DISC_CMPL_EVT:
    {
        ESP_LOGI(BLE_GAP_TAG, "ESP_GAP_SEARCH_DI_DISC_CMPL_EVT");
        break;
    }
    case ESP_GAP_SEARCH_SEARCH_CANCEL_CMPL_EVT:
    {
        ESP_LOGI(BLE_GAP_TAG, "ESP_GAP_SEARCH_SEARCH_CANCEL_CMPL_EVT");
        break;
    }
    case ESP_GAP_SEARCH_INQ_DISCARD_NUM_EVT:
    {
        ESP_LOGI(BLE_GAP_TAG, "ESP_GAP_SEARCH_INQ_DISCARD_NUM_EVT");
        break;
    }
    default:
    {
        ESP_LOGE(BLE_GAP_TAG, "Unknown search event");
        break;
    }
    }
}
