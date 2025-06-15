
#include <stdlib.h>
#include <string.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/uart.h"
#include "argtable3/argtable3.h"
#include "esp_log.h"

#include "custom_console.h"

#define UART_NUM UART_NUM_0
#define BUF_SIZE (1024) // Buffer size for RX/TX
#define MAX_ARGS 10

static void parse_command(char *buffer, int *argc, char **argv);
static void custom_console_uart_read_task(void *params);

static console_command_handler ble_command_handler_func;
static console_command_handler ble_char_command_handler_func;

int custom_console_init()
{
    uart_config_t uart_config = {
        .baud_rate = 115200,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
    };

    // Install UART driver (disable RX/TX buffers since they are not needed for logging)
    ESP_ERROR_CHECK(uart_driver_install(UART_NUM, BUF_SIZE, BUF_SIZE, 0, NULL, 0));
    ESP_ERROR_CHECK(uart_param_config(UART_NUM, &uart_config));

    // Set UART0 to default TX and RX pins (no change needed)
    ESP_ERROR_CHECK(uart_set_pin(UART_NUM, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE));

    ESP_LOGI("UART0", "UART0 initialized for communication.");
    xTaskCreate(custom_console_uart_read_task, "cons-read-tsk", 4096, NULL, 3, NULL);
    return 0;
}

void custom_console_set_handler_func(console_command_handler ble_handler, console_command_handler ble_char_handler)
{
    ble_command_handler_func = ble_handler;
    ble_char_command_handler_func = ble_char_handler;
}

static void parse_command(char *buffer, int *argc, char **argv)
{
    *argc = 0;
    char *token = strtok(buffer, " \n"); // Tokenize by space or newline
    while (token != NULL && *argc < MAX_ARGS)
    {
        argv[(*argc)++] = token;
        token = strtok(NULL, " \n");
    }
}

static void custom_console_uart_read_task(void *params)
{
    size_t temp_data_len = 0;
    char *argv[MAX_ARGS];
    int argc = 0;
    esp_log_level_set("RX_TASK", ESP_LOG_INFO);
    while (1)
    {
        vTaskDelay(10 / portTICK_PERIOD_MS);
        ESP_ERROR_CHECK(uart_get_buffered_data_len(UART_NUM, &temp_data_len));
        if (0 == temp_data_len)
        {
            continue;
        }

        size_t buffer_size = temp_data_len + 1;
        uint8_t *received_data = (uint8_t *)malloc(buffer_size);
        if (!received_data)
        {
            continue;
        }

        memset(received_data, 0, buffer_size);
        size_t actual_bytes_read = uart_read_bytes(UART_NUM, received_data, buffer_size, 1000 / portTICK_PERIOD_MS);
        if (0 == actual_bytes_read)
        {
            continue;
        }

        received_data[actual_bytes_read] = '\0';
        printf("Received data is: %s, received bytes: %d\n", received_data, actual_bytes_read);
        parse_command((char *)received_data, &argc, argv);
        printf("Argc: %d\n", argc);
        if (argc < 0)
        {
            continue;
        }

        if (0 == strncmp("ble", argv[0], strlen("ble"))) 
        {
            printf("BLE command\n");
            if (ble_command_handler_func)
            {
                ble_command_handler_func(argc, argv);
            }
        }
        else if(0 == strncmp("char", argv[0], strlen("char")))
        {
            printf("BLE characteristics command\n");
            if(ble_char_command_handler_func)
            {
                ble_char_command_handler_func(argc, argv);
            }
        }
        else
        {
            printf("Wrong command provided\n");
        }

        argc = 0;
        free(received_data);
    }

    vTaskDelete(NULL);
}
