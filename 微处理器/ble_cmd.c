#include "ble_cmd.h"
#include "main.h"           // 包含GPIO操作宏和全局变量声明
#include <string.h>
#include <stdio.h>          // 如果需要sprintf
#include "centre_control.h"
#include "lock.h"

extern UART_HandleTypeDef huart1;   
extern uint8_t g_ble_rx_data[];     
// 存储密码的缓冲区
char password[8];

// 历史数据存储（环形缓冲区，保留最新5组）
#define HISTORY_SIZE 5
typedef struct {
    uint8_t temp;
    uint8_t humi;
    uint32_t tick;      // 采集时刻（可用于调试，此处未使用）
} SensorHistory_t;

static SensorHistory_t g_history[HISTORY_SIZE];
static uint8_t g_history_index = 0;      // 下一次写入的位置
static uint8_t g_history_count = 0;      // 已存储的有效数据个数（最大5）

// 更新历史数据（由主循环调用）
void BLE_UpdateSensorData(uint8_t temp, uint8_t humi)
{
    g_history[g_history_index].temp = temp;
    g_history[g_history_index].humi = humi;
    g_history[g_history_index].tick = HAL_GetTick();
    g_history_index = (g_history_index + 1) % HISTORY_SIZE;
    if (g_history_count < HISTORY_SIZE) {
        g_history_count++;
    }
}

//发送反馈字符串到蓝牙
static void BLE_SendAck(const char *ack_str)
{
    HAL_UART_Transmit(&huart1, (uint8_t*)ack_str, strlen(ack_str), 100);
}

uint8_t BLE_ProcessCommand(uint8_t *cmd_str)
{
    // 1. 去除末尾换行符（兼容 \n, \r, \r\n）
    size_t len = strlen((char*)cmd_str);
    if (len > 0) {
        if (cmd_str[len-1] == '\n' || cmd_str[len-1] == '\r') {
            cmd_str[len-1] = '\0';
            if (len > 1 && cmd_str[len-2] == '\r') {
                cmd_str[len-2] = '\0';
            }
        }
    }

    // 2. 处理 "hello"
    if (strcmp((char*)cmd_str, "hello") == 0) {
        BLE_SendAck("world\n");
        return 0;
    }

    // 3. 风扇控制
    if (strcmp((char*)cmd_str, "FAN_LOW") == 0) {
        g_sys_config.fan_mode = 2;
        BLE_SendAck("FAN_LOW_OK\n");
        return 0;
    }
		else if (strcmp((char*)cmd_str, "FAN_MID") == 0) {
        g_sys_config.fan_mode = 3;
        BLE_SendAck("FAN_MID_OK\n");
        return 0;
    }
		else if (strcmp((char*)cmd_str, "FAN_HIGH") == 0) {
        g_sys_config.fan_mode = 4;
        BLE_SendAck("FAN_HIGH_OK\n");
        return 0;
    }
    else if (strcmp((char*)cmd_str, "FAN_OFF") == 0) {
        g_sys_config.fan_mode = 1;
        BLE_SendAck("FAN_OFF_OK\n");
        return 0;
    }
		else if (strcmp((char*)cmd_str, "FAN_AUTO") == 0){
			  g_sys_config.fan_mode = 0;
        BLE_SendAck("FAN_AUTO_OK\n");
        return 0;
		}

    // 4. 水闸控制
    else if (strcmp((char*)cmd_str, "WATER_ON") == 0) {
        g_sys_config.water_mode = 2;
        BLE_SendAck("WATER_ON_OK\n");
        return 0;
    }
    else if (strcmp((char*)cmd_str, "WATER_OFF") == 0) {
        g_sys_config.water_mode = 1;
        BLE_SendAck("WATER_OFF_OK\n");
        return 0;
    }
		else if (strcmp((char*)cmd_str, "WATER_AUTO") == 0) {
        g_sys_config.water_mode = 0;
        BLE_SendAck("WATER_AUTO_OK\n");
        return 0;
		}

    // 5. 解析 "temp:数值" 和 "humid:数值"
    int value;
    if (sscanf((char*)cmd_str, "temp:%d", &value) == 1) {
        if (value > 0 && value < 100) {
            g_sys_config.temp_threshold = (uint8_t)value;
            char ack[30];
            sprintf(ack, "TEMP_SET:%d\n", value);
            BLE_SendAck(ack);
            return 0;
        } else {
            BLE_SendAck("INVALID_VALUE\n");
            return 1;
        }
    }
    else if (sscanf((char*)cmd_str, "humid:%d", &value) == 1) {
        if (value > 0 && value < 100) {
            g_sys_config.humi_threshold = (uint8_t)value;
            char ack[30];
            sprintf(ack, "HUMI_SET:%d\n", value);
            BLE_SendAck(ack);
            return 0;
        } else {
            BLE_SendAck("INVALID_VALUE\n");
            return 1;
        }
    }

    // 6. 查询当前温湿度（保持不变）
    else if (strcmp((char*)cmd_str, "GET_DATA") == 0) {
        extern uint8_t dht11_humidity_int;
        extern uint8_t dht11_temperature_int;
        char data_buf[50];
        sprintf(data_buf, "T:%d H:%d\n", dht11_temperature_int, dht11_humidity_int);
        BLE_SendAck(data_buf);
        return 0;
    }
		
		    // 新增：查询湿度历史
    else if (strcmp((char*)cmd_str, "humid") == 0) {
        char ack[80];
        char *p = ack;
        p += sprintf(p, "HUMID_HIST: ");
        // 计算起始索引（最早的数据）
        uint8_t start = (g_history_count < HISTORY_SIZE) ? 0 : g_history_index;
        for (uint8_t i = 0; i < g_history_count; i++) {
            uint8_t idx = (start + i) % HISTORY_SIZE;
            p += sprintf(p, "%d", g_history[idx].humi);
            if (i < g_history_count - 1) {
                p += sprintf(p, ",");
            }
        }
        p += sprintf(p, "\n");
        BLE_SendAck(ack);
        return 0;
    }
    // 新增：查询温度历史
    else if (strcmp((char*)cmd_str, "temp") == 0) {
        char ack[80];
        char *p = ack;
        p += sprintf(p, "TEMP_HIST: ");
        uint8_t start = (g_history_count < HISTORY_SIZE) ? 0 : g_history_index;
        for (uint8_t i = 0; i < g_history_count; i++) {
            uint8_t idx = (start + i) % HISTORY_SIZE;
            p += sprintf(p, "%d", g_history[idx].temp);
            if (i < g_history_count - 1) {
                p += sprintf(p, ",");
            }
        }
        p += sprintf(p, "\n");
        BLE_SendAck(ack);
        return 0;
    }
		
		else if (strncmp((char*)cmd_str, "password:", 9) == 0) {
    // 如果已经登录，直接忽略密码命令
    if (Lockin_Mode == 1) {
        BLE_SendAck("ALREADY_LOGGED_IN\n");
        return 0;
    }
		
		if(Lockin_Mode ==3)
			ring(500,800);
		
    char *pwd = (char*)cmd_str + 9;
    uint8_t result = check_password(pwd);
    switch(result) {
        case 0: BLE_SendAck("PASSWORD_OK\n"); break;
        case 1: BLE_SendAck("PASSWORD_WRONG\n"); break;
        case 2: BLE_SendAck("PASSWORD_LOCKED\n"); break;
    }
    password[0] = '\0';  // 清空缓冲区
    return 0;
}
		
    // 7. 未知命令
    else {
        BLE_SendAck("UNKNOWN_CMD\n");
        return 1;
    }
}
