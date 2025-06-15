
#ifndef __BLE_CLIENT_H__
#define __BLE_CLIENT_H__

#include "esp_gap_ble_api.h"
#include "esp_gattc_api.h"

#define BLE_EXAMPLE_SERVICE_UUID {0x2e, 0xd1, 0x6a, 0x66, 0x9b, 0xcf, 0x4f, 0xb3, 0x9d, 0x09, 0x0e, 0x35, 0x0e, 0xa8, 0xe3, 0xef}

#define BLE_EXAMPLE_SERVICE_WRITE_CHAR_UUID {0x6b, 0xdf, 0xd2, 0xad, 0xcc, 0x68, 0x42, 0xe6, 0x88, 0xf1, 0x46, 0x3c, 0x2e, 0xf9, 0x84, 0xea}
#define BLE_EXAMPLE_SERVICE_READ_CHAR_UUID {0xb6, 0xc1, 0xba, 0x10, 0x0f, 0x87, 0x48, 0x96, 0xb7, 0xf0, 0x65, 0x99, 0x0c, 0x20, 0x16, 0x5f}

#define BLE_EXAMPLE_SERVICE_WRITE_CHAR_DESCRIPTOR_1_UUID {0x6c, 0x45, 0x36, 0x0d, 0x3a, 0x33, 0x47, 0x83, 0x84, 0x7b, 0xef, 0xda, 0x9b, 0x21, 0x08, 0xdf}
#define BLE_EXAMPLE_SERVICE_READ_CHAR_DESCRIPTOR_1_UUID {0x9c, 0x54, 0x8e, 0x0c, 0xde, 0x1b, 0x44, 0x3b, 0x8d, 0x59, 0x6e, 0x98, 0x15, 0x03, 0x49, 0x0c}

typedef enum e_ble_profile_ids
{
    BLE_PROFILE_ID_EXAMPLE,
    BLE_PROFILE_ID_MAX,
} e_ble_profile_ids_t;

/* ==================== Security service enums ==================== */
typedef enum e_ble_service_security_char_ids
{
    BLE_SECURE_SESSION_CHAR_WRITE,
    BLE_SECURE_SESSION_CHAR_READ,
    BLE_SECURE_SESSION_CHAR_ID_MAX,
} e_ble_service_security_char_ids_t;

typedef struct s_gattc_characteristics_instance
{
    uint16_t char_handle;
    esp_gatt_char_prop_t property;
    esp_bt_uuid_t uuid;
    esp_gattc_descr_elem_t *descriptors;
    uint16_t descriptor_size;
} s_gattc_characteristics_instance_t;

typedef struct s_gattc_service_instance
{
    esp_gattc_cb_t gattc_cb;
    uint16_t gattc_if;
    uint16_t profile_id;
    uint16_t conn_id;
    uint16_t service_handle_start;
    uint16_t service_handle_end;
    esp_bd_addr_t remote_bda;
    esp_gatt_srvc_id_t service_id;
    s_gattc_characteristics_instance_t *characteristics;
    uint16_t characteristics_size;
} s_gattc_service_instance_t;

void ble_init();
void ble_set_scan_parameters();
void ble_start_scanning();
s_gattc_service_instance_t *ble_gattc_get_service_instance_by_gattc_if(uint16_t gattc_if);
s_gattc_service_instance_t *ble_gattc_get_service_instance_by_id(uint16_t app_id);
void ble_gap_callbacak(esp_gap_ble_cb_event_t event, esp_ble_gap_cb_param_t *param);
void ble_gattc_callback(esp_gattc_cb_event_t event, esp_gatt_if_t gattc_if, esp_ble_gattc_cb_param_t *param);
void ble_set_server_name(const char *name);

int ble_write_char(uint16_t service_id, uint16_t char_id, const char *data, size_t data_len);
int ble_read_char(uint16_t service_id, uint16_t char_id);

void ble_gattc_example_service_callback(esp_gattc_cb_event_t event, esp_gatt_if_t gattc_if, esp_ble_gattc_cb_param_t *param);

#endif // __BLE_CLIENT_H__
