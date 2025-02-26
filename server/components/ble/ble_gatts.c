
#include <string.h>

#include "esp_log.h"
#include "esp_gatts_api.h"

#include "ble.h"

#define TAG __FILE__

void ble_gatts_callback(esp_gatts_cb_event_t event, esp_gatt_if_t gatts_if, esp_ble_gatts_cb_param_t *param)
{
    switch (event)
    {
    case ESP_GATTS_MTU_EVT:
    {
        ESP_LOGI(TAG, "GATTS set MTU complete(conn_id: 0x%02x; mtu: %d)", param->mtu.conn_id, param->mtu.mtu);
        break;
    }
    case ESP_GATTS_CONF_EVT:
    {
        ESP_LOGI(TAG, "ESP_GATTS_CONF_EVT(status: %d; handle: %d; connid: %d)", param->conf.status, param->conf.handle, param->conf.conn_id);
        break;
    }
    case ESP_GATTS_UNREG_EVT:
    {
        ESP_LOGI(TAG, "ESP_GATTS_UNREG_EVT");
        break;
    }
    case ESP_GATTS_OPEN_EVT:
    {
        ESP_LOGI(TAG, "ESP_GATTS_OPEN_EVT(status: 0x%x)", param->open.status);
        break;
    }
    case ESP_GATTS_CANCEL_OPEN_EVT:
    {
        ESP_LOGI(TAG, "ESP_GATTS_CANCEL_OPEN_EVT(status: 0x%x)", param->cancel_open.status);
        break;
    }
    case ESP_GATTS_CLOSE_EVT:
    {
        ESP_LOGI(TAG, "ESP_GATTS_CLOSE_EVT(status: 0x%x; conn_id: 0x%02x)", param->close.status, param->close.conn_id);
        break;
    }
    case ESP_GATTS_REG_EVT:
    {
        if (ESP_GATT_OK != param->reg.status)
        {
            ESP_LOGE(TAG, "Failed to register(code: 0x%x)", param->reg.status);
        }
        else
        {
            ESP_LOGI(TAG, "GATTS register event for app id 0x%02X", param->reg.app_id);
            s_gatts_service_inst_t *service_instance = ble_gap_get_service_instance_by_id(param->reg.app_id);
            if (service_instance)
            {
                service_instance->gatts_if = gatts_if;
                esp_ble_gatts_create_service(service_instance->gatts_if, &service_instance->service_id, service_instance->num_handle);
            }
        }
        break;
    }
    case ESP_GATTS_CREATE_EVT:
    {
        if (ESP_GATT_OK != param->create.status)
        {
            ESP_LOGE(TAG, "Error creating service(0x%x)", param->create.status);
        }
        else
        {
            s_gatts_service_inst_t *service_instance = ble_gap_get_service_instance_by_gatts_if(gatts_if);
            if (service_instance)
            {
                service_instance->service_handle = param->create.service_handle;
                esp_ble_gatts_start_service(service_instance->service_handle);
            }
        }
        break;
    }
    case ESP_GATTS_START_EVT:
    {
        if (ESP_GATT_OK != param->start.status)
        {
            ESP_LOGE(TAG, "Error starting service(0x%x)", param->create.status);
        }
        else
        {
            s_gatts_service_inst_t *service_instance = ble_gap_get_service_instance_by_gatts_if(gatts_if);
            if (service_instance)
            {
                /*
                    Since the GATT is successful to start a service, now characteristics can be added.
                    service.characteristics_added will keep track of added characteristics to the service while working as an index to the characteristics array
                    in the service array.
                */
                if ((service_instance->characteristics_len > 0) && (service_instance->characteristics_added < service_instance->characteristics_len))
                {
                    esp_ble_gatts_add_char(service_instance->service_handle,
                                           &service_instance->characteristics[service_instance->characteristics_added].char_uuid,
                                           service_instance->characteristics[service_instance->characteristics_added].perm,
                                           service_instance->characteristics[service_instance->characteristics_added].property,
                                           NULL,
                                           NULL);
                }
            }
        }
        break;
    }
    case ESP_GATTS_ADD_CHAR_EVT:
    {
        if (ESP_GATT_OK != param->add_char.status)
        {
            ESP_LOGE(TAG, "Error adding characteristics(0x%02X)", param->add_char.status);
        }
        else
        {
            s_gatts_service_inst_t *service_instance = ble_gap_get_service_instance_by_gatts_if(gatts_if);
            if (service_instance)
            {
                s_gatts_char_inst_t *characteristics_instance = &service_instance->characteristics[service_instance->characteristics_added];
                if (characteristics_instance)
                {
                    characteristics_instance->added = true;
                    characteristics_instance->char_handle = param->add_char.attr_handle;
                    if ((characteristics_instance->descriptors_len > 0) && (characteristics_instance->descriptors_added < characteristics_instance->descriptors_len))
                    {
                        /*
                            Since the GATT is successful to add a characteristics to the provided service, now characteristic descriptor can be added.
                            characteristics.descriptor_added will keep track of added descriptors to the characteristics while working as an index to the descriptors array
                            in the characteristics array.
                        */
                        esp_ble_gatts_add_char_descr(service_instance->service_handle,
                                                     &characteristics_instance->descriptors[characteristics_instance->descriptors_added].descr_uuid,
                                                     characteristics_instance->descriptors[characteristics_instance->descriptors_added].perm,
                                                     &characteristics_instance->descriptors[characteristics_instance->descriptors_added].desc_val,
                                                     &characteristics_instance->descriptors[characteristics_instance->descriptors_added].ctrl);
                    }
                    else
                    {
                        /*
                            If no any descriptors are available to add, code can move forward to add other characteristics remaining.
                        */
                        service_instance->characteristics_added++;
                        if ((service_instance->characteristics_len > 0) && (service_instance->characteristics_added < service_instance->characteristics_len))
                        {
                            esp_ble_gatts_add_char(service_instance->service_handle,
                                                   &service_instance->characteristics[service_instance->characteristics_added].char_uuid,
                                                   service_instance->characteristics[service_instance->characteristics_added].perm,
                                                   service_instance->characteristics[service_instance->characteristics_added].property,
                                                   NULL,
                                                   NULL);
                        }
                    }
                }
            }
        }
        break;
    }
    case ESP_GATTS_ADD_CHAR_DESCR_EVT:
    {
        if (ESP_GATT_OK != param->add_char_descr.status)
        {
            ESP_LOGE(TAG, "Error adding characteristics descriptor(0x%02X)", param->add_char_descr.status);
        }
        else
        {
            s_gatts_service_inst_t *service_instance = ble_gap_get_service_instance_by_gatts_if(gatts_if);
            if (service_instance)
            {
                s_gatts_char_inst_t *characteristics_instance = &service_instance->characteristics[service_instance->characteristics_added];
                if (characteristics_instance)
                {
                    s_gatts_disc_inst_t *descriptor_instance = &characteristics_instance->descriptors[characteristics_instance->descriptors_added];
                    if (descriptor_instance)
                    {
                        descriptor_instance->added = true;
                        descriptor_instance->descr_handle = param->add_char_descr.attr_handle;
                        characteristics_instance->descriptors_added++;
                        if ((characteristics_instance->descriptors_len > 0) && (characteristics_instance->descriptors_added < characteristics_instance->descriptors_len))
                        {
                            /*
                                In case of multiple descriptor within a singe characteristics, the code will proceed to complete the remaining descriptros.
                            */
                            esp_ble_gatts_add_char_descr(service_instance->service_handle,
                                                         &characteristics_instance->descriptors[characteristics_instance->descriptors_added].descr_uuid,
                                                         characteristics_instance->descriptors[characteristics_instance->descriptors_added].perm,
                                                         NULL,
                                                         NULL);
                        }
                        else
                        {
                            service_instance->characteristics++;
                            if ((service_instance->characteristics_len > 0) && (service_instance->characteristics_added < service_instance->characteristics_len))
                            {
                                /*
                                    If no any descriptors are available to add, code can move forward to add other characteristics remaining.
                                */
                                esp_ble_gatts_add_char(service_instance->service_handle,
                                                       &service_instance->characteristics[service_instance->characteristics_added].char_uuid,
                                                       service_instance->characteristics[service_instance->characteristics_added].perm,
                                                       service_instance->characteristics[service_instance->characteristics_added].property,
                                                       NULL,
                                                       NULL);
                            }
                        }
                    }
                }
            }
        }
        break;
    }
    case ESP_GATTS_RESPONSE_EVT:
    {
        ESP_LOGI(TAG, "ESP_GATTS_RESPONSE_EVT(status: %d) for %d", param->rsp.status, param->rsp.handle);
        break;
    }
    // These are handled by the callbacks themselves.
    case ESP_GATTS_CONNECT_EVT:
    {
        ESP_LOGI(TAG, "ESP_GATTS_CONNECT_EVT\nConn id: %d\nRole: %d\nBLE address: %02x:%02x:%02x:%02x:%02x:%02x\nAddress type: 0x%02x", param->connect.conn_id,
                 param->connect.link_role, param->connect.remote_bda[0],
                 param->connect.remote_bda[1], param->connect.remote_bda[2],
                 param->connect.remote_bda[3], param->connect.remote_bda[4],
                 param->connect.remote_bda[5], param->connect.ble_addr_type);
        s_gatts_service_inst_t *service_instance = ble_gap_get_service_instance_by_gatts_if(gatts_if);
        if (service_instance && service_instance->gatts_cb)
        {
            service_instance->conn_id = param->connect.conn_id;
            memcpy(service_instance->remote_bda, param->connect.remote_bda, sizeof(esp_bd_addr_t));
            service_instance->gatts_cb(event, gatts_if, param);
        }
        break;
    }
    case ESP_GATTS_DISCONNECT_EVT:
    {
        ESP_LOGI(TAG, "ESP_GATTS_DISCONNECT_EVT(conn_id: 0x%02x; BLE address: %02x:%02x:%02x:%02x:%02x:%02x; reason: 0x%02x)", param->disconnect.conn_id, param->disconnect.remote_bda[0],
                 param->disconnect.remote_bda[1], param->disconnect.remote_bda[2],
                 param->disconnect.remote_bda[3], param->disconnect.remote_bda[4],
                 param->disconnect.remote_bda[5], param->disconnect.reason);
        ESP_LOGE(TAG, "Restarting device advertisement");
        ble_gap_start_ble_advertisement();
        s_gatts_service_inst_t *service_instance = ble_gap_get_service_instance_by_gatts_if(gatts_if);
        if (service_instance && service_instance->gatts_cb)
        {
            service_instance->gatts_cb(event, gatts_if, param);
        }
        break;
    }
    case ESP_GATTS_READ_EVT:
    case ESP_GATTS_WRITE_EVT:
    case ESP_GATTS_EXEC_WRITE_EVT:
    case ESP_GATTS_ADD_INCL_SRVC_EVT:
    case ESP_GATTS_DELETE_EVT:
    case ESP_GATTS_STOP_EVT:
    {
        s_gatts_service_inst_t *service_instance = ble_gap_get_service_instance_by_gatts_if(gatts_if);
        if (service_instance && service_instance->gatts_cb)
        {
            service_instance->gatts_cb(event, gatts_if, param);
        }
        break;
    }
    default:
    {
        ESP_LOGE(TAG, "GATTS event %d detected untracked", event);
        break;
    }
    }
}
