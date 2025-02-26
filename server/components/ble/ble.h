
#ifndef _BLE_H_
#define _BLE_H_

#include "esp_gap_ble_api.h"
#include "esp_gatts_api.h"

#define BLE_GATTS_MAX_CHAR_LEN 5

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

/* ==================== Service example write characcteristics enums ==================== */
typedef enum e_ble_example_service_write_char_descriptor
{
    EXAMPLE_SERVICE_WRITE_CHAR_DESCRIPTOR_1,
    EXAMPLE_SERVICE_WRITE_CHAR_DESCRIPTOR_MAX,
} e_ble_example_service_write_char_descriptor_t;

/* ==================== Service example read characcteristics enums ==================== */
typedef enum e_ble_example_service_read_char_descriptor
{
    EXAMPLE_SERVICE_READ_CHAR_DESCRIPTOR_1,
    EXAMPLE_SERVICE_READ_CHAR_DESCRIPTOR_MAX,
} e_ble_example_service_read_char_descriptor_t;

/* ==================== Service example characcteristics enums ==================== */
typedef enum e_ble_example_char_ids
{
    EXAMPLE_CHAR_ID_WRITE,
    EXAMPLE_CHAR_ID_READ,
    EXAMPLE_CHAR_ID_MAX,
} e_ble_example_char_ids_t;

/* ==================== BLE service structs ==================== */
typedef struct s_gatts_disc_inst
{
    uint16_t descr_handle;
    esp_bt_uuid_t descr_uuid;
    esp_gatt_perm_t perm;
    esp_attr_value_t desc_val;
    esp_attr_control_t ctrl;
    bool added;
} s_gatts_disc_inst_t;

typedef struct s_gatts_char_inst
{
    uint16_t char_handle;
    esp_bt_uuid_t char_uuid;
    esp_gatt_perm_t perm;
    esp_gatt_char_prop_t property;
    uint8_t descriptors_len;
    s_gatts_disc_inst_t *descriptors;
    uint8_t descriptors_added;
    bool added;
} s_gatts_char_inst_t;

typedef struct s_gatts_service_inst
{
    esp_gatts_cb_t gatts_cb;
    uint8_t gatts_if;
    uint16_t profile_id;
    uint16_t conn_id;
    uint16_t service_handle;
    esp_gatt_srvc_id_t service_id;
    uint8_t characteristics_len;
    s_gatts_char_inst_t *characteristics;
    uint8_t characteristics_added;
    uint8_t num_handle;
    esp_bd_addr_t remote_bda;
} s_gatts_service_inst_t;

void ble_init();
void ble_gap_start_ble_advertisement();
s_gatts_service_inst_t *ble_gap_get_service_instance_by_id(e_ble_profile_ids_t profile_id);
s_gatts_service_inst_t *ble_gap_get_service_instance_by_gatts_if(esp_gatt_if_t gatts_if);
s_gatts_service_inst_t *ble_gap_get_service_instance_by_service_handle(uint16_t service_handle);
void ble_gap_callbacak(esp_gap_ble_cb_event_t event, esp_ble_gap_cb_param_t *param);
void ble_gatts_callback(esp_gatts_cb_event_t event, esp_gatt_if_t gatts_if, esp_ble_gatts_cb_param_t *param);

/*** ================================================================ Service callbacks definitions start ================================================================ ***/

void ble_example_service_callback(esp_gatts_cb_event_t event, esp_gatt_if_t gatts_if, esp_ble_gatts_cb_param_t *param);

/*** ================================================================ Service callbacks definitions end ================================================================ ***/

#endif //  _BLE_H_
