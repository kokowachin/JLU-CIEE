#include "main.h"
#include "dht11.h"
#include "centre_control.h"
#include "tim.h"
#include "ble_cmd.h"

uint32_t watercantick = 0;

/* 定义全局状态变量，初始值均为关闭/0档 */
ControlState g_control_state = {
    .fan   = FAN_OFF,
    .water = WATER_CLOSE
};

SystemConfig g_sys_config = {
    .fan_mode = 0,          // 默认自动
    .water_mode = 0,        // 默认自动
    .temp_threshold = 30,
    .humi_threshold = 40
};
//风扇转速挡位
void fan_rank(int rank)
{
    int duty_cycle = 0;
    switch (rank) {
        case FAN_LOW:   duty_cycle = 900; break;
        case FAN_MED:   duty_cycle = 600; break;
        case FAN_HIGH:  duty_cycle = 300; break;
        case FAN_OFF:  
        default:        duty_cycle = 1000; break;   // 关闭风扇（占空比1000可能为0%或100%，请根据实际PWM极性调整）
    }
    __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_2, duty_cycle);
    
    ///同步更新全局状态中的档位
    g_control_state.fan = (FanRank)rank;
}

//是否浇水
void water(int temp)
{
    if (temp == WATER_OPEN) {
        HAL_GPIO_WritePin(GPIOE, GPIO_PIN_5, GPIO_PIN_RESET);
			 
    } else {
        HAL_GPIO_WritePin(GPIOE, GPIO_PIN_5, GPIO_PIN_SET);
    }

    g_control_state.water = (WaterState)temp;

}

//自动控制逻辑
void auto_control(void)
{
    // 风扇控制
	switch (g_sys_config.fan_mode){
		case 1:
			fan_rank(FAN_OFF);
			break;
		case 2:
			fan_rank(FAN_LOW);
			break;
		case 3:
			fan_rank(FAN_MED);
			break;
		case 4:
			fan_rank(FAN_HIGH);
			break;
		default:		
			if (dht11_temperature_int >= g_sys_config.temp_threshold) {
        int diff = dht11_temperature_int - g_sys_config.temp_threshold;
        if (diff < 3) {
            fan_rank(FAN_LOW);
        } else if (diff < 5) {
            fan_rank(FAN_MED);
        } else {
            fan_rank(FAN_HIGH);
        }
    } else {
        fan_rank(FAN_OFF);
    }
	}

    uint8_t should_open = 0;
    switch(g_sys_config.water_mode){
        case 1:   // 强制关闭
            should_open = 0;
            break;
        case 2:   // 强制打开，但需容量>10
            should_open = (watercan > 10) ? 1 : 0;
            break;
        default:  // 自动模式
            if (dht11_humidity_int < g_sys_config.humi_threshold && watercan > 10)
                should_open = 1;
            else
                should_open = 0;
            break;
    }
    water(should_open ? WATER_OPEN : WATER_CLOSE);
	
}