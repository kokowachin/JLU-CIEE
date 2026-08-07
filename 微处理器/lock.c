#include "lock.h"
#include <string.h>
#include "stm32f1xx_hal.h"
#include "tim.h"

#define CORRECT_PASSWORD "JLU01946"
#define MAX_ATTEMPTS 3

volatile uint8_t Lockin_Mode = 0;
static uint8_t error_count = 0;
uint32_t lock_start_time = 0;
volatile uint8_t beep_active = 0;
uint32_t beep_start_time = 0;
uint16_t beep_duration = 0;


void ring(uint16_t duration_ms,uint16_t pwms) {
    __HAL_TIM_SET_COMPARE(&htim4, TIM_CHANNEL_3, pwms); // 开启
    HAL_Delay(duration_ms);                            // 等待指定毫秒
    __HAL_TIM_SET_COMPARE(&htim4, TIM_CHANNEL_3, 0);   // 关闭
}

uint8_t check_password(const char *input_pwd) {
    if (Lockin_Mode == 3) {
        return 2;
    }
    if (strcmp(input_pwd, CORRECT_PASSWORD) == 0) {
        Lockin_Mode = 1;
        ring(300, 500);
        isWrite = 1;          // 正确登录，显示主界面
        error_count = 0;
        return 0;
    } else {
        error_count++;
        ring(200, 800);
        if (error_count >= MAX_ATTEMPTS) {
            Lockin_Mode = 3;
            lock_start_time = HAL_GetTick();
            isWrite = 0;      // 锁定，回到密码输入界面（显示锁定信息）
            return 2;
        } else {
            Lockin_Mode = 2;
            isWrite = 0;      // 错误，回到密码输入界面（显示错误信息）
            return 1;
        }
    }
}

void reset_lock_state(void) {
    Lockin_Mode = 0;
    error_count = 0;
    isWrite = 0;              // 重置后回到初始密码输入界面
}