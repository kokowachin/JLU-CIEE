#ifndef __DHT11_H
#define __DHT11_H

#include "stm32f1xx_hal.h"

// ------------------- 用户配置区域 -------------------
#define DHT11_GPIO_PORT   GPIOB         // 根据实际连接修改
#define DHT11_PIN         GPIO_PIN_15   // 与 main.h 的 dht11_Pin 保持一致
// ---------------------------------------------------

// 存储读取结果：湿度整数、湿度小数、温度整数、温度小数
extern volatile uint8_t dht11_humidity_int;
extern volatile uint8_t dht11_humidity_dec;
extern volatile uint8_t dht11_temperature_int;
extern volatile uint8_t dht11_temperature_dec;

// 外部引用定时器句柄（在 tim.c 中已定义）
extern TIM_HandleTypeDef htim2;

// 函数声明
void DHT11_Init(void);             // 可选，若需额外初始化
uint8_t DHT11_ReadData(void);      // 读取一次数据，返回 0 成功，1 失败
void DHT11_Delay_us(uint16_t us);  // 微秒延时（基于定时器）

#endif