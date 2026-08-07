#include "lcd.h"
#include "font.h"
#include "main.h"
#include <stdlib.h>
//全局变量
_lcd_dev lcd;
u32 POINT_COLOR = BLACK;
u32 BACK_COLOR  = WHITE;

//写寄存器
void LCD_WR_REG(vu16 regval)
{
    LCD->LCD_REG = regval;
}
//写GRAM数据
void LCD_WR_DATA(vu16 data)
{
    LCD->LCD_RAM = data;
}
//读数据
u16 LCD_RD_DATA(void)
{
    vu16 ram;
    ram = LCD->LCD_RAM;
    return ram;
}
//写寄存器封装
void LCD_WriteReg(u16 LCD_Reg, u16 LCD_RegValue)
{
    LCD->LCD_REG = LCD_Reg;
    LCD->LCD_RAM = LCD_RegValue;
}
//读寄存器
u16 LCD_ReadReg(u16 LCD_Reg)
{
    LCD_WR_REG(LCD_Reg);
    delay_us(5);
    return LCD_RD_DATA();
}
//准备写GRAM
void LCD_WriteRAM_Prepare(void)
{
    LCD->LCD_REG = lcd.wramcmd;
}
//写入颜色
void LCD_WriteRAM(u16 RGB_Code)
{
    LCD->LCD_RAM = RGB_Code;
}
//BGR转RGB（ILI9341专用）
u16 LCD_BGR2RGB(u16 c)
{
    u16 r, g, b, rgb;
    b = (c >> 0) & 0x1f;
    g = (c >> 5) & 0x3f;
    r = (c >> 11) & 0x1f;
    rgb = (b << 11) + (g << 5) + (r << 0);
    return rgb;
}
//简易延时
void opt_delay(u8 i)
{
    while(i--);
}
//读取像素点颜色
u16 LCD_ReadPoint(u16 x, u16 y)
{
    u16 r, g, b;
    if (x >= lcd.width || y >= lcd.height) return 0;
    LCD_SetCursor(x, y);
    if (lcd.id == 0X5510) LCD_WR_REG(0X2E00);
    else LCD_WR_REG(0X2E);
    r = LCD_RD_DATA();
    if (lcd.id == 0X1963) return r;
    r = LCD_RD_DATA();
    b = LCD_RD_DATA();
    g = r & 0XFF;
    g <<= 8;
    return (((r >> 11) << 11) | ((g >> 10) << 5) | (b >> 11));
}
//开启显示
void LCD_DisplayOn(void)
{
    if (lcd.id == 0X5510) LCD_WR_REG(0X2900);
    else LCD_WR_REG(0X29);
}
//关闭显示
void LCD_DisplayOff(void)
{
    if (lcd.id == 0X5510) LCD_WR_REG(0X2800);
    else LCD_WR_REG(0X28);
}
//设置坐标光标
void LCD_SetCursor(u16 Xpos, u16 Ypos)
{
    if (lcd.id == 0X1963)
    {
        if(lcd.dir == 0) Xpos = lcd.width - 1 - Xpos;
        LCD_WR_REG(lcd.setxcmd);
        LCD_WR_DATA(Xpos >> 8); LCD_WR_DATA(Xpos & 0XFF);
        LCD_WR_DATA((lcd.width - 1) >> 8); LCD_WR_DATA((lcd.width - 1) & 0XFF);
        LCD_WR_REG(lcd.setycmd);
        LCD_WR_DATA(Ypos >> 8); LCD_WR_DATA(Ypos & 0XFF);
        LCD_WR_DATA((lcd.height - 1) >> 8); LCD_WR_DATA((lcd.height - 1) & 0XFF);
    }
    else if (lcd.id == 0X5510)
    {
        LCD_WR_REG(lcd.setxcmd);
        LCD_WR_DATA(Xpos >> 8);
        LCD_WR_REG(lcd.setxcmd + 1); LCD_WR_DATA(Xpos & 0XFF);
        LCD_WR_REG(lcd.setycmd);
        LCD_WR_DATA(Ypos >> 8);
        LCD_WR_REG(lcd.setycmd + 1); LCD_WR_DATA(Ypos & 0XFF);
    }
    else
    {
        LCD_WR_REG(lcd.setxcmd);
        LCD_WR_DATA(Xpos >> 8); LCD_WR_DATA(Xpos & 0XFF);
        LCD_WR_REG(lcd.setycmd);
        LCD_WR_DATA(Ypos >> 8); LCD_WR_DATA(Ypos & 0XFF);
    }
}
//屏幕扫描方向配置
void LCD_Scan_Dir(u8 dir)
{
    u16 regval = 0, dirreg = 0, temp;
    if ((lcd.dir == 1 && lcd.id != 0X1963) || (lcd.dir == 0 && lcd.id == 0X1963))
    {
        switch(dir)
        {
            case 0: dir=6; break;
            case 1: dir=7; break;
            case 2: dir=4; break;
            case 3: dir=5; break;
            case 4: dir=1; break;
            case 5: dir=0; break;
            case 6: dir=3; break;
            case 7: dir=2; break;
        }
    }
    switch(dir)
    {
        case L2R_U2D: regval |= (0 << 7) | (0 << 6) | (0 << 5); break;
        case L2R_D2U: regval |= (1 << 7) | (0 << 6) | (0 << 5); break;
        case R2L_U2D: regval |= (0 << 7) | (1 << 6) | (0 << 5); break;
        case R2L_D2U: regval |= (1 << 7) | (1 << 6) | (0 << 5); break;
        case U2D_L2R: regval |= (0 << 7) | (0 << 6) | (1 << 5); break;
        case U2D_R2L: regval |= (0 << 7) | (1 << 6) | (1 << 5); break;
        case D2U_L2R: regval |= (1 << 7) | (0 << 6) | (1 << 5); break;
        case D2U_R2L: regval |= (1 << 7) | (1 << 6) | (1 << 5); break;
    }
    if(lcd.id == 0X5510) dirreg = 0X3600;
    else dirreg = 0X36;
    if(lcd.id == 0X9341 || lcd.id == 0X7789 ||
       lcd.id == 0X9486 || lcd.id == 0X9488)
    {
        regval |= 0X08;  /* BGR color order */
        regval ^= 0X40;  /* Correct the panel's horizontal mirror direction */
    }
    LCD_WriteReg(dirreg, regval);
    if(lcd.id != 0X1963)
    {
        if(regval & 0X20)
        {
            if(lcd.width < lcd.height){temp=lcd.width;lcd.width=lcd.height;lcd.height=temp;}
        }
        else
        {
            if(lcd.width > lcd.height){temp=lcd.width;lcd.width=lcd.height;lcd.height=temp;}
        }
    }
    if(lcd.id == 0X5510)
    {
        LCD_WR_REG(lcd.setxcmd);
        LCD_WR_DATA(0);LCD_WR_REG(lcd.setxcmd+1);LCD_WR_DATA(0);
        LCD_WR_REG(lcd.setxcmd+2);LCD_WR_DATA((lcd.width-1)>>8);LCD_WR_REG(lcd.setxcmd+3);LCD_WR_DATA((lcd.width-1)&0XFF);
        LCD_WR_REG(lcd.setycmd);
        LCD_WR_DATA(0);LCD_WR_REG(lcd.setycmd+1);LCD_WR_DATA(0);
        LCD_WR_REG(lcd.setycmd+2);LCD_WR_DATA((lcd.height-1)>>8);LCD_WR_REG(lcd.setycmd+3);LCD_WR_DATA((lcd.height-1)&0XFF);
    }
    else
    {
        LCD_WR_REG(lcd.setxcmd);
        LCD_WR_DATA(0);LCD_WR_DATA(0);
        LCD_WR_DATA((lcd.width-1)>>8);LCD_WR_DATA((lcd.width-1)&0XFF);
        LCD_WR_REG(lcd.setycmd);
        LCD_WR_DATA(0);LCD_WR_DATA(0);
        LCD_WR_DATA((lcd.height-1)>>8);LCD_WR_DATA((lcd.height-1)&0XFF);
    }
}
//画单个点
void LCD_DrawPoint(u16 x, u16 y)
{
    LCD_SetCursor(x, y);
    LCD_WriteRAM_Prepare();
    LCD->LCD_RAM = POINT_COLOR;
}
//快速画点
void LCD_Fast_DrawPoint(u16 x, u16 y, u16 color)
{
    if(lcd.id == 0X5510)
    {
        LCD_WR_REG(lcd.setxcmd);LCD_WR_DATA(x>>8);
        LCD_WR_REG(lcd.setxcmd+1);LCD_WR_DATA(x&0XFF);
        LCD_WR_REG(lcd.setycmd);LCD_WR_DATA(y>>8);
        LCD_WR_REG(lcd.setycmd+1);LCD_WR_DATA(y&0XFF);
    }
    else if(lcd.id == 0X1963)
    {
        if(lcd.dir == 0) x = lcd.width - 1 - x;
        LCD_WR_REG(lcd.setxcmd);
        LCD_WR_DATA(x>>8);LCD_WR_DATA(x&0XFF);
        LCD_WR_DATA(x>>8);LCD_WR_DATA(x&0XFF);
        LCD_WR_REG(lcd.setycmd);
        LCD_WR_DATA(y>>8);LCD_WR_DATA(y&0XFF);
        LCD_WR_DATA(y>>8);LCD_WR_DATA(y&0XFF);
    }
    else
    {
        LCD_WR_REG(lcd.setxcmd);
        LCD_WR_DATA(x>>8);LCD_WR_DATA(x&0XFF);
        LCD_WR_REG(lcd.setycmd);
        LCD_WR_DATA(y>>8);LCD_WR_DATA(y&0XFF);
    }
    LCD->LCD_REG = lcd.wramcmd;
    LCD->LCD_RAM = color;
}
//SSD1963背光设置
void LCD_SSD_BackLightSet(u8 pwm)
{
    LCD_WR_REG(0xBE);
    LCD_WR_DATA(0x05);
    LCD_WR_DATA(pwm * 2.55);
    LCD_WR_DATA(0x01);
    LCD_WR_DATA(0xFF);
    LCD_WR_DATA(0x00);
    LCD_WR_DATA(0x00);
}
//横竖屏切换
void LCD_Display_Dir(u8 dir)
{
    lcd.dir = dir;
    if(dir == 0)
    {
        lcd.width = 240;
        lcd.height = 320;
        if(lcd.id == 0x5510)
        {
            lcd.wramcmd = 0X2C00; lcd.setxcmd = 0X2A00; lcd.setycmd = 0X2B00;
            lcd.width = 480; lcd.height = 800;
        }
        else if(lcd.id == 0X1963)
        {
            lcd.wramcmd = 0X2C; lcd.setxcmd = 0X2B; lcd.setycmd = 0X2A;
            lcd.width = 480; lcd.height = 800;
        }
        else
        {
            lcd.wramcmd = 0X2C; lcd.setxcmd = 0X2A; lcd.setycmd = 0X2B;
        }
        if(lcd.id == 0X5310 || lcd.id == 0X9486 || lcd.id == 0X9488)
        {
            lcd.width=320;
            lcd.height=480;
        }
    }
    else
    {
        lcd.width = 320;
        lcd.height = 240;
        if(lcd.id == 0x5510)
        {
            lcd.wramcmd = 0X2C00; lcd.setxcmd = 0X2A00; lcd.setycmd = 0X2B00;
            lcd.width = 800; lcd.height = 480;
        }
        else if(lcd.id == 0X1963)
        {
            lcd.wramcmd = 0X2C; lcd.setxcmd = 0X2A; lcd.setycmd = 0X2B;
            lcd.width = 800; lcd.height = 480;
        }
        else
        {
            lcd.wramcmd = 0X2C; lcd.setxcmd = 0X2A; lcd.setycmd = 0X2B;
        }
        if(lcd.id == 0X5310 || lcd.id == 0X9486 || lcd.id == 0X9488)
        {
            lcd.width=480;
            lcd.height=320;
        }
    }
    LCD_Scan_Dir(DFT_SCAN_DIR);
}
//设置显示开窗
void LCD_Set_Window(u16 sx, u16 sy, u16 width, u16 height)
{
    u16 twidth = sx + width - 1, theight = sy + height - 1;
    if(lcd.id == 0X1963 && lcd.dir != 1)
    {
        sx = lcd.width - width - sx;
        height = sy + height - 1;
        LCD_WR_REG(lcd.setxcmd);
        LCD_WR_DATA(sx>>8);LCD_WR_DATA(sx&0XFF);
        LCD_WR_DATA(twidth>>8);LCD_WR_DATA(twidth&0XFF);
        LCD_WR_REG(lcd.setycmd);
        LCD_WR_DATA(sy>>8);LCD_WR_DATA(sy&0XFF);
        LCD_WR_DATA(height>>8);LCD_WR_DATA(height&0XFF);
    }
    else if(lcd.id == 0X5510)
    {
        LCD_WR_REG(lcd.setxcmd);LCD_WR_DATA(sx>>8);LCD_WR_REG(lcd.setxcmd+1);LCD_WR_DATA(sx&0XFF);
        LCD_WR_REG(lcd.setxcmd+2);LCD_WR_DATA(twidth>>8);LCD_WR_REG(lcd.setxcmd+3);LCD_WR_DATA(twidth&0XFF);
        LCD_WR_REG(lcd.setycmd);LCD_WR_DATA(sy>>8);LCD_WR_REG(lcd.setycmd+1);LCD_WR_DATA(sy&0XFF);
        LCD_WR_REG(lcd.setycmd+2);LCD_WR_DATA(theight>>8);LCD_WR_REG(lcd.setycmd+3);LCD_WR_DATA(theight&0XFF);
    }
    else
    {
        LCD_WR_REG(lcd.setxcmd);
        LCD_WR_DATA(sx>>8);LCD_WR_DATA(sx&0XFF);
        LCD_WR_DATA(twidth>>8);LCD_WR_DATA(twidth&0XFF);
        LCD_WR_REG(lcd.setycmd);
        LCD_WR_DATA(sy>>8);LCD_WR_DATA(sy&0XFF);
        LCD_WR_DATA(theight>>8);LCD_WR_DATA(theight&0XFF);
    }
}
//LCD初始化函数（HAL适配，自动使用CubeMX生成hsram4）
#if 0
void LCD_Init(void)
{
    GPIO_InitTypeDef GPIO_Initure;
    __HAL_RCC_GPIOB_CLK_ENABLE();
    //初始化背光PB0
    GPIO_Initure.Pin = LCD_LED_PIN;
    GPIO_Initure.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_Initure.Pull = GPIO_PULLUP;
    GPIO_Initure.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(LCD_LED_PORT, &GPIO_Initure);
    LCD_LED_OFF();
    HAL_Delay(50);
    //读取屏幕ID
    LCD_WR_REG(0XD3);
    lcd.id = LCD_RD_DATA(); lcd.id = LCD_RD_DATA(); lcd.id = LCD_RD_DATA(); lcd.id <<=8; lcd.id |= LCD_RD_DATA();
    if(lcd.id != 0X9341)
    {
        LCD_WR_REG(0X04);
        lcd.id = LCD_RD_DATA(); lcd.id = LCD_RD_DATA(); lcd.id = LCD_RD_DATA(); lcd.id <<=8; lcd.id |= LCD_RD_DATA();
        if(lcd.id == 0X8552) lcd.id = 0x7789;
        if(lcd.id != 0x7789)
        {
            LCD_WR_REG(0XD4);
            lcd.id = LCD_RD_DATA(); lcd.id = LCD_RD_DATA(); lcd.id = LCD_RD_DATA(); lcd.id <<=8; lcd.id |= LCD_RD_DATA();
            if(lcd.id != 0X5310)
            {
                LCD_WriteReg(0xF000,0x0055);
                LCD_WriteReg(0xF001,0x00AA);
                LCD_WriteReg(0xF002,0x0052);
                LCD_WriteReg(0xF003,0x0008);
                LCD_WriteReg(0xF004,0x0001);
                LCD_WR_REG(0xC500); lcd.id = LCD_RD_DATA(); lcd.id <<=8; LCD_WR_REG(0xC501); lcd.id |= LCD_RD_DATA();
                HAL_Delay(5);
                if(lcd.id != 0X5510)
                {
                    LCD_WR_REG(0XA1);
                    lcd.id = LCD_RD_DATA(); lcd.id = LCD_RD_DATA(); lcd.id <<=8; lcd.id |= LCD_RD_DATA();
                    if(lcd.id == 0X5761) lcd.id = 0X1963;
                }
            }
        }
    }
    // 9341初始化序列
    if(lcd.id == 0X9341)
    {
        LCD_WR_REG(0xCF);LCD_WR_DATA(0x00);LCD_WR_DATA(0xC1);LCD_WR_DATA(0X30);
        LCD_WR_REG(0xED);LCD_WR_DATA(0x64);LCD_WR_DATA(0x03);LCD_WR_DATA(0X12);LCD_WR_DATA(0X81);
        LCD_WR_REG(0xE8);LCD_WR_DATA(0x85);LCD_WR_DATA(0x10);LCD_WR_DATA(0x7A);
        LCD_WR_REG(0xCB);LCD_WR_DATA(0x39);LCD_WR_DATA(0x2C);LCD_WR_DATA(0x00);LCD_WR_DATA(0x34);LCD_WR_DATA(0x02);
        LCD_WR_REG(0xF7);LCD_WR_DATA(0x20);
        LCD_WR_REG(0xEA);LCD_WR_DATA(0x00);LCD_WR_DATA(0x00);
        LCD_WR_REG(0xC0);LCD_WR_DATA(0x1B);
        LCD_WR_REG(0xC1);LCD_WR_DATA(0x01);
        LCD_WR_REG(0xC5);LCD_WR_DATA(0x30);LCD_WR_DATA(0x30);
        LCD_WR_REG(0xC7);LCD_WR_DATA(0XB7);
        LCD_WR_REG(0x36);LCD_WR_DATA(0x48);
        LCD_WR_REG(0x3A);LCD_WR_DATA(0x55);
        LCD_WR_REG(0xB1);LCD_WR_DATA(0x00);LCD_WR_DATA(0x1A);
        LCD_WR_REG(0xB6);LCD_WR_DATA(0x0A);LCD_WR_DATA(0xA2);
        LCD_WR_REG(0xF2);LCD_WR_DATA(0x00);
        LCD_WR_REG(0x26);LCD_WR_DATA(0x01);
        LCD_WR_REG(0xE0);
        LCD_WR_DATA(0x0F);LCD_WR_DATA(0x2A);LCD_WR_DATA(0x28);LCD_WR_DATA(0x08);LCD_WR_DATA(0x0E);
        LCD_WR_DATA(0x08);LCD_WR_DATA(0x54);LCD_WR_DATA(0XA9);LCD_WR_DATA(0x43);
        LCD_WR_DATA(0x0A);LCD_WR_DATA(0x0F);LCD_WR_DATA(0x00);LCD_WR_DATA(0x00);LCD_WR_DATA(0x00);LCD_WR_DATA(0x00);
        LCD_WR_REG(0XE1);
        LCD_WR_DATA(0x00);LCD_WR_DATA(0x15);LCD_WR_DATA(0x17);LCD_WR_DATA(0x07);LCD_WR_DATA(0x11);
        LCD_WR_DATA(0x06);LCD_WR_DATA(0x2B);LCD_WR_DATA(0x56);LCD_WR_DATA(0x3C);
        LCD_WR_DATA(0x05);LCD_WR_DATA(0x10);LCD_WR_DATA(0x0F);LCD_WR_DATA(0x3F);LCD_WR_DATA(0x3F);LCD_WR_DATA(0x0F);
        LCD_WR_REG(0x2B);LCD_WR_DATA(0x00);LCD_WR_DATA(0x00);LCD_WR_DATA(0x01);LCD_WR_DATA(0x3f);
        LCD_WR_REG(0x2A);LCD_WR_DATA(0x00);LCD_WR_DATA(0x00);LCD_WR_DATA(0x00);LCD_WR_DATA(0xef);
        LCD_WR_REG(0x11);HAL_Delay(120);
        LCD_WR_REG(0x29);
    }
    LCD_Display_Dir(0);
    LCD_LED_ON();
    LCD_Clear(WHITE);
}
//清屏
#endif

static u16 LCD_DetectID(void)
{
    u16 id;

    LCD_WR_REG(0xD3);
    (void)LCD_RD_DATA();
    (void)LCD_RD_DATA();
    id = (LCD_RD_DATA() & 0xFFU) << 8;
    id |= LCD_RD_DATA() & 0xFFU;
    if (id == 0x9341U || id == 0x9486U || id == 0x9488U) return id;

    LCD_WR_REG(0x04);
    (void)LCD_RD_DATA();
    (void)LCD_RD_DATA();
    id = (LCD_RD_DATA() & 0xFFU) << 8;
    id |= LCD_RD_DATA() & 0xFFU;
    if (id == 0x8552U) return 0x7789U;
    if (id == 0x7789U) return id;

    return id;
}

static void LCD_Init_ILI9341(void)
{
    LCD_WR_REG(0x01);
    HAL_Delay(20);
    LCD_WR_REG(0xCF);LCD_WR_DATA(0x00);LCD_WR_DATA(0xC1);LCD_WR_DATA(0X30);
    LCD_WR_REG(0xED);LCD_WR_DATA(0x64);LCD_WR_DATA(0x03);LCD_WR_DATA(0X12);LCD_WR_DATA(0X81);
    LCD_WR_REG(0xE8);LCD_WR_DATA(0x85);LCD_WR_DATA(0x10);LCD_WR_DATA(0x7A);
    LCD_WR_REG(0xCB);LCD_WR_DATA(0x39);LCD_WR_DATA(0x2C);LCD_WR_DATA(0x00);LCD_WR_DATA(0x34);LCD_WR_DATA(0x02);
    LCD_WR_REG(0xF7);LCD_WR_DATA(0x20);
    LCD_WR_REG(0xEA);LCD_WR_DATA(0x00);LCD_WR_DATA(0x00);
    LCD_WR_REG(0xC0);LCD_WR_DATA(0x1B);
    LCD_WR_REG(0xC1);LCD_WR_DATA(0x01);
    LCD_WR_REG(0xC5);LCD_WR_DATA(0x30);LCD_WR_DATA(0x30);
    LCD_WR_REG(0xC7);LCD_WR_DATA(0xB7);
    LCD_WR_REG(0x36);LCD_WR_DATA(0x48);
    LCD_WR_REG(0x3A);LCD_WR_DATA(0x55);
    LCD_WR_REG(0xB1);LCD_WR_DATA(0x00);LCD_WR_DATA(0x1A);
    LCD_WR_REG(0xB6);LCD_WR_DATA(0x0A);LCD_WR_DATA(0xA2);
    LCD_WR_REG(0xF2);LCD_WR_DATA(0x00);
    LCD_WR_REG(0x26);LCD_WR_DATA(0x01);
    LCD_WR_REG(0xE0);
    LCD_WR_DATA(0x0F);LCD_WR_DATA(0x2A);LCD_WR_DATA(0x28);LCD_WR_DATA(0x08);LCD_WR_DATA(0x0E);
    LCD_WR_DATA(0x08);LCD_WR_DATA(0x54);LCD_WR_DATA(0xA9);LCD_WR_DATA(0x43);
    LCD_WR_DATA(0x0A);LCD_WR_DATA(0x0F);LCD_WR_DATA(0x00);LCD_WR_DATA(0x00);LCD_WR_DATA(0x00);LCD_WR_DATA(0x00);
    LCD_WR_REG(0xE1);
    LCD_WR_DATA(0x00);LCD_WR_DATA(0x15);LCD_WR_DATA(0x17);LCD_WR_DATA(0x07);LCD_WR_DATA(0x11);
    LCD_WR_DATA(0x06);LCD_WR_DATA(0x2B);LCD_WR_DATA(0x56);LCD_WR_DATA(0x3C);
    LCD_WR_DATA(0x05);LCD_WR_DATA(0x10);LCD_WR_DATA(0x0F);LCD_WR_DATA(0x3F);LCD_WR_DATA(0x3F);LCD_WR_DATA(0x0F);
    LCD_WR_REG(0x11);
    HAL_Delay(120);
    LCD_WR_REG(0x29);
    HAL_Delay(20);
}

static void LCD_Init_ST7789(void)
{
    LCD_WR_REG(0x01);
    HAL_Delay(120);
    LCD_WR_REG(0x11);
    HAL_Delay(120);
    LCD_WR_REG(0x3A);LCD_WR_DATA(0x55);
    LCD_WR_REG(0x36);LCD_WR_DATA(0x08);
    LCD_WR_REG(0xB2);LCD_WR_DATA(0x0C);LCD_WR_DATA(0x0C);LCD_WR_DATA(0x00);LCD_WR_DATA(0x33);LCD_WR_DATA(0x33);
    LCD_WR_REG(0xB7);LCD_WR_DATA(0x35);
    LCD_WR_REG(0xBB);LCD_WR_DATA(0x19);
    LCD_WR_REG(0xC0);LCD_WR_DATA(0x2C);
    LCD_WR_REG(0xC2);LCD_WR_DATA(0x01);
    LCD_WR_REG(0xC3);LCD_WR_DATA(0x12);
    LCD_WR_REG(0xC4);LCD_WR_DATA(0x20);
    LCD_WR_REG(0xC6);LCD_WR_DATA(0x0F);
    LCD_WR_REG(0xD0);LCD_WR_DATA(0xA4);LCD_WR_DATA(0xA1);
    LCD_WR_REG(0xE0);
    LCD_WR_DATA(0xD0);LCD_WR_DATA(0x04);LCD_WR_DATA(0x0D);LCD_WR_DATA(0x11);LCD_WR_DATA(0x13);LCD_WR_DATA(0x2B);LCD_WR_DATA(0x3F);
    LCD_WR_DATA(0x54);LCD_WR_DATA(0x4C);LCD_WR_DATA(0x18);LCD_WR_DATA(0x0D);LCD_WR_DATA(0x0B);LCD_WR_DATA(0x1F);LCD_WR_DATA(0x23);
    LCD_WR_REG(0xE1);
    LCD_WR_DATA(0xD0);LCD_WR_DATA(0x04);LCD_WR_DATA(0x0C);LCD_WR_DATA(0x11);LCD_WR_DATA(0x13);LCD_WR_DATA(0x2C);LCD_WR_DATA(0x3F);
    LCD_WR_DATA(0x44);LCD_WR_DATA(0x51);LCD_WR_DATA(0x2F);LCD_WR_DATA(0x1F);LCD_WR_DATA(0x1F);LCD_WR_DATA(0x20);LCD_WR_DATA(0x23);
    LCD_WR_REG(0x20);
    LCD_WR_REG(0x29);
    HAL_Delay(20);
}

static void LCD_Init_ILI948x(u16 id)
{
    LCD_WR_REG(0x01);
    HAL_Delay(120);

    if (id == 0x9486U)
    {
        LCD_WR_REG(0xF2);
        LCD_WR_DATA(0x1C);LCD_WR_DATA(0xA3);LCD_WR_DATA(0x32);LCD_WR_DATA(0x02);
        LCD_WR_DATA(0xB2);LCD_WR_DATA(0x12);LCD_WR_DATA(0xFF);LCD_WR_DATA(0x12);LCD_WR_DATA(0x00);
        LCD_WR_REG(0xF1);LCD_WR_DATA(0x36);LCD_WR_DATA(0xA4);
        LCD_WR_REG(0xF8);LCD_WR_DATA(0x21);LCD_WR_DATA(0x04);
        LCD_WR_REG(0xF9);LCD_WR_DATA(0x00);LCD_WR_DATA(0x08);
        LCD_WR_REG(0xC0);LCD_WR_DATA(0x0D);LCD_WR_DATA(0x0D);
        LCD_WR_REG(0xC1);LCD_WR_DATA(0x43);LCD_WR_DATA(0x00);
        LCD_WR_REG(0xC2);LCD_WR_DATA(0x00);
        LCD_WR_REG(0xC5);LCD_WR_DATA(0x00);LCD_WR_DATA(0x48);
        LCD_WR_REG(0xB4);LCD_WR_DATA(0x02);
        LCD_WR_REG(0xB6);LCD_WR_DATA(0x00);LCD_WR_DATA(0x22);LCD_WR_DATA(0x3B);
        LCD_WR_REG(0xE0);
        LCD_WR_DATA(0x0F);LCD_WR_DATA(0x24);LCD_WR_DATA(0x1C);LCD_WR_DATA(0x0A);LCD_WR_DATA(0x0F);
        LCD_WR_DATA(0x08);LCD_WR_DATA(0x43);LCD_WR_DATA(0x88);LCD_WR_DATA(0x32);LCD_WR_DATA(0x0F);
        LCD_WR_DATA(0x10);LCD_WR_DATA(0x06);LCD_WR_DATA(0x0F);LCD_WR_DATA(0x07);LCD_WR_DATA(0x00);
        LCD_WR_REG(0xE1);
        LCD_WR_DATA(0x0F);LCD_WR_DATA(0x38);LCD_WR_DATA(0x30);LCD_WR_DATA(0x09);LCD_WR_DATA(0x0F);
        LCD_WR_DATA(0x0F);LCD_WR_DATA(0x4E);LCD_WR_DATA(0x77);LCD_WR_DATA(0x3C);LCD_WR_DATA(0x07);
        LCD_WR_DATA(0x10);LCD_WR_DATA(0x05);LCD_WR_DATA(0x23);LCD_WR_DATA(0x1B);LCD_WR_DATA(0x00);
    }
    else
    {
        LCD_WR_REG(0xE0);
        LCD_WR_DATA(0x00);LCD_WR_DATA(0x03);LCD_WR_DATA(0x09);LCD_WR_DATA(0x08);LCD_WR_DATA(0x16);
        LCD_WR_DATA(0x0A);LCD_WR_DATA(0x3F);LCD_WR_DATA(0x78);LCD_WR_DATA(0x4C);LCD_WR_DATA(0x09);
        LCD_WR_DATA(0x0A);LCD_WR_DATA(0x08);LCD_WR_DATA(0x16);LCD_WR_DATA(0x1A);LCD_WR_DATA(0x0F);
        LCD_WR_REG(0xE1);
        LCD_WR_DATA(0x00);LCD_WR_DATA(0x16);LCD_WR_DATA(0x19);LCD_WR_DATA(0x03);LCD_WR_DATA(0x0F);
        LCD_WR_DATA(0x05);LCD_WR_DATA(0x32);LCD_WR_DATA(0x45);LCD_WR_DATA(0x46);LCD_WR_DATA(0x04);
        LCD_WR_DATA(0x0E);LCD_WR_DATA(0x0D);LCD_WR_DATA(0x35);LCD_WR_DATA(0x37);LCD_WR_DATA(0x0F);
        LCD_WR_REG(0xC0);LCD_WR_DATA(0x17);LCD_WR_DATA(0x15);
        LCD_WR_REG(0xC1);LCD_WR_DATA(0x41);
        LCD_WR_REG(0xC5);LCD_WR_DATA(0x00);LCD_WR_DATA(0x12);LCD_WR_DATA(0x80);
        LCD_WR_REG(0xB0);LCD_WR_DATA(0x00);
        LCD_WR_REG(0xB1);LCD_WR_DATA(0xA0);
        LCD_WR_REG(0xB4);LCD_WR_DATA(0x02);
        LCD_WR_REG(0xB6);LCD_WR_DATA(0x02);LCD_WR_DATA(0x02);
        LCD_WR_REG(0xE9);LCD_WR_DATA(0x00);
        LCD_WR_REG(0xF7);LCD_WR_DATA(0xA9);LCD_WR_DATA(0x51);LCD_WR_DATA(0x2C);LCD_WR_DATA(0x82);
    }

    LCD_WR_REG(0x3A);LCD_WR_DATA(0x55);
    LCD_WR_REG(0x11);
    HAL_Delay(120);
    LCD_WR_REG(0x29);
    HAL_Delay(20);
}

void LCD_Init(void)
{
    GPIO_InitTypeDef GPIO_Initure = {0};

    __HAL_RCC_GPIOB_CLK_ENABLE();
    GPIO_Initure.Pin = LCD_LED_PIN;
    GPIO_Initure.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_Initure.Pull = GPIO_NOPULL;
    GPIO_Initure.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(LCD_LED_PORT, &GPIO_Initure);
    LCD_LED_OFF();
    HAL_Delay(50);

    lcd.detected_id = LCD_DetectID();
    if (lcd.detected_id == 0x7789U)
    {
        lcd.id = 0x7789U;
        LCD_Init_ST7789();
    }
    else if (lcd.detected_id == 0x9486U || lcd.detected_id == 0x9488U)
    {
        lcd.id = lcd.detected_id;
        LCD_Init_ILI948x(lcd.id);
    }
    else if (lcd.detected_id == 0x9341U)
    {
        lcd.id = 0x9341U;
        LCD_Init_ILI9341();
    }
    else
    {
        /* Some 8080 LCD modules do not connect the read bus. */
        lcd.id = LCD_FALLBACK_ID;
        LCD_Init_ILI948x(lcd.id);
    }

    /* Use the RGB565 values as written; INVON complements every color bit. */
    LCD_WR_REG(0x20);
    LCD_Display_Dir(0);
    LCD_Clear(BLACK);
    LCD_LED_ON();
}

void LCD_Clear(u16 color)
{
    u32 total = lcd.width * lcd.height;
    LCD_SetCursor(0,0);
    LCD_WriteRAM_Prepare();
    for(u32 i=0;i<total;i++) LCD->LCD_RAM = color;
}
//矩形单色填充
void LCD_Fill(u16 sx,u16 sy,u16 ex,u16 ey,u16 color)
{
    u16 w = ex - sx + 1;
    for(u16 i=sy;i<=ey;i++)
    {
        LCD_SetCursor(sx,i);
        LCD_WriteRAM_Prepare();
        for(u16 j=0;j<w;j++) LCD->LCD_RAM = color;
    }
}
//彩色块填充
void LCD_Color_Fill(u16 sx,u16 sy,u16 ex,u16 ey,u16 *color)
{
    u16 w=ex-sx+1,h=ey-sy+1;
    for(u16 i=0;i<h;i++)
    {
        LCD_SetCursor(sx,sy+i);
        LCD_WriteRAM_Prepare();
        for(u16 j=0;j<w;j++) LCD->LCD_RAM = color[i*w+j];
    }
}
//画线
void LCD_DrawLine(u16 x1,u16 y1,u16 x2,u16 y2)
{
    int xerr=0,yerr=0,dx=x2-x1,dy=y2-y1,dist,incx,incy,uRow=x1,uCol=y1;
    incx = dx>0?1:(dx<0?-1:0);
    incy = dy>0?1:(dy<0?-1:0);
    dx=abs(dx); dy=abs(dist=dx>dy?dx:dy);
    for(u16 t=0;t<=dist+1;t++)
    {
        LCD_DrawPoint(uRow,uCol);
        xerr += dx; yerr += dy;
        if(xerr>dist){xerr-=dist;uRow+=incx;}
        if(yerr>dist){yerr-=dist;uCol+=incy;}
    }
}
//画圆
void LCD_Draw_Circle(u16 x0,u16 y0,u8 r)
{
    int a=0,b=r,di=3-(r<<1);
    while(a<=b)
    {
        LCD_DrawPoint(x0+a,y0-b); LCD_DrawPoint(x0+b,y0-a);
        LCD_DrawPoint(x0+b,y0+a); LCD_DrawPoint(x0+a,y0+b);
        LCD_DrawPoint(x0-a,y0+b); LCD_DrawPoint(x0-b,y0+a);
        LCD_DrawPoint(x0-a,y0-b); LCD_DrawPoint(x0-b,y0-a);
        a++;
        if(di<0) di += 4*a+6;
        else { di += 10 + 4*(a-b); b--; }
    }
}
//矩形边框
void LCD_DrawRectangle(u16 x1,u16 y1,u16 x2,u16 y2)
{
    LCD_DrawLine(x1,y1,x2,y1);
    LCD_DrawLine(x1,y1,x1,y2);
    LCD_DrawLine(x1,y2,x2,y2);
    LCD_DrawLine(x2,y1,x2,y2);
}
//字符显示
void LCD_ShowChar(u16 x,u16 y,u8 num,u8 size,u8 mode)
{
    u8 draw_width;
    u8 source_x, source_y;
    const unsigned char *glyph;

    if (size != 12 && size != 16 && size != 24) return;
    if (num >= 'a' && num <= 'z') num -= ('a' - 'A');
    if (num < ' ' || num > '~') num = '?';

    draw_width = size / 2;
    glyph = asc2_0808[num - ' '];
    for (u8 py = 0; py < size; py++)
    {
        source_y = (u8)(((u16)py * 8U) / size);
        for (u8 px = 0; px < draw_width; px++)
        {
            source_x = (u8)(((u16)px * 8U) / draw_width);
            if (glyph[source_y] & (0x80U >> source_x))
                LCD_Fast_DrawPoint(x + px, y + py, POINT_COLOR);
            else if (mode == 0)
                LCD_Fast_DrawPoint(x + px, y + py, BACK_COLOR);
        }
    }
}
//次方工具
u32 LCD_Pow(u8 m,u8 n)
{
    u32 res=1;
    while(n--) res *= m;
    return res;
}
//无前置0数字
void LCD_ShowNum(u16 x,u16 y,u32 num,u8 len,u8 size)
{
    u8 t,temp,show=0;
    for(t=0;t<len;t++)
    {
        temp = num / LCD_Pow(10,len-t-1) %10;
        if(show ==0 && temp ==0 && t < len-1)
        {
            LCD_ShowChar(x + size/2*t,y,' ',size,0);
            continue;
        }
        show = 1;
        LCD_ShowChar(x + size/2*t,y,temp+'0',size,0);
    }
}
//可填充前置0数字
void LCD_ShowxNum(u16 x,u16 y,u32 num,u8 len,u8 size,u8 mode)
{
    u8 t,temp,enshow=0;
    for(t=0;t<len;t++)
    {
        temp = num / LCD_Pow(10,len-t-1) %10;
        if(enshow ==0 && t < len-1 && temp ==0)
        {
            if(mode & 0x80) LCD_ShowChar(x+size/2*t,y,'0',size,mode&1);
            else LCD_ShowChar(x+size/2*t,y,' ',size,mode&1);
            continue;
        }
        enshow = 1;
        LCD_ShowChar(x+size/2*t,y,temp+'0',size,mode&1);
    }
}
//字符串显示
void LCD_ShowString(u16 x,u16 y,u16 w,u16 h,u8 size,u8 *p)
{
    u16 x0 = x;
    w +=x; h +=y;
    while(*p >= ' ' && *p <= '~')
    {
        if(x >=w) {x=x0; y+=size;}
        if(y >=h) break;
        LCD_ShowChar(x,y,*p,size,0);
        x += size/2; p++;
    }
}
//微秒延时
void delay_us(uint32_t nus)
{
    uint32_t fac = SystemCoreClock / 1000000U;
    uint32_t start = SysTick->VAL;
    uint32_t target = start - nus * fac;
    if(target > start)
        while(SysTick->VAL <= target);
    else
        while(SysTick->VAL > target);
}

