#ifndef LOCK_H
#define LOCK_H

#include <stdint.h>
#include "stm32f1xx_hal.h"

extern volatile uint8_t Lockin_Mode;   // 0=等待输入, 1=解锁成功, 2=密码错误, 3=锁定
extern uint32_t lock_start_time;   // 新增

// 验证密码，返回 0=成功, 1=错误未锁定, 2=错误已锁定
uint8_t check_password(const char *input_pwd);
void ring(uint16_t duration_ms,uint16_t pwms);

// 复位锁定状态（可在超时后调用）
void reset_lock_state(void);

#endif