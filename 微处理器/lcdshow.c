#include "lcd.h"
#include "main.h"
#include "lcdshow.h"
#include "lock.h"
#include "centre_control.h"
#include "dht11.h"
#include "light.h"

#define PLOT_WIDTH  80  // 曲线图宽度
#define PLOT_HEIGHT 40  // 曲线图高度
static uint8_t temp_history[PLOT_WIDTH];

void Startup(void)
{
    static uint8_t last_mode = 0xFF;  // 记录上一次的模式
    
    // 如果模式没变，直接跳过，不刷新屏幕
    if (last_mode == Lockin_Mode) {
        return;
    }
    last_mode = Lockin_Mode;

    // 只有模式变化了，才执行下面的清屏和绘制
    LCD_Fill(10, 60, 230, 100, WHITE);
    LCD_Fill(10, 140, 230, 260, WHITE);
    
    switch(Lockin_Mode) {
        case 0:
            LCD_ShowString(10, 60, 200, 16, 16, (u8*)"Input_Your_Password_Please:");
            break;
        case 2:
            LCD_Clear(WHITE);
            LCD_ShowString(10, 60, 200, 16, 16, (u8*)"Wrong_Password,Input_Again");
            break;
        case 3:
            LCD_Clear(WHITE);
            LCD_ShowString(10, 60, 200, 16, 16, (u8*)"Many_mistakes,Wait_for_a_while!");
            break;
        default: break;
    }
}

// 在 lcdshow.c 中维护一个环形缓冲区（例如存20个温度点）
void Update_Trend_Graph(void) {
    static uint8_t index = 0;
    // 1. 移动数据（将新温度加入队列）
    temp_history[index++] = dht11_temperature_int;
    if(index >= PLOT_WIDTH) index = 0;

    // 2. 在屏幕右下角画一个小坐标系
    u16 x0 = 160, y0 = 180;
    // 清空绘图区域
    LCD_Fill(x0, y0 - PLOT_HEIGHT, x0 + PLOT_WIDTH, y0, WHITE);
    // 画坐标轴
    LCD_DrawLine(x0, y0, x0 + PLOT_WIDTH, y0); // X轴
    LCD_DrawLine(x0, y0 - PLOT_HEIGHT, x0, y0); // Y轴

    // 3. 画折线（遍历历史数据）
    for(int i=0; i<PLOT_WIDTH-1; i++) {
        int idx1 = (index + i) % PLOT_WIDTH;
        int idx2 = (index + i + 1) % PLOT_WIDTH;
        // 把温度映射为Y坐标（0-40度映射到40像素高度）
        u16 y1 = y0 - (temp_history[idx1] * PLOT_HEIGHT / 50); 
        u16 y2 = y0 - (temp_history[idx2] * PLOT_HEIGHT / 50);
        LCD_DrawLine(x0 + i, y1, x0 + i + 1, y2);
    }
}

// lcdshow.c
void Show_RunTime(void)
{
    static uint32_t last_second = 0;          // 上一次显示的秒数
    uint32_t current_second = HAL_GetTick() / 1000;

    // 只有当秒数变化时才更新
    if (current_second == last_second) return;
    last_second = current_second;

    // 计算时、分、秒
    uint32_t h = current_second / 3600;
    uint32_t m = (current_second % 3600) / 60;
    uint32_t s = current_second % 60;

    // 清除旧的显示区域（覆盖上一次的时间文本）
    // 位置 (10,10) 宽度 150 高度 20，可根据实际字符串长度微调
    LCD_Fill(10, 10, 150, 26, WHITE);

    // 显示 "Run: "
    LCD_ShowString(10, 10, 200, 16, 16, (u8*)"Run:");

    // 显示小时（2位，不足补零用 LCD_ShowxNum）
    LCD_ShowxNum(10 + 40, 10, h, 2, 16, 0x80); // 0x80 表示前置补零

    // 显示冒号
    LCD_ShowString(10 + 40 + 16*2, 10, 10,16, 16, (u8*)":");

    // 显示分钟
    LCD_ShowxNum(10 + 40 + 16*2 + 8, 10, m, 2, 16, 0x80);
    LCD_ShowString(10 + 40 + 16*2 + 8 + 16*2, 10,10, 16, 16, (u8*)":");

    // 显示秒
    LCD_ShowxNum(10 + 40 + 16*2 + 8 + 16*2 + 8, 10, s, 2, 16, 0x80);
}

void Showhumid(void){
	static uint32_t last_show_time = 0;
    uint32_t now = HAL_GetTick();
    
    // 限制刷新速度：至少间隔 300ms（如果数据没变，300ms 内绝不重绘）
    if (now - last_show_time < 300) {
        return;
    }
    last_show_time = now;
	// 清空显示区域
        // LCD_Clear(WHITE);

        // 只覆盖上一次的显示内容
        // 用白色矩形覆盖之前显示的区域，再重新绘制
        LCD_Fill(10, 60, 230, 100, WHITE);  // 清除旧数据
        // 显示湿度（整数部分 + 小数部分）
				POINT_COLOR = BLACK;
        LCD_ShowString(10, 60, 200, 16, 16, (u8*)"Humidity: ");
				POINT_COLOR = BLUE;
        LCD_ShowNum(10 + 90, 60, dht11_humidity_int, 2, 16);
        LCD_ShowString(10 + 90 + 16, 60, 200, 16, 16, (u8*)".");
        LCD_ShowNum(10 + 80 + 16*2 + 8, 60, dht11_humidity_dec, 1, 16);
        LCD_ShowString(10 + 80 + 16*2 + 8 + 8, 60, 200, 16, 16, (u8*)"%");
				//显示湿度阈值
		
				POINT_COLOR = BLACK;
				LCD_ShowString(10, 80, 200, 16, 16, (u8*)"warning: ");
				POINT_COLOR = BLUE;
				LCD_ShowNum(10 + 90, 80, g_sys_config.humi_threshold, 2, 16);
				LCD_ShowString(10 + 80 + 16*2 + 8, 80, 200, 16, 16, (u8*)"%");
				
				POINT_COLOR = BLACK;
				//显示水阀和警告
				LCD_ShowString(10, 100, 200, 16, 16, (u8*)"water_mode: ");
				if (watercan <= 10) {
						// 低水量警告
					POINT_COLOR = RED;
						LCD_ShowString(10+90, 100, 200, 16, 16, (u8*)"LOW WATER ");
						// 显示剩余容量百分比
					POINT_COLOR = BLUE;
						LCD_ShowNum(10+90+80, 100, watercan, 2, 16);
						LCD_ShowString(10+90+80+16, 100, 200, 16, 16, (u8*)"%");
				} else {
						// 正常显示模式
						if(g_sys_config.water_mode == 1)
								LCD_ShowString(10+90, 100, 200, 16, 16, (u8*)"auto          ");
						else if(g_sys_config.water_mode == 2)
								LCD_ShowString(10+90, 100, 200, 16, 16, (u8*)"on__          ");
						else
								LCD_ShowString(10+90, 100, 200, 16, 16, (u8*)"off_          ");
				}
				
				
				// 显示温度（整数 + 小数）
				POINT_COLOR = BLACK;
        LCD_ShowString(10, 140, 200, 16, 16, (u8*)"Temperature: ");
				POINT_COLOR = BLUE;
        LCD_ShowNum(10 + 90, 140, dht11_temperature_int, 2, 16);
        LCD_ShowString(10 + 90 + 16, 140, 200, 16, 16, (u8*)".");
        LCD_ShowNum(10 + 80 + 16*2 + 8, 140, dht11_temperature_dec, 1, 16);
        LCD_ShowString(10 + 80 + 16*2 + 8 + 8, 140, 200, 16, 16, (u8*)"C");
				//显示温度阈值
				POINT_COLOR = BLACK;
				LCD_ShowString(10, 160, 200, 16, 16, (u8*)"warning: ");
				POINT_COLOR = BLUE;
				LCD_ShowNum(10 + 90, 160, g_sys_config.temp_threshold, 2, 16);
				LCD_ShowString(10 + 80 + 16*2 + 8 + 8, 160, 200, 16, 16, (u8*)"C");
				
				//显示风扇状态
				POINT_COLOR = BLACK;
				LCD_ShowString(10, 180, 200, 16, 16, (u8*)"fan_mode: ");
				if(g_sys_config.fan_mode==0)
					LCD_ShowString(10+90, 180, 200, 16, 16, (u8*)"FAN__auto");
				else if(g_sys_config.fan_mode==1)
					LCD_ShowString(10+90, 180, 200, 16, 16, (u8*)"FAN__off ");
				else if(g_sys_config.fan_mode==2)
					LCD_ShowString(10+90, 180, 200, 16, 16, (u8*)"FAN__low_");
				else if(g_sys_config.fan_mode==3)
					LCD_ShowString(10+90, 180, 200, 16, 16, (u8*)"FAN__mid_");
				else if(g_sys_config.fan_mode==4)
					LCD_ShowString(10+90, 180, 200, 16, 16, (u8*)"FAN__high");
				
				LCD_ShowString(10, 220, 220, 16, 16, (u8*)"Water_capacity:");
				POINT_COLOR = BLUE;
				LCD_ShowNum(10 + 140, 220, watercan, 2, 16);
				LCD_ShowString(10 + 140 + 16*2 , 220, 220, 16, 16, (u8*)"%");
				
				//显示光照
				POINT_COLOR = BLACK;
				LCD_ShowString(10, 240, 200, 16, 16, (u8*)"LIGHT:");
				switch(voltage_light){
					case 1:
						LCD_ShowString(10+90, 240, 200, 16, 16, (u8*)"LOW_");
						break;
					case 2 :
						LCD_ShowString(10+90, 240, 200, 16, 16, (u8*)"MID_");
						break;
					default:
						LCD_ShowString(10+90, 240, 200, 16, 16, (u8*)"HIGH");
				}
	Update_Trend_Graph();
}

