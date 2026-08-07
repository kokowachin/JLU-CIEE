#ifndef __CENTRE_CONTROL_H
#define __CENTRE_CONTROL_H

#include <stdint.h>

typedef struct {
    uint8_t fan_mode;    // 0=自动, 1=关闭, 2=低, 3=中, 4=高
    uint8_t water_mode;  // 0=自动, 1=关闭, 2=开启
    uint8_t temp_threshold;
    uint8_t humi_threshold;
} SystemConfig;

extern SystemConfig g_sys_config;   // 全局配置变量声明

/* 风扇档位枚举（0~3档，0表示关闭） */
typedef enum {
    FAN_OFF = 0,
    FAN_LOW = 1,
    FAN_MED = 2,
    FAN_HIGH = 3
} FanRank;

/* 水闸状态枚举（0关闭，1开启） */
typedef enum {
    WATER_CLOSE = 0,
    WATER_OPEN  = 1
} WaterState;

/* 整体控制状态结构体 */
typedef struct {
    FanRank    fan;    // 当前风扇档位
    WaterState water;  // 当前水闸状态
} ControlState;


/* 全局状态变量声明（定义在 .c 中） */
extern ControlState g_control_state;

/* 原有函数声明（若需要可保留） */
void fan_rank(int rank);
void water(int temp);
void auto_control(void);

#endif