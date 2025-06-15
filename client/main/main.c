#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <inttypes.h>
#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "nvs_flash.h"
#include "esp_system.h"
#include "argtable3/argtable3.h"

#include "ble_client.h"
#include "custom_console.h"

static void console_ble_command_initializer();
static void console_ble_char_command_initializer();

static void handle_ble_command(int argc, char **argv);
static void handle_ble_char_command(int argc, char **argv);

static struct
{
    struct arg_int *start;
    struct arg_str *server_name;
    struct arg_end *end;
} ble_cmd_arg_table;

static struct
{
    struct arg_int *ble_svc_id;
    struct arg_int *ble_char_id;
    struct arg_str *ble_char_val;
    struct arg_end *end;
} ble_char_cmd_arg_table;

void app_main(void)
{
    esp_err_t ret;

    // Initialize NVS.
    ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    console_ble_command_initializer();
    console_ble_char_command_initializer();
    custom_console_init();
    custom_console_set_handler_func(handle_ble_command, handle_ble_char_command);
    printf("ble --start=1 --name=BLE_device_33_4d_d2\nchar --svcid=0 --chrid=0 --chrval=Data_to_send\n");
}

static void console_ble_command_initializer()
{
    ble_cmd_arg_table.start = arg_int1("s", "start", "<int>", "Start BLE client");
    ble_cmd_arg_table.server_name = arg_str1("n", "name", "<str>", "Name of the server to connect to");
    ble_cmd_arg_table.end = arg_end(2);
}

static void console_ble_char_command_initializer()
{
    ble_char_cmd_arg_table.ble_svc_id = arg_int1("s", "svcid", "<int>", "Service ID characteristics belongs to");
    ble_char_cmd_arg_table.ble_char_id = arg_int1("i", "chrid", "<int>", "Characteristics ID to read/write");
    ble_char_cmd_arg_table.ble_char_val = arg_str0("v", "chrval", "<str>", "Value to send to the characteristics");
    ble_char_cmd_arg_table.end = arg_end(3);
}

static void handle_ble_command(int argc, char **argv)
{
    if (argc <= 0)
    {
        return;
    }

    int nerrors = arg_parse(argc, argv, (void **)&ble_cmd_arg_table);
    if (nerrors != 0)
    {
        printf("Error parsing argument\n");
        return;
    }

    const int *start = ble_cmd_arg_table.start->ival;
    const char *server_name = ble_cmd_arg_table.server_name->sval[0];
    if (server_name)
    {
        printf("Setting server name to %s\n", server_name);
        ble_set_server_name(server_name);
    }

    if (1 == *start)
    {
        ble_init();
    }
}

static void handle_ble_char_command(int argc, char **argv)
{
    if (argc <= 0)
    {
        return;
    }

    int nerrors = arg_parse(argc, argv, (void **)&ble_char_cmd_arg_table);
    if (nerrors != 0)
    {
        printf("Error parsing argument\n");
        return;
    }

    const int *svc_id = ble_char_cmd_arg_table.ble_svc_id->ival;
    const int *char_id = ble_char_cmd_arg_table.ble_char_id->ival;
    const char *char_val = ble_char_cmd_arg_table.ble_char_val->sval[0];
    if (char_val && 0 != strlen(char_val))
    {
        printf("Sending `%s` tp char `%d`\n", char_val, *char_id);
        ble_write_char(*svc_id, *char_id, char_val, strlen(char_val));
    }
    else 
    {
        ble_read_char(*svc_id, *char_id);
    }
}