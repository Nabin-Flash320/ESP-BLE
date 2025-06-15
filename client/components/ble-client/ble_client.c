
#include "string.h"

#include "esp_log.h"
#include "esp_err.h"

#include "esp_mac.h"
#include "esp_bt.h"
#include "esp_gap_ble_api.h"
#include "esp_gattc_api.h"
#include "esp_gatt_defs.h"
#include "esp_bt_main.h"
#include "esp_gatt_common_api.h"

#include "ble_client.h"

#define BLE_TAG "BLE Client"

static esp_err_t ble_gap_set_ble_device_name();

static s_gattc_service_instance_t ble_profiles[BLE_PROFILE_ID_MAX] = {
    [BLE_PROFILE_ID_EXAMPLE] = {
        .gattc_cb = ble_gattc_example_service_callback,
        .gattc_if = ESP_GATT_IF_NONE,
        .profile_id = BLE_PROFILE_ID_EXAMPLE,
        .service_id = {
            .is_primary = true,
            .id = {
                .uuid = {
                    .len = ESP_UUID_LEN_128,
                    .uuid = {
                        .uuid128 = BLE_EXAMPLE_SERVICE_UUID,
                    },
                },
            },
        },
    },
};

static esp_ble_scan_params_t ble_scan_params = {
    .scan_type = BLE_SCAN_TYPE_ACTIVE,
    .own_addr_type = BLE_ADDR_TYPE_PUBLIC,
    .scan_filter_policy = BLE_SCAN_FILTER_ALLOW_ALL,
    .scan_interval = 0x50,
    .scan_window = 0x30,
    .scan_duplicate = BLE_SCAN_DUPLICATE_DISABLE,
};

void ble_init()
{
    ESP_ERROR_CHECK(esp_bt_controller_mem_release(ESP_BT_MODE_CLASSIC_BT));
    esp_bt_controller_config_t bt_cfg = BT_CONTROLLER_INIT_CONFIG_DEFAULT();
    esp_err_t ret = esp_bt_controller_init(&bt_cfg);
    if (ret)
    {
        ESP_LOGE(BLE_TAG, "%s initialize controller failed: %s", __func__, esp_err_to_name(ret));
        return;
    }

    ret = esp_bt_controller_enable(ESP_BT_MODE_BLE);
    if (ret)
    {
        ESP_LOGE(BLE_TAG, "%s enable controller failed: %s", __func__, esp_err_to_name(ret));
        return;
    }

    ret = esp_bluedroid_init();
    if (ret)
    {
        ESP_LOGE(BLE_TAG, "%s init bluetooth failed: %s", __func__, esp_err_to_name(ret));
        return;
    }
    ret = esp_bluedroid_enable();
    if (ret)
    {
        ESP_LOGE(BLE_TAG, "%s enable bluetooth failed: %s", __func__, esp_err_to_name(ret));
        return;
    }

    ESP_ERROR_CHECK(esp_ble_gattc_register_callback(ble_gattc_callback));
    ESP_ERROR_CHECK(esp_ble_gap_register_callback(ble_gap_callbacak));
    ble_gap_set_ble_device_name();
    if (ret)
    {
        ESP_LOGE(BLE_TAG, "config adv data failed, error code = %s", esp_err_to_name(ret));
    }
}

void ble_set_scan_parameters()
{
    esp_err_t error = esp_ble_gap_set_scan_params(&ble_scan_params);
    if (error)
    {
        ESP_LOGE(BLE_TAG, "set local  MTU failed, error code = %x", error);
    }
}

s_gattc_service_instance_t *ble_gattc_get_service_instance_by_gattc_if(uint16_t gattc_if)
{
    for (int i = 0; i < BLE_PROFILE_ID_MAX; i++)
    {
        s_gattc_service_instance_t *service_instance = ble_gattc_get_service_instance_by_id(i);
        if (service_instance && (gattc_if == service_instance->gattc_if))
        {
            return service_instance;
        }
    }

    return NULL;
}

s_gattc_service_instance_t *ble_gattc_get_service_instance_by_id(uint16_t app_id)
{
    return &ble_profiles[app_id];
}

int ble_write_char(uint16_t service_id, uint16_t char_id, const char *data, size_t data_len)
{
    if (NULL == data || service_id > BLE_PROFILE_ID_MAX)
    {
        return -1;
    }

    s_gattc_service_instance_t *service_instance = ble_gattc_get_service_instance_by_id(service_id);
    if (service_instance)
    {
        if (service_instance->characteristics)
        {
            ESP_ERROR_CHECK(esp_ble_gattc_write_char(
                service_instance->gattc_if,
                service_instance->conn_id,
                service_instance->characteristics[char_id].char_handle,
                data_len,
                (uint8_t *)data,
                ESP_GATT_WRITE_TYPE_NO_RSP,
                ESP_GATT_AUTH_REQ_NONE));
        }
    }

    return 0;
}

int ble_read_char(uint16_t service_id, uint16_t char_id)
{

    s_gattc_service_instance_t *service_instance = ble_gattc_get_service_instance_by_id(service_id);
    if (service_instance)
    {
        if (service_instance->characteristics)
        {
            ESP_ERROR_CHECK(esp_ble_gattc_read_char(
                service_instance->gattc_if,
                service_instance->conn_id,
                service_instance->characteristics[char_id].char_handle,
                ESP_GATT_AUTH_REQ_NONE));
        }
    }

    return 0;
}

static esp_err_t ble_gap_set_ble_device_name()
{
    uint8_t ble_mac[6];
    if (ESP_OK != esp_read_mac(ble_mac, ESP_MAC_BT))
    {
        return ESP_FAIL;
    }

    char *ble_name_prefix = "BLE_device";
    char ble_dev_name[30];
    memset(ble_dev_name, 0, sizeof(ble_dev_name));
    snprintf(ble_dev_name, sizeof(ble_dev_name), "%s_%x_%x_%x", ble_name_prefix, ble_mac[3], ble_mac[4], ble_mac[5]);

    ESP_LOGI(BLE_TAG, "Setting device name");
    ESP_ERROR_CHECK(esp_ble_gap_set_device_name(ble_dev_name));
    ESP_LOGI(BLE_TAG, "BLE device name successfully set to %s", ble_dev_name);
    esp_err_t error = esp_ble_gattc_app_register(BLE_PROFILE_ID_EXAMPLE);
    if (error)
    {
        ESP_LOGE(BLE_TAG, "%s gattc app register failed, error code = %x", __func__, error);
    }

    error = esp_ble_gatt_set_local_mtu(500);
    if (error)
    {
        ESP_LOGE(BLE_TAG, "set local  MTU failed, error code = %x", error);
    }

    return ESP_OK;
}
