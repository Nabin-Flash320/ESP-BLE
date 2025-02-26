| Supported Targets | ESP32 | ESP32-C2 | ESP32-C3 | ESP32-C5 | ESP32-C6 | ESP32-C61 | ESP32-H2 | ESP32-P4 | ESP32-S2 | ESP32-S3 | Linux |
| ----------------- | ----- | -------- | -------- | -------- | -------- | --------- | -------- | -------- | -------- | -------- | ----- |

# BLE in esp-idf

This repository can be used as a wrapper for BLE functionalities to the esp-idf project.

## How to use 
- Install esp-idf to your system
- Create a esp-idf project with BLE enabled.
- Clone this repository and add it to the component, you can delete the main directory, or start the project from itself.
- Create service callback source file within service_callbacks and provide source name in the CMakeLists.txt file in ble component.
- Then register the services and characteristics in ble.c file.
- Initialize the nvs flash and then call ble_init() function within the place of your liking.
- Finally build and flash the firmware to your
