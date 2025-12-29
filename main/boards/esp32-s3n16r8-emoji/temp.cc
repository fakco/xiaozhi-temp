#include <stdio.h>
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "onewire_bus.h"
#include "ds18b20.h"

#define EXAMPLE_ONEWIRE_BUS_GPIO    13
#define EXAMPLE_ONEWIRE_MAX_DS18B20 1

static const char *TAG = "example";

// 全局变量
static onewire_bus_handle_t bus = NULL;
static ds18b20_device_handle_t ds18b20s[EXAMPLE_ONEWIRE_MAX_DS18B20];
static int ds18b20_device_num = 0;
static bool is_initialized = false;

esp_err_t init_temp_sensor(void)
{
    // 安装1-Wire总线
    onewire_bus_config_t bus_config = {
        .bus_gpio_num = EXAMPLE_ONEWIRE_BUS_GPIO,
        .flags = {
            .en_pull_up = true,
        }
    };
    onewire_bus_rmt_config_t rmt_config = {
        .max_rx_bytes = 10,
    };
    ESP_ERROR_CHECK(onewire_new_bus_rmt(&bus_config, &rmt_config, &bus));
    ESP_LOGI(TAG, "1-Wire bus installed on GPIO%d", EXAMPLE_ONEWIRE_BUS_GPIO);

    // 创建设备迭代器
    onewire_device_iter_handle_t iter = NULL;
    onewire_device_t next_onewire_device;
    esp_err_t search_result = ESP_OK;

    ESP_ERROR_CHECK(onewire_new_device_iter(bus, &iter));
    ESP_LOGI(TAG, "Device iterator created, start searching...");
    
    do {
        search_result = onewire_device_iter_get_next(iter, &next_onewire_device);
        if (search_result == ESP_OK) {
            ds18b20_config_t ds_cfg = {};
            onewire_device_address_t address;
            if (ds18b20_new_device_from_enumeration(&next_onewire_device, &ds_cfg, 
                &ds18b20s[ds18b20_device_num]) == ESP_OK) {
                ds18b20_get_device_address(ds18b20s[ds18b20_device_num], &address);
                ESP_LOGI(TAG, "Found a DS18B20[%d], address: %016llX", 
                         ds18b20_device_num, address);
                ds18b20_device_num++;
                if (ds18b20_device_num >= EXAMPLE_ONEWIRE_MAX_DS18B20) {
                    ESP_LOGI(TAG, "Max DS18B20 number reached, stop searching...");
                    break;
                }
            } else {
                ESP_LOGI(TAG, "Found an unknown device, address: %016llX", 
                         next_onewire_device.address);
            }
        }
    } while (search_result != ESP_ERR_NOT_FOUND);
    
    ESP_ERROR_CHECK(onewire_del_device_iter(iter));
    ESP_LOGI(TAG, "Searching done, %d DS18B20 device(s) found", ds18b20_device_num);
    
    if (ds18b20_device_num > 0) {
        is_initialized = true;
        return ESP_OK;
    } else {
        return ESP_ERR_NOT_FOUND;
    }
}

bool is_temp_sensor_initialized(void)
{
    return is_initialized;
}

float read_temperature(int sensor_index)
{
    if (!is_initialized) {
        ESP_LOGE(TAG, "Temperature sensor not initialized!");
        return -127.0f;
    }
    
    if (sensor_index < 0 || sensor_index >= ds18b20_device_num) {
        ESP_LOGE(TAG, "Invalid sensor index!");
        return -127.0f;
    }
    
    float temperature;
    esp_err_t ret;
    
    // 触发温度转换
    ret = ds18b20_trigger_temperature_conversion_for_all(bus);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to trigger temperature conversion: %s", 
                 esp_err_to_name(ret));
        return -127.0f;
    }
    
    // DS18B20需要时间进行温度转换（最大750ms）
    vTaskDelay(pdMS_TO_TICKS(750));
    
    // 读取温度
    ret = ds18b20_get_temperature(ds18b20s[sensor_index], &temperature);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to read temperature: %s", esp_err_to_name(ret));
        return -127.0f;
    }
    
    return temperature;
}

// 简化版本，只读取第一个传感器
float get_temp(void)
{
    return read_temperature(0);
}