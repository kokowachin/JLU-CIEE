#ifndef __LCD_H
#define __LCD_H

// 解决u8/u16未知类型报错
#include <stdint.h>
typedef uint8_t  u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef volatile uint16_t vu16;

// 引入FSMC句柄
#include "fsmc.h"
extern SRAM_HandleTypeDef hsram1;
#define TFTSRAM_Handler hsram1

//////////////////////////////////////////////////////////////////////////////////
//ALIENTEK战舰STM32F103 TFT驱动 V4.1 适配HAL库
//////////////////////////////////////////////////////////////////////////////////
//LCD参数结构体
typedef struct
{
    u16 width;
    u16 height;
    u16 id;
    u16 detected_id;
    u8  dir;
    u16 wramcmd;
    u16 setxcmd;
    u16 setycmd;
} _lcd_dev;
extern _lcd_dev lcd;

//画笔、背景颜色
extern u32 POINT_COLOR;
extern u32 BACK_COLOR;

//背光引脚 PB0
#define LCD_LED_PIN GPIO_PIN_0
#define LCD_LED_PORT GPIOB
#define LCD_LED_ON()  HAL_GPIO_WritePin(LCD_LED_PORT, LCD_LED_PIN, GPIO_PIN_SET)
#define LCD_LED_OFF() HAL_GPIO_WritePin(LCD_LED_PORT, LCD_LED_PIN, GPIO_PIN_RESET)

//FSMC LCD地址定义 NE4 A10区分RS
typedef struct
{
    vu16 LCD_REG;
    vu16 LCD_RAM;
} LCD_TypeDef;
#define LCD_BASE ((u32)0x6C000000 | 0x000007FE)
#define LCD ((LCD_TypeDef *) LCD_BASE)

//扫描方向宏定义
#define L2R_U2D  0
#define L2R_D2U  1
#define R2L_U2D  2
#define R2L_D2U  3
#define U2D_L2R  4
#define U2D_R2L  5
#define D2U_L2R  6
#define D2U_R2L  7
#define DFT_SCAN_DIR  L2R_U2D

/* The connected panel is wider than a 240x320 ILI9341. Use ILI9488 when
   the LCD read bus is not connected and automatic ID detection fails. */
#define LCD_FALLBACK_ID 0x9488U

//RGB565标准颜色
#define WHITE     0xFFFF
#define BLACK     0x0000
#define BLUE      0x001F
#define BRED      0XF81F
#define GRED      0XFFE0
#define GBLUE     0X07FF
#define RED       0xF800
#define MAGENTA   0xF81F
#define GREEN     0x07E0
#define CYAN      0x7FFF
#define YELLOW    0xFFE0
#define BROWN     0XBC40
#define BRRED     0XFC07
#define GRAY      0X8430
#define DARKBLUE  0X01CF
#define LIGHTBLUE 0X7D7C
#define GRAYBLUE  0X5458
#define LIGHTGREEN 0X841F
#define LGRAY     0XC618
#define LGRAYBLUE 0XA651
#define LBBLUE    0X2B12

//SSD1963大屏分辨率参数
#define SSD_HOR_RESOLUTION      800
#define SSD_VER_RESOLUTION      480
#define SSD_HOR_PULSE_WIDTH     1
#define SSD_HOR_BACK_PORCH      46
#define SSD_HOR_FRONT_PORCH     210
#define SSD_VER_PULSE_WIDTH     1
#define SSD_VER_BACK_PORCH      23
#define SSD_VER_FRONT_PORCH     22
#define SSD_HT  (SSD_HOR_RESOLUTION+SSD_HOR_BACK_PORCH+SSD_HOR_FRONT_PORCH)
#define SSD_HPS (SSD_HOR_BACK_PORCH)
#define SSD_VT  (SSD_VER_RESOLUTION+SSD_VER_BACK_PORCH+SSD_VER_FRONT_PORCH)
#define SSD_VPS (SSD_VER_BACK_PORCH)

//函数声明
void LCD_Init(void);
void LCD_DisplayOn(void);
void LCD_DisplayOff(void);
void LCD_Clear(u16 Color);
void LCD_SetCursor(u16 Xpos, u16 Ypos);
void LCD_DrawPoint(u16 x, u16 y);
void LCD_Fast_DrawPoint(u16 x, u16 y, u16 color);
u16  LCD_ReadPoint(u16 x, u16 y);
void LCD_Draw_Circle(u16 x0, u16 y0, u8 r);
void LCD_DrawLine(u16 x1, u16 y1, u16 x2, u16 y2);
void LCD_DrawRectangle(u16 x1, u16 y1, u16 x2, u16 y2);
void LCD_Fill(u16 sx, u16 sy, u16 ex, u16 ey, u16 color);
void LCD_Color_Fill(u16 sx, u16 sy, u16 ex, u16 ey, u16 *color);
void LCD_ShowChar(u16 x, u16 y, u8 num, u8 size, u8 mode);
void LCD_ShowNum(u16 x, u16 y, u32 num, u8 len, u8 size);
void LCD_ShowxNum(u16 x, u16 y, u32 num, u8 len, u8 size, u8 mode);
void LCD_ShowString(u16 x, u16 y, u16 width, u16 height, u8 size, u8 *p);
void LCD_WriteReg(u16 LCD_Reg, u16 LCD_RegValue);
u16 LCD_ReadReg(u16 LCD_Reg);
void LCD_WriteRAM_Prepare(void);
void LCD_WriteRAM(u16 RGB_Code);
void LCD_SSD_BackLightSet(u8 pwm);
void LCD_Scan_Dir(u8 dir);
void LCD_Display_Dir(u8 dir);
void LCD_Set_Window(u16 sx, u16 sy, u16 width, u16 height);
u32 LCD_Pow(u8 m, u8 n);

//微秒延时
void delay_us(uint32_t nus);

#endif

