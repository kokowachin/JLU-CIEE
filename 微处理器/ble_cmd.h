#ifndef __BLE_CMD_H
#define __BLE_CMD_H

#include <stdint.h>

extern  char password[8];
// 命令处理函数声明
// 参数：cmd_str - 以 '\0' 结尾的完整命令字符串
// 返回值：0 表示处理成功，非0表示错误
uint8_t BLE_ProcessCommand(uint8_t *cmd_str);
void BLE_UpdateSensorData(uint8_t temp, uint8_t humi);
#endif