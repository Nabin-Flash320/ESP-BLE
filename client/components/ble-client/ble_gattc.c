
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
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "ble_client.h"

#define TAG "BLE Gattc"

static void ble_gattc_populate_characteristics(esp_gatt_if_t gattc_if, esp_ble_gattc_cb_param_t *param);
static void ble_gattc_populate_characteristic_descriptors(esp_gatt_if_t gattc_if, esp_ble_gattc_cb_param_t *param);
static void ble_gattc_log_ble_profile_db();

static bool scan_param_set = false;

void ble_gattc_callback(esp_gattc_cb_event_t event, esp_gatt_if_t gattc_if, esp_ble_gattc_cb_param_t *param)
{
    switch (event)
    {
    case ESP_GATTC_REG_EVT:
    {
        ESP_LOGI(TAG, "ESP_GATTC_REG_EVT");
        if (ESP_GATT_OK == param->reg.status)
        {
            s_gattc_service_instance_t *service_instance = ble_gattc_get_service_instance_by_id(param->reg.app_id);
            if (service_instance)
            {
                service_instance->gattc_if = gattc_if;
                if (!scan_param_set)
                {
                    ESP_LOGI(TAG, "Setting scan parameters");
                    ble_set_scan_parameters();
                    scan_param_set = true;
                }
            }
        }
        break;
    }
    case ESP_GATTC_OPEN_EVT:
    {
        ESP_LOGI(TAG, "ESP_GATTC_OPEN_EVT(status: %d, mtu: %d)", param->open.status, param->open.mtu);
        break;
    }
    case ESP_GATTC_CONNECT_EVT:
    {
        ESP_LOGI(TAG, "ESP_GATTC_CONNECT_EVT");
        s_gattc_service_instance_t *service_instance = ble_gattc_get_service_instance_by_gattc_if(gattc_if);
        if (service_instance)
        {
            service_instance->gattc_cb(event, gattc_if, param);
            service_instance->conn_id = param->connect.conn_id;
            memcpy(service_instance->remote_bda, param->connect.remote_bda, sizeof(esp_bd_addr_t));
            ESP_ERROR_CHECK_WITHOUT_ABORT(esp_ble_gattc_send_mtu_req(gattc_if, param->connect.conn_id));
        }
        break;
    }
    case ESP_GATTC_CFG_MTU_EVT:
    {
        ESP_LOGI(TAG, "ESP_GATTC_CFG_MTU_EVT(status: %d, mtu: %d)", param->cfg_mtu.status, param->cfg_mtu.mtu);
        break;
    }
    case ESP_GATTC_DIS_SRVC_CMPL_EVT:
    {
        if (ESP_GATT_OK == param->dis_srvc_cmpl.status)
        {
            ESP_LOGI(TAG, "ESP_GATTC_DIS_SRVC_CMPL_EVT");
            ESP_ERROR_CHECK_WITHOUT_ABORT(esp_ble_gattc_search_service(gattc_if, param->dis_srvc_cmpl.conn_id, NULL));
        }
        else
        {
            ESP_LOGE(TAG, "Error on discover service complete(status: %d)", param->dis_srvc_cmpl.status);
        }
        break;
    }
    case ESP_GATTC_SEARCH_RES_EVT:
    {
        ESP_LOGI(TAG, "ESP_GATTC_SEARCH_RES_EVT(start: %d, end: %d)", param->search_res.start_handle, param->search_res.end_handle);
        for (int i = 0; i < BLE_PROFILE_ID_MAX; i++)
        {
            s_gattc_service_instance_t *service_instance = ble_gattc_get_service_instance_by_id(i);
            if (service_instance)
            {
                if ((ESP_UUID_LEN_128 == service_instance->service_id.id.uuid.len) && (0 == memcmp(param->search_res.srvc_id.uuid.uuid.uuid128, service_instance->service_id.id.uuid.uuid.uuid128, 16)))
                {
                    service_instance->service_handle_start = param->search_res.start_handle;
                    service_instance->service_handle_end = param->search_res.end_handle;
                    service_instance->service_id.id.inst_id = param->search_res.srvc_id.inst_id;
                }
            }
        }
        break;
    }
    case ESP_GATTC_SEARCH_CMPL_EVT:
    {
        if (ESP_GATT_OK == param->search_cmpl.status)
        {
            ESP_LOGI(TAG, "ESP_GATTC_SEARCH_CMPL_EVT(service source: %d)", param->search_cmpl.searched_service_source);
            ble_gattc_populate_characteristics(gattc_if, param);
            ble_gattc_populate_characteristic_descriptors(gattc_if, param);
            ble_gattc_log_ble_profile_db();

            s_gattc_service_instance_t *service_instance = ble_gattc_get_service_instance_by_id(BLE_PROFILE_ID_EXAMPLE);
            if (service_instance && service_instance->gattc_cb)
            {
                service_instance->gattc_cb(event, gattc_if, param);
            }
        }
        else
        {
            ESP_LOGE(TAG, "Error at search complete event(status: %d)", param->search_cmpl.status);
        }
        break;
    }
    case ESP_GATTC_CLOSE_EVT:
    {
        ESP_LOGE(TAG, "ESP_GATTC_CLOSE_EVT");
        scan_param_set = false;
        break;
    }
    case ESP_GATTC_DISCONNECT_EVT:
    {
        ESP_LOGE(TAG, "ESP_GATTC_DISCONNECT_EVT");
        if (!scan_param_set)
        {
            s_gattc_service_instance_t *service_instance = ble_gattc_get_service_instance_by_gattc_if(gattc_if);
            if (service_instance && service_instance->gattc_cb)
            {
                service_instance->gattc_cb(event, gattc_if, param);
            }

            ble_set_scan_parameters();
            scan_param_set = true;
        }
        break;
    }
    case ESP_GATTC_NOTIFY_EVT:
    case ESP_GATTC_WRITE_CHAR_EVT:
    case ESP_GATTC_READ_CHAR_EVT:
    {
        s_gattc_service_instance_t *service_instance = ble_gattc_get_service_instance_by_gattc_if(gattc_if);
        if (service_instance && service_instance->gattc_cb)
        {
            service_instance->gattc_cb(event, gattc_if, param);
        }
        break;
    }
    case ESP_GATTC_REG_FOR_NOTIFY_EVT:
    {
        if (ESP_GATT_OK == param->reg_for_notify.status)
        {
            ESP_LOGI(TAG, "Notification enabled for handle %d", param->reg_for_notify.handle);
        }
        else
        {
            ESP_LOGE(TAG, "Failed to register for notification for handle: %d", param->reg_for_notify.handle);
        }
        break;
    }
    default:
    {
        ESP_LOGI(TAG, "GATTC event %d unhandled", event);
        break;
    }
    }
}

// Comparison function
static int compare_char_handles(const void *a, const void *b)
{
    return ((s_gattc_characteristics_instance_t *)a)->char_handle -
           ((s_gattc_characteristics_instance_t *)b)->char_handle;
}

static void ble_gattc_populate_characteristics(esp_gatt_if_t gattc_if, esp_ble_gattc_cb_param_t *param)
{
    for (int i = 0; i < BLE_PROFILE_ID_MAX; i++)
    {
        s_gattc_service_instance_t *service_instance = ble_gattc_get_service_instance_by_id(i);
        if (service_instance)
        {
            uint16_t attribute_count = 0;
            esp_gatt_status_t status = esp_ble_gattc_get_attr_count(gattc_if,
                                                                    param->search_cmpl.conn_id,
                                                                    ESP_GATT_DB_CHARACTERISTIC,
                                                                    service_instance->service_handle_start,
                                                                    service_instance->service_handle_end,
                                                                    ESP_GATT_INVALID_HANDLE,
                                                                    &attribute_count);
            if (ESP_GATT_OK != status)
            {
                ESP_LOGE(TAG, "Error getting attribute count(status: %d)", status);
                break;
            }

            if (attribute_count > 0)
            {
                esp_gattc_char_elem_t *characteristics = (esp_gattc_char_elem_t *)malloc(sizeof(esp_gattc_char_elem_t) * attribute_count);
                if (!characteristics)
                {
                    ESP_LOGE(TAG, "Error allocating memory for characteristics elements");
                    break;
                }

                service_instance->characteristics_size = attribute_count;
                status = esp_ble_gattc_get_all_char(gattc_if,
                                                    param->search_cmpl.conn_id,
                                                    service_instance->service_handle_start,
                                                    service_instance->service_handle_end,
                                                    characteristics,
                                                    &service_instance->characteristics_size,
                                                    0);
                if (ESP_GATT_OK != status)
                {
                    ESP_LOGE(TAG, "Error getting all characteristics(status: %d)", status);
                    free(characteristics);
                    free(service_instance->characteristics);
                    service_instance->characteristics = NULL;
                    service_instance->characteristics_size = 0;
                    break;
                }

                service_instance->characteristics = (s_gattc_characteristics_instance_t *)malloc(sizeof(s_gattc_characteristics_instance_t) * service_instance->characteristics_size);
                if (service_instance->characteristics)
                {
                    memset(service_instance->characteristics, 0, sizeof(s_gattc_characteristics_instance_t) * service_instance->characteristics_size);
                    for (int i = 0; i < service_instance->characteristics_size; i++)
                    {
                        service_instance->characteristics[i].char_handle = characteristics[i].char_handle;
                        service_instance->characteristics[i].property = characteristics[i].properties;
                        memcpy(&service_instance->characteristics[i].uuid, &characteristics[i].uuid, sizeof(esp_bt_uuid_t));
                    }
                }

                qsort(service_instance->characteristics, service_instance->characteristics_size, sizeof(s_gattc_characteristics_instance_t), compare_char_handles);
                free(characteristics);
            }
        }
    }
}

static void ble_gattc_populate_characteristic_descriptors(esp_gatt_if_t gattc_if, esp_ble_gattc_cb_param_t *param)
{
    for (int service_id = 0; service_id < BLE_PROFILE_ID_MAX; service_id++)
    {
        s_gattc_service_instance_t *service_instance = ble_gattc_get_service_instance_by_id(service_id);
        if (service_instance && (service_instance->characteristics_size > 0) && service_instance->characteristics)
        {
            for (int char_idx = 0; char_idx < service_instance->characteristics_size; char_idx++)
            {
                uint16_t attribute_count = 0;
                uint16_t start_handle = service_instance->characteristics[char_idx].char_handle + 1;
                uint16_t end_handle = char_idx < service_instance->characteristics_size - 1 ? service_instance->characteristics[char_idx + 1].char_handle - 1 : service_instance->service_handle_end;
                printf("Start: %d and End: %d\n", start_handle, end_handle);
                esp_gatt_status_t status = esp_ble_gattc_get_attr_count(gattc_if,
                                                                        param->search_cmpl.conn_id,
                                                                        ESP_GATT_DB_DESCRIPTOR,
                                                                        start_handle,
                                                                        end_handle,
                                                                        service_instance->characteristics[char_idx].char_handle,
                                                                        &attribute_count);
                if (ESP_GATT_OK != status)
                {
                    ESP_LOGE(TAG, "Error getting attribute count(status: %d)", status);
                    break;
                }

                if (attribute_count > 0)
                {
                    service_instance->characteristics[char_idx].descriptors = (esp_gattc_descr_elem_t *)malloc(sizeof(esp_gattc_descr_elem_t) * attribute_count);
                    if (!service_instance->characteristics[char_idx].descriptors)
                    {
                        ESP_LOGE(TAG, "Error allocating memory for characteristics descriptor element");
                        break;
                    }

                    service_instance->characteristics[char_idx].descriptor_size = attribute_count;
                    status = esp_ble_gattc_get_all_descr(gattc_if,
                                                         param->search_cmpl.conn_id,
                                                         service_instance->characteristics[char_idx].char_handle,
                                                         service_instance->characteristics[char_idx].descriptors,
                                                         &service_instance->characteristics[char_idx].descriptor_size,
                                                         0);

                    if (ESP_GATT_OK != status)
                    {
                        ESP_LOGE(TAG, "Error getting all characteristics(status: %d)", status);
                        free(service_instance->characteristics[char_idx].descriptors);
                        service_instance->characteristics[char_idx].descriptors = NULL;
                        service_instance->characteristics[char_idx].descriptor_size = 0;
                        break;
                    }
                }
            }
        }
    }
}

void print_uuid_128(const uint8_t *uuid_128)
{
    // Print the UUID in the standard format: xxxxxxxx-xxxx-xxxx-xxxx-xxxxxxxxxxxx
    printf(" UUID: %02X%02X%02X%02X-%02X%02X-%02X%02X-%02X%02X-%02X%02X%02X%02X%02X%02X;",
           uuid_128[0], uuid_128[1], uuid_128[2], uuid_128[3],
           uuid_128[4], uuid_128[5], uuid_128[6], uuid_128[7],
           uuid_128[8], uuid_128[9], uuid_128[10], uuid_128[11],
           uuid_128[12], uuid_128[13], uuid_128[12], uuid_128[15]);
}

static void ble_gattc_log_ble_profile_db()
{
    printf("************************* Services Start *************************\n");
    for (int i = 0; i < BLE_PROFILE_ID_MAX; i++)
    {
        s_gattc_service_instance_t *service_instance = ble_gattc_get_service_instance_by_id(i);
        if (service_instance)
        {
            printf("[Service %d]\n\tStart handle: %d; End handle: %d; Is primary: %d; Instance ID: %d;", i + 1, service_instance->service_handle_start,
                   service_instance->service_handle_end,
                   service_instance->service_id.is_primary,
                   service_instance->service_id.id.inst_id);
            if (ESP_UUID_LEN_16 == service_instance->service_id.id.uuid.len)
            {
                printf(" UUID: 0x%04X;", service_instance->service_id.id.uuid.uuid.uuid16);
            }
            else if (ESP_UUID_LEN_32 == service_instance->service_id.id.uuid.len)
            {
                printf(" UUID: 0x%04lX;", service_instance->service_id.id.uuid.uuid.uuid32);
            }
            else if (ESP_UUID_LEN_128 == service_instance->service_id.id.uuid.len)
            {
                print_uuid_128(service_instance->service_id.id.uuid.uuid.uuid128);
            }

            if (service_instance->characteristics_size > 0 && service_instance->characteristics)
            {
                printf(" Characteristics count: %d\n", service_instance->characteristics_size);
                for (int i = 0; i < service_instance->characteristics_size; i++)
                {
                    printf("\t[Characteristic: %d]\n\t\tHandle: %d; Property: 0x%X;", i + 1, service_instance->characteristics[i].char_handle, service_instance->characteristics[i].property);
                    if (ESP_UUID_LEN_16 == service_instance->characteristics[i].uuid.len)
                    {
                        printf(" UUID: 0x%04X;", service_instance->characteristics[i].uuid.uuid.uuid16);
                    }
                    else if (ESP_UUID_LEN_32 == service_instance->characteristics[i].uuid.len)
                    {
                        printf(" UUID: 0x%04lX;", service_instance->characteristics[i].uuid.uuid.uuid32);
                    }
                    else if (ESP_UUID_LEN_128 == service_instance->characteristics[i].uuid.len)
                    {
                        print_uuid_128(service_instance->characteristics[i].uuid.uuid.uuid128);
                    }

                    printf(" Descriptor count: %d\n", service_instance->characteristics[i].descriptor_size);
                    if ((service_instance->characteristics[i].descriptor_size > 0) && service_instance->characteristics[i].descriptors)
                    {
                        for (int j = 0; j < service_instance->characteristics[i].descriptor_size; j++)
                        {
                            printf("\t\t[Descriptor: %d]\n\t\t\tHandle: %d;", j + 1, service_instance->characteristics[i].descriptors[j].handle);
                            if (ESP_UUID_LEN_16 == service_instance->characteristics[i].descriptors[j].uuid.len)
                            {
                                printf(" UUID: 0x%04X;\n", service_instance->characteristics[i].descriptors[j].uuid.uuid.uuid16);
                            }
                            else if (ESP_UUID_LEN_32 == service_instance->characteristics[i].descriptors[j].uuid.len)
                            {
                                printf(" UUID: 0x%04lX;\n", service_instance->characteristics[i].descriptors[j].uuid.uuid.uuid32);
                            }
                            else if (ESP_UUID_LEN_128 == service_instance->characteristics[i].descriptors[j].uuid.len)
                            {
                                print_uuid_128(service_instance->characteristics[i].descriptors[j].uuid.uuid.uuid128);
                                printf("\n");
                            }
                        }
                    }
                }
            }
        }
    }
    printf("************************* Services End *************************\n");
}
