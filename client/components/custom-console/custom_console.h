
#ifndef __CUSTOM_CONSOLE_H__
#define __CUSTOM_CONSOLE_H__

typedef void (*console_command_handler)(int argc, char **argv);

int custom_console_init();
void custom_console_set_handler_func(console_command_handler ble_handler, console_command_handler ble_char_handler);

#endif // __CUSTOM_CONSOLE_H__
