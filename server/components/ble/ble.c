
#include <string.h>

#include "esp_mac.h"
#include "esp_log.h"
#include "esp_err.h"
#include "esp_bt.h"
#include "esp_bt_main.h"
#include "esp_bt_defs.h"
#include "esp_gap_ble_api.h"
#include "esp_gatts_api.h"
#include "esp_gatt_defs.h"

#include "ble.h"

#define BLE_TAG __FILE__
#define WRITE_CHAR_DESCRIPTOR_1_ATTR_VAL "Write only property"
#define READ_CHAR_DESCRIPTOR_1_ATTR_VAL "Read only property"

static s_gatts_disc_inst_t example_service_write_char_descriptor[EXAMPLE_SERVICE_WRITE_CHAR_DESCRIPTOR_MAX] = {
    [EXAMPLE_SERVICE_WRITE_CHAR_DESCRIPTOR_1] = {
        .descr_uuid = {
            .len = ESP_UUID_LEN_128,
            .uuid.uuid128 = BLE_EXAMPLE_SERVICE_WRITE_CHAR_DESCRIPTOR_1_UUID,
        },
        .perm = ESP_GATT_PERM_READ,
        .desc_val = {
            .attr_max_len = 64,
            .attr_len = sizeof(WRITE_CHAR_DESCRIPTOR_1_ATTR_VAL),
            .attr_value = (uint8_t *)WRITE_CHAR_DESCRIPTOR_1_ATTR_VAL,
        },
        .ctrl.auto_rsp = ESP_GATT_AUTO_RSP,
    },
};

static s_gatts_disc_inst_t example_service_read_char_descriptor[EXAMPLE_SERVICE_READ_CHAR_DESCRIPTOR_MAX] = {
    [EXAMPLE_SERVICE_WRITE_CHAR_DESCRIPTOR_1] = {
        .descr_uuid = {
            .len = ESP_UUID_LEN_128,
            .uuid.uuid128 = BLE_EXAMPLE_SERVICE_READ_CHAR_DESCRIPTOR_1_UUID,
        },
        .perm = ESP_GATT_PERM_READ,
        .desc_val = {
            .attr_max_len = 64,
            .attr_len = sizeof(READ_CHAR_DESCRIPTOR_1_ATTR_VAL),
            .attr_value = (uint8_t *)READ_CHAR_DESCRIPTOR_1_ATTR_VAL,
        },
        .ctrl.auto_rsp = ESP_GATT_AUTO_RSP,
    },
};

static s_gatts_char_inst_t example_service_characteristics[EXAMPLE_CHAR_ID_MAX] = {
    [EXAMPLE_CHAR_ID_WRITE] = {
        .char_uuid = {
            .len = ESP_UUID_LEN_128,
            .uuid = {
                .uuid128 = BLE_EXAMPLE_SERVICE_WRITE_CHAR_UUID,
            },
        },
        .perm = ESP_GATT_PERM_WRITE,
        .property = ESP_GATT_CHAR_PROP_BIT_WRITE,
        .descriptors_len = EXAMPLE_SERVICE_WRITE_CHAR_DESCRIPTOR_MAX,
        .descriptors = example_service_write_char_descriptor,
        .descriptors_added = 0,
        .added = false,
    },
    [EXAMPLE_CHAR_ID_READ] = {
        .char_uuid = {
            .len = ESP_UUID_LEN_128,
            .uuid = {
                .uuid128 = BLE_EXAMPLE_SERVICE_READ_CHAR_UUID,
            },
        },
        .perm = ESP_GATT_PERM_READ,
        .property = ESP_GATT_CHAR_PROP_BIT_READ,
        .descriptors_len = EXAMPLE_SERVICE_READ_CHAR_DESCRIPTOR_MAX,
        .descriptors = example_service_read_char_descriptor,
        .descriptors_added = 0,
        .added = false,
    },
};

/*
    This array holds all the details about required for creating a service along with len and pointer to the characteristics to respective profiles/services.
*/
static s_gatts_service_inst_t gatt_profiles[BLE_PROFILE_ID_MAX] = {
    [BLE_PROFILE_ID_EXAMPLE] = {
        .gatts_cb = ble_example_service_callback,
        .gatts_if = ESP_GATT_IF_NONE,
        .profile_id = BLE_PROFILE_ID_EXAMPLE,
        .characteristics_len = EXAMPLE_CHAR_ID_MAX,
        .characteristics = example_service_characteristics,
        .characteristics_added = 0,
        // this can also be calculated dinamically before creating the service
        .num_handle = 1 + (2 * EXAMPLE_CHAR_ID_MAX) + EXAMPLE_SERVICE_READ_CHAR_DESCRIPTOR_MAX + EXAMPLE_SERVICE_WRITE_CHAR_DESCRIPTOR_MAX, //  1 for service, 2 per characteristics and 1 per descriptors
        .service_id = {
            .id = {
                .uuid = {
                    .len = ESP_UUID_LEN_128,
                    .uuid.uuid128 = BLE_EXAMPLE_SERVICE_UUID,
                },
                .inst_id = 0x00,
            },
            .is_primary = true,
        },
    },
};

esp_ble_adv_data_t adv_data = {
    .set_scan_rsp = false,
    .include_name = true,
    .include_txpower = true,
    .min_interval = 0x0006,
    .max_interval = 0x0010,
    .appearance = 0,
    .manufacturer_len = 0,
    .p_manufacturer_data = NULL,
    .service_data_len = 0,
    .p_service_data = NULL,
    .service_uuid_len = 0,
    .p_service_uuid = NULL,
    .flag = (ESP_BLE_ADV_FLAG_GEN_DISC | ESP_BLE_ADV_FLAG_BREDR_NOT_SPT),
};

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

    return ESP_OK;
}

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

    ESP_ERROR_CHECK(esp_ble_gatts_register_callback(ble_gatts_callback));
    ESP_ERROR_CHECK(esp_ble_gap_register_callback(ble_gap_callbacak));
    ble_gap_set_ble_device_name();
    esp_ble_gatts_app_register(BLE_PROFILE_ID_EXAMPLE);
    ret = esp_ble_gap_config_adv_data(&adv_data);
    if (ret)
    {
        ESP_LOGE(BLE_TAG, "config adv data failed, error code = %s", esp_err_to_name(ret));
    }
    esp_ble_gatts_show_local_database();
}

s_gatts_service_inst_t *ble_gap_get_service_instance_by_id(e_ble_profile_ids_t profile_id)
{
    return &gatt_profiles[profile_id];
}

s_gatts_service_inst_t *ble_gap_get_service_instance_by_gatts_if(esp_gatt_if_t gatts_if)
{
    for (int i = 0; i < BLE_PROFILE_ID_MAX; i++)
    {
        s_gatts_service_inst_t *service_instance = ble_gap_get_service_instance_by_id(i);
        if (service_instance && (gatts_if == service_instance->gatts_if) && service_instance->gatts_cb)
        {
            return service_instance;
        }
    }
    return NULL;
}

s_gatts_service_inst_t *ble_gap_get_service_instance_by_service_handle(uint16_t service_handle)
{
    for (int i = 0; i < BLE_PROFILE_ID_MAX; i++)
    {
        s_gatts_service_inst_t *service_instance = ble_gap_get_service_instance_by_id(i);
        if (service_instance && (service_handle == service_instance->service_handle))
        {
            return service_instance;
        }
    }
    return NULL;
}
