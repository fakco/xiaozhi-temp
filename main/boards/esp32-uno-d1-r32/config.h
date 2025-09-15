/**
 * @file config.h
 * @brief ESP32 UNO D1 R32开发板配置文件
 * 
 * 本文件定义了ESP32 UNO D1 R32开发板的针脚配置和其他参数
 */

#ifndef _BOARD_CONFIG_H_
#define _BOARD_CONFIG_H_

#include <driver/gpio.h>

// 音频采样率配置
#define AUDIO_INPUT_SAMPLE_RATE  16000
#define AUDIO_OUTPUT_SAMPLE_RATE 24000  // 与服务器匹配的采样率，避免重采样

// 音频缓冲区和DMA配置
#define AUDIO_DMA_DESC_NUM       32     // 进一步增加DMA描述符数量，提高缓冲能力
#define AUDIO_DMA_FRAME_NUM      1024   // 进一步增加DMA帧数，减少缓冲区不足导致的卡顿
#define AUDIO_BATCH_SIZE         128    // 调整批处理大小，平衡处理延迟和平滑度

// I2S时钟配置
#define AUDIO_MCLK_MULTIPLE      256    // 使用标准的256倍数，提高兼容性

// 使用Simplex I2S模式
#define AUDIO_I2S_METHOD_SIMPLEX

// INMP441麦克风I2S配置
#define AUDIO_I2S_MIC_GPIO_WS   GPIO_NUM_16  // WS (Word Select)
#define AUDIO_I2S_MIC_GPIO_SCK  GPIO_NUM_14  // SCK (Serial Clock)
#define AUDIO_I2S_MIC_GPIO_DIN  GPIO_NUM_17  // SD (Serial Data)

// MAX98357A音频放大器I2S配置
#define AUDIO_I2S_SPK_GPIO_DOUT GPIO_NUM_25  // DIN
#define AUDIO_I2S_SPK_GPIO_BCLK GPIO_NUM_26  // BCLK
#define AUDIO_I2S_SPK_GPIO_LRCK GPIO_NUM_27  // LRC
// 注意：MAX98357A的SD引脚应该保持悬空，不要接GND，否则没有声音输出

// 音频音量设置（0-100）
#define AUDIO_VOLUME            90     // 调整音量到更高水平，提高信号强度

// 触摸传感器配置
#define TOUCH_SENSOR_GPIO       GPIO_NUM_4   // 触摸传感器引脚
#define TOUCH_THRESHOLD         1000         // 触摸阈值（电容式触摸传感器）
#define TOUCH_SENSOR_TYPE       1            // 0: ESP32内置电容式触摸传感器, 1: 外部TTP223触摸模块

// 按钮配置
#define BOOT_BUTTON_GPIO        GPIO_NUM_0   // 板载BOOT按钮
#define ASR_BUTTON_GPIO         GPIO_NUM_19  // 语音识别按钮（可选）
#define BUILTIN_LED_GPIO        GPIO_NUM_2   // 板载LED

// 按钮长按时间定义（毫秒）
#define BUTTON_LONG_PRESS_TIME  1000         // 1秒

// 舵机配置
#define SERVO_HORIZONTAL_PIN    GPIO_NUM_19  // 水平舵机控制引脚
#define SERVO_VERTICAL_PIN      GPIO_NUM_18  // 垂直舵机控制引脚

// OLED显示屏I2C配置
#define DISPLAY_SDA_PIN GPIO_NUM_21
#define DISPLAY_SCL_PIN GPIO_NUM_22
#define DISPLAY_WIDTH   128

#if CONFIG_OLED_SSD1306_128X32
#define DISPLAY_HEIGHT  32
#elif CONFIG_OLED_SSD1306_128X64
#define DISPLAY_HEIGHT  64
#else
#define DISPLAY_HEIGHT  64  // 默认使用128x64 OLED
#endif

#define DISPLAY_MIRROR_X true
#define DISPLAY_MIRROR_Y true

#endif // _BOARD_CONFIG_H_
