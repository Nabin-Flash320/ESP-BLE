

#include "esp_gap_ble_api.h"
#include "esp_log.h"

#include "ble.h"

#define TAG __FILE__

esp_ble_adv_params_t adv_param = {
    .adv_int_min = 0x0020,
    .adv_int_max = 0x0040,
    .adv_type = ADV_TYPE_IND,
    .own_addr_type = BLE_ADDR_TYPE_PUBLIC,
    .channel_map = ADV_CHNL_ALL,
    .adv_filter_policy = ADV_FILTER_ALLOW_SCAN_ANY_CON_ANY,
};

void ble_gap_start_ble_advertisement()
{
    esp_ble_gap_start_advertising(&adv_param);
}

void ble_gap_callbacak(esp_gap_ble_cb_event_t event, esp_ble_gap_cb_param_t *param)
{
    switch (event)
    {
    case ESP_GAP_BLE_ADV_DATA_SET_COMPLETE_EVT:
    {
        ESP_LOGI(TAG, "ESP_GAP_BLE_ADV_DATA_SET_COMPLETE_EVT\n");
        if (ESP_BT_STATUS_SUCCESS == param->adv_data_cmpl.status)
        {
            ESP_LOGI(TAG, "Starting BLE advertisement");
            ble_gap_start_ble_advertisement();
        }
        break;
    }
    case ESP_GAP_BLE_SCAN_RSP_DATA_SET_COMPLETE_EVT:
    {
        ESP_LOGI(TAG, "ESP_GAP_BLE_SCAN_RSP_DATA_SET_COMPLETE_EVT\n");
        break;
    }
    case ESP_GAP_BLE_ADV_DATA_RAW_SET_COMPLETE_EVT:
    {
        ESP_LOGI(TAG, "ESP_GAP_BLE_ADV_DATA_RAW_SET_COMPLETE_EVT(status: 0x%x)\n", param->adv_data_raw_cmpl.status);
        break;
    }
    case ESP_GAP_BLE_SCAN_RSP_DATA_RAW_SET_COMPLETE_EVT:
    {
        ESP_LOGI(TAG, "ESP_GAP_BLE_SCAN_RSP_DATA_RAW_SET_COMPLETE_EVT\n");
        break;
    }
    case ESP_GAP_BLE_ADV_START_COMPLETE_EVT:
    {
        if (ESP_BT_STATUS_SUCCESS != param->adv_start_cmpl.status)
        {
            ESP_LOGE(TAG, "Error starting advertisement");
        }
        else
        {
            ESP_LOGI(TAG, "Advertisement started successfully.");
        }
        break;
    }
    case ESP_GAP_BLE_SCAN_START_COMPLETE_EVT:
    {
        ESP_LOGI(TAG, "ESP_GAP_BLE_SCAN_START_COMPLETE_EVT\n");
        break;
    }
    case ESP_GAP_BLE_ADV_STOP_COMPLETE_EVT:
    {
        if (ESP_BT_STATUS_SUCCESS != param->adv_stop_cmpl.status)
        {
            ESP_LOGE(TAG, "Error stopping advertisement.");
        }
        else
        {
            ESP_LOGI(TAG, "Stopped advertisement.");
        }
        break;
    }
    case ESP_GAP_BLE_UPDATE_CONN_PARAMS_EVT:
    {
        if (ESP_BT_STATUS_SUCCESS == param->update_conn_params.status)
        {
            ESP_LOGI(TAG, "BLE GAP connection parameters updated.");
            ESP_LOGI(TAG, "BLE device address: %x:%x:%x:%x:%x:%x", param->update_conn_params.bda[0],
                     param->update_conn_params.bda[1],
                     param->update_conn_params.bda[2],
                     param->update_conn_params.bda[3],
                     param->update_conn_params.bda[4],
                     param->update_conn_params.bda[5]);
        }
        else
        {
            ESP_LOGE(TAG, "Failed to update BLE GAP connection parameters");
        }
        break;
    }
    case ESP_GAP_BLE_SET_PKT_LENGTH_COMPLETE_EVT:
    {
        if (ESP_BT_STATUS_SUCCESS == param->pkt_data_length_cmpl.status)
        {
            ESP_LOGI(TAG, "Pakcet length set complete(rx_len: %d and tx_len: %d)", param->pkt_data_length_cmpl.params.rx_len,
                     param->pkt_data_length_cmpl.params.tx_len);
        }
        else
        {
            ESP_LOGE(TAG, "Error setting packet length");
        }
        break;
    }
    default:
    {
        ESP_LOGE(TAG, "GAP event %d detected untracked!", event);
        break;
    }
    }
}
