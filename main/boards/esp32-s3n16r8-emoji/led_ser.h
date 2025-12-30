// #ifndef LED_SER_H
// #define LED_SER_H

// #include "led/single_led.h"
// #include "mcp_server.h"
// #include <esp_log.h>

// #define TAG "led_ser"

// class LED_ser {
// private:
//     gpio_num_t gpio_num_;
//     SingleLed led_;
// public: 
//     explicit LED_ser(gpio_num_t gpio_num) : gpio_num_(gpio_num), led_(gpio_num_) {  
//         ESP_LOGI(TAG, "LED_ser initialized on GPIO %d", gpio_num_);
//         auto& server = McpServer::GetInstance();
//         server.AddTool("turn_on_led", "Turn on LED",     
//                        PropertyList(), [this](const PropertyList&) {
//                            this->led_.TurnOn();  
//                            return "{led_status: \"on\"}"; 
//                        });

//         server.AddTool("turn_off_led", "Turn off LED",     
//                         PropertyList(), [this](const PropertyList&) {
//                             this->led_.TurnOff();
//                             return "{led_status: \"off\"}"; 
//                        });
//     }
// };

// #endif /* LED_SER_H */


#ifndef LED_SER_H
#define LED_SER_H

#include "mcp_server.h"
#include <esp_log.h>

#define TAG "led_ser"

class LED_ser{
private:
    bool power_ = false;
    gpio_num_t gpio_num_;

public:
    explicit LED_ser (gpio_num_t gpio_num) : gpio_num_(gpio_num) {
        gpio_config_t cfg = {
            .pin_bit_mask = (1ULL << gpio_num_),
            .mode = GPIO_MODE_OUTPUT,
            .pull_up_en = GPIO_PULLUP_DISABLE,
            .pull_down_en = GPIO_PULLDOWN_DISABLE,
            .intr_type = GPIO_INTR_DISABLE,
        };
        ESP_ERROR_CHECK(gpio_config(&cfg));
        gpio_set_level(gpio_num_, 1);       

        
        auto& server = McpServer::GetInstance();
        server.AddTool("获取开关状态", "返回灯的开/关状态",     
                       PropertyList(), [this](const PropertyList&) {

                           ESP_LOGW(TAG, "获取到了灯的当前状态，当前状态为%s", power_ ? "开" : "关");    
                           return power_ ? "{\"灯光状态：\":灯是开着的！}" : "{\"灯光状态：\":灯是关着的！}";  
                       
                       });
   
        server.AddTool("灯.打开", "打开灯",  
                       PropertyList(), [this](const PropertyList&) {
                           power_ = true;
                           gpio_set_level(gpio_num_, 1);    
                           ESP_LOGW(TAG, "已打开灯！");   
                           return true;     
                       });

        server.AddTool("灯.关闭", "关闭灯",     
                       PropertyList(), [this](const PropertyList&) {
                           power_ = false;
                           gpio_set_level(gpio_num_, 0);    
                           ESP_LOGW(TAG, "已关闭灯！");  
                           return true;   
                       });
    }
};


#endif /* LED_SER_H */