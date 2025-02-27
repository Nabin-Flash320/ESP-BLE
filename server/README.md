
## How to use 
- Install esp-idf to your system
- Create a esp-idf project with BLE enabled, or sdkconfig can be copied for BLE configuration.
- Clone this repository and add server/client to the component, you can delete the main directory, or start the project from itself.
- Create service callback source file within service_callbacks and provide source name in the CMakeLists.txt file in ble component.
- Then register the services and characteristics in ble.c file.
- Initialize the nvs flash and then call ble_init() function within the place of your liking.
```c
    ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);
    ble_init();
```
- Finally build and flash the firmware to your

## Insight

#### Example service

1. **UUID definitions**

    Services, characteristics and descriptors requires a universal unique identifier(UUID); this helps to distinguish the BLE attributes during communications. Here, in this example, 128-bit UUIDs are used
    ```c
    // UUID for example service
    #define BLE_EXAMPLE_SERVICE_UUID {0x2e, 0xd1, 0x6a, 0x66, 0x9b, 0xcf, 0x4f, 0xb3, 0x9d, 0x09, 0x0e, 0x35, 0x0e, 0xa8, 0xe3, 0xef}

    // UUIDs for example service characteristics
    #define BLE_EXAMPLE_SERVICE_WRITE_CHAR_UUID {0x6b, 0xdf, 0xd2, 0xad, 0xcc, 0x68, 0x42, 0xe6, 0x88, 0xf1, 0x46, 0x3c, 0x2e, 0xf9, 0x84, 0xea}
    #define BLE_EXAMPLE_SERVICE_READ_CHAR_UUID {0xb6, 0xc1, 0xba, 0x10, 0x0f, 0x87, 0x48, 0x96, 0xb7, 0xf0, 0x65, 0x99, 0x0c, 0x20, 0x16, 0x5f}

    // UUIDs for characteristics descriptors
    #define BLE_EXAMPLE_SERVICE_WRITE_CHAR_DESCRIPTOR_1_UUID {0x6c, 0x45, 0x36, 0x0d, 0x3a, 0x33, 0x47, 0x83, 0x84, 0x7b, 0xef, 0xda, 0x9b, 0x21, 0x08, 0xdf}
    #define BLE_EXAMPLE_SERVICE_READ_CHAR_DESCRIPTOR_1_UUID {0x9c, 0x54, 0x8e, 0x0c, 0xde, 0x1b, 0x44, 0x3b, 0x8d, 0x59, 0x6e, 0x98, 0x15, 0x03, 0x49, 0x0c}

    ```
2. **Profile/Service ID definition and instance table:**

    Each of the profiles/services are registered using an id [0 .... (2<sup>16</sup>-1)] as an app; so an enum is defined for keeping profile ids in check
    ```c
    typedef enum e_ble_profile_ids
    {
        BLE_PROFILE_ID_EXAMPLE,
        BLE_PROFILE_ID_MAX,
    } e_ble_profile_ids_t;
    ```
    To track the created services and information surrounding it, following is the service instance definition struct
    ```c
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
    ```

    Following is the instance creation process
    ```c
    static s_gatts_service_inst_t gatt_profiles[BLE_PROFILE_ID_MAX] = {
        [BLE_PROFILE_ID_EXAMPLE] = {
            .gatts_cb = ble_example_service_callback,
            .gatts_if = ESP_GATT_IF_NONE,
            .profile_id = BLE_PROFILE_ID_EXAMPLE,
            .characteristics_len = EXAMPLE_CHAR_ID_MAX,
            .characteristics = example_service_characteristics,
            .characteristics_added = 0,
            //  1 per service, 2 per characteristic and 1 per descriptor
            .num_handle = 1 + (2 * EXAMPLE_CHAR_ID_MAX) + EXAMPLE_SERVICE_READ_CHAR_DESCRIPTOR_MAX + EXAMPLE_SERVICE_WRITE_CHAR_DESCRIPTOR_MAX, 
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
    ```
3. **Characteristics instance table:**

    Characteristics are linked to a specific service using `characteristics` member of the service instance structure.
    `characteristics_len` holds the number of charcteristics and `characteristics_added` member holds the index of the characteristics that will be added to the BLE attribute database next.
    An enum is defined to index the characteristics for a service
    ```c
    /* ==================== Service example characteristics enums ==================== */
    typedef enum e_ble_example_char_ids
    {
        EXAMPLE_CHAR_ID_WRITE,
        EXAMPLE_CHAR_ID_READ,
        EXAMPLE_CHAR_ID_MAX,
    } e_ble_example_char_ids_t;
    ```
    Following is the characteristics instance definition
    ```c
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
    ```

    Following is the characteristics instance creation process
    ```c
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
    ```
4. **Descriptor instance table:**

    Descriptors are linked to a specific characteristic using `descriptors` member of the characteristic instance structure.
    `descriptors_len` holds the number of charcteristics and `descriptors_added` member holds the index of the descriptors that will be added to the BLE attribute database next.
    Enums per characteristics are created to keep track of descriptors
    ```c
    /* ==================== Service example write characteristics enums ==================== */
    typedef enum e_ble_example_service_write_char_descriptor
    {
        EXAMPLE_SERVICE_WRITE_CHAR_DESCRIPTOR_1,
        EXAMPLE_SERVICE_WRITE_CHAR_DESCRIPTOR_MAX,
    } e_ble_example_service_write_char_descriptor_t;

    /* ==================== Service example read characteristics enums ==================== */
    typedef enum e_ble_example_service_read_char_descriptor
    {
        EXAMPLE_SERVICE_READ_CHAR_DESCRIPTOR_1,
        EXAMPLE_SERVICE_READ_CHAR_DESCRIPTOR_MAX,
    } e_ble_example_service_read_char_descriptor_t;

    ```
    Following is the descriptor instance definition struct
    ```c
    typedef struct s_gatts_disc_inst
    {
        uint16_t descr_handle;
        esp_bt_uuid_t descr_uuid;
        esp_gatt_perm_t perm;
        esp_attr_value_t desc_val;
        esp_attr_control_t ctrl;
        bool added;
    } s_gatts_disc_inst_t;
    ```
    Following is the descriptor instance creation process
    ```c
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
    ```

### Example code flow
1. **Start bluetooth and bluedriod stack:**

    1. Initialize nvs flash
        ```c
        ret = nvs_flash_init();
        if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND)
        {
            ESP_ERROR_CHECK(nvs_flash_erase());
            ret = nvs_flash_init();
        }
        ```
    2. Initialize and enable bluetooth controller in BLE mode
        ```c
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
        ```
    3. Initialize and enable bluedroid stack for BLE operations
        ```c
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
        ```
    4. Register GAP and GATT server callback function
        ```c
        ESP_ERROR_CHECK(esp_ble_gatts_register_callback(ble_gatts_callback));
        ESP_ERROR_CHECK(esp_ble_gap_register_callback(ble_gap_callbacak));
        ```
        GAP callback handles event related to connection; GAP manages the BLE device and its configurations
        GATTS callback handles the event related to changes in device states, communication between devices, etc.
        GATTS also handles service, characteristic and descriptor database creation through events. This is the place where service callbacks are called from, gatts_cb member in s_gatts_service_inst_t struct.
    5. Register each services as app, here, service/profiees are registered as app providing the profile id mentioned above. Then add advertisement data.
        ```c
        esp_ble_gatts_app_register(BLE_PROFILE_ID_EXAMPLE);
        ret = esp_ble_gap_config_adv_data(&adv_data);
        if (ret)
        {
            ESP_LOGE(BLE_TAG, "config adv data failed, error code = %s", esp_err_to_name(ret));
        }
        ```
2. **Registration, creation and addition of service, characteristics and descriptors:**

    After the BLE service/profile is registered, the registration of attrubutes are handled in GATTS callback when ESP_GATTS_REG_EVT event is triggered on `esp_ble_gatts_app_register` function call.
    1. ESP_GATTS_REG_EVT
        1. Service is provided with the an interface number as `gatts_if` wich will be stored in service instance struct.
        3. Create service function is called to create service within the BLE stack
        4. num_handle is the total number of handle to be created, a service reqires 1 handle, each of the service characteristics requires 2 handles and each of the characteristic descriptor required 1 handle.
        ```c
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
        ```
    2. `esp_ble_gatts_create_service` function triggers `ESP_GATTS_CREATE_EVT` event
        1. This event will provide service handle
        2. If service creation is successfully done, it has to be started using `esp_ble_gatts_start_service` function
        ```c
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
        ```
    3. `esp_ble_gatts_start_service` function triggers `ESP_GATTS_START_EVT` event
        1. If service is started successfully, start adding characteristics 
        ```c
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
        ```
    4. `esp_ble_gatts_add_char` function triggers `ESP_GATTS_ADD_CHAR_EVT` event
        1. If successfully added the characteristics, add the attriburte handle to the `char_handle` member of characteristics instance.
        2. Check if descriptors are associated to the characteristics, if yes add the descriptor
        3. If no descriptors are available, the advance to the next characteristics
        ```c
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
        ```
    5. `esp_ble_gatts_add_char_descr` function triggers `ESP_GATTS_ADD_CHAR_DESCR_EVT` event
        1. If successfully added the descsriptor, add the attriburte handle to the `descr_handle` member of descriptor instance.
        2. Check if descriptors are associated to the characteristics are still pending, if yes add the descriptor
        3. If no descriptors are available, the advance to the next characteristics
        ```c
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
        ```
3. **BLE GATTS event handling:**

    Since not all of the GATTS events are required to be processed on the service callback, only `ESP_GATTS_CONNECT_EVT`, `ESP_GATTS_DISCONNECT_EVT`, `ESP_GATTS_READ_EVT` and `ESP_GATTS_WRITE_EVT` events are passed to the service callback. However, events can be passed to the callback as per the requirement.
    Following is the code for example service callback handler
    ```c
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
    ```

# Thank You

