#ifndef __temp_h
#define __temp_h

void init_temp_sensor(void);
float get_temp(void);

#include "mcp_server.h"
#include <esp_log.h>
#define TAG "ds18b20"

class DS18B20Sensor {       
private:
    float temperature;
    gpio_num_t gpio_num_;

public: 
    explicit DS18B20Sensor(gpio_num_t gpio_num) : gpio_num_(gpio_num) {
        
        //ESP_LOGI(TAG, "DS18B20Sensor initialized on GPIO %d", gpio_num_);
        auto& server = McpServer::GetInstance();
        init_temp_sensor();
        server.AddTool("获得温度", "返回温度值",     
                       PropertyList(), [this](const PropertyList&) {
                            temperature=get_temp();
                            ESP_LOGW(TAG, "获取到了温度值：%f", temperature); 
                           return "{\"温度值\": " + std::to_string(temperature) + "}";      
                       });
    }
};


#endif /* __temp_h */
  //Example of using DS18B20 temperature sensor with 1-Wire bus on ESP32