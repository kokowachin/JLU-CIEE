#include "dht11.h"
#include "tim.h"

/* ================================================================
 * DHT11 驱动 (HAL库版本, STM32F1)
 * - 微秒延时: TIM2 硬件定时器 (1MHz, 1µs/tick)
 * - 毫秒延时: HAL_Delay()
 * - GPIO方向切换: HAL_GPIO_Init()
 * - 所有等待循环均含超时保护，防止死锁
 * ================================================================ */

/* 全局变量定义（dht11.h 中声明为 extern） */
volatile uint8_t dht11_humidity_int    = 0;
volatile uint8_t dht11_humidity_dec    = 0;
volatile uint8_t dht11_temperature_int = 0;
volatile uint8_t dht11_temperature_dec = 0;

/* ---------------------------------------------------------------
 * 1. 配置GPIO为输出模式（推挽）
 * --------------------------------------------------------------- */
static void DH11_GPIO_Init_OUT(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    GPIO_InitStruct.Pin   = DHT11_PIN;
    GPIO_InitStruct.Mode  = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull  = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;

    HAL_GPIO_Init(DHT11_GPIO_PORT, &GPIO_InitStruct);
}

/* ---------------------------------------------------------------
 * 2. 配置GPIO为输入模式（浮空）
 * --------------------------------------------------------------- */
static void DH11_GPIO_Init_IN(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    GPIO_InitStruct.Pin   = DHT11_PIN;
    GPIO_InitStruct.Mode  = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull  = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;

    HAL_GPIO_Init(DHT11_GPIO_PORT, &GPIO_InitStruct);
}

/* ---------------------------------------------------------------
 * 3. 微秒延时函数 —— TIM2 (对应 dht11.h 的声明)
 *    依赖: MX_TIM2_Init()，Prescaler=71, 72MHz/72=1MHz=1µs/tick
 * --------------------------------------------------------------- */
void DHT11_Delay_us(uint16_t us)
{
    __HAL_TIM_SET_COUNTER(&htim2, 0);
    HAL_TIM_Base_Start(&htim2);
    while (__HAL_TIM_GET_COUNTER(&htim2) < us);
    HAL_TIM_Base_Stop(&htim2);
}

/* ---------------------------------------------------------------
 * 4. 主机发送起始信号 (拉低18ms + 拉高20~40us)
 * --------------------------------------------------------------- */
static void DHT11_Start(void)
{
    DH11_GPIO_Init_OUT();

    HAL_GPIO_WritePin(DHT11_GPIO_PORT, DHT11_PIN, GPIO_PIN_SET);
    DHT11_Delay_us(30);

    HAL_GPIO_WritePin(DHT11_GPIO_PORT, DHT11_PIN, GPIO_PIN_RESET);
    HAL_Delay(20);

    HAL_GPIO_WritePin(DHT11_GPIO_PORT, DHT11_PIN, GPIO_PIN_SET);
    DHT11_Delay_us(30);

    DH11_GPIO_Init_IN();
}

/* ---------------------------------------------------------------
 * 5. 读取一个字节（8位），带超时保护
 * --------------------------------------------------------------- */
static uint8_t DHT11_Rec_Byte(void)
{
    uint8_t i, data = 0;
    uint16_t timeout;

    for (i = 0; i < 8; i++)
    {
        /* 等待低电平结束 */
        timeout = 1000;
        while (HAL_GPIO_ReadPin(DHT11_GPIO_PORT, DHT11_PIN) == GPIO_PIN_RESET)
        {
            if (--timeout == 0) return 0;
        }

        /* 延时 30us 判断数据位是 0 还是 1 */
        DHT11_Delay_us(30);

        data <<= 1;
        if (HAL_GPIO_ReadPin(DHT11_GPIO_PORT, DHT11_PIN) == GPIO_PIN_SET)
        {
            data |= 1;
        }

        /* 等待高电平结束 */
        timeout = 1000;
        while (HAL_GPIO_ReadPin(DHT11_GPIO_PORT, DHT11_PIN) == GPIO_PIN_SET)
        {
            if (--timeout == 0) return 0;
        }
    }
    return data;
}

/* ---------------------------------------------------------------
 * 6. 完整读取一次 DHT11 数据（对应 dht11.h 的声明）
 *    返回: 0=成功, 1=超时/校验失败
 * --------------------------------------------------------------- */
uint8_t DHT11_ReadData(void)
{
    uint8_t rh, rl, th, tl, check;
    uint16_t retry;

    DHT11_Start();

    /* 等待传感器响应（拉低总线） */
    retry = 200;
    while (HAL_GPIO_ReadPin(DHT11_GPIO_PORT, DHT11_PIN) == GPIO_PIN_SET)
    {
        if (--retry == 0) return 1;
        DHT11_Delay_us(5);
    }

    /* 等待 80us 低电平结束 */
    retry = 200;
    while (HAL_GPIO_ReadPin(DHT11_GPIO_PORT, DHT11_PIN) == GPIO_PIN_RESET)
    {
        if (--retry == 0) return 1;
        DHT11_Delay_us(5);
    }

    /* 等待 80us 高电平结束（准备发送数据） */
    retry = 200;
    while (HAL_GPIO_ReadPin(DHT11_GPIO_PORT, DHT11_PIN) == GPIO_PIN_SET)
    {
        if (--retry == 0) return 1;
        DHT11_Delay_us(5);
    }

    /* 连续读取 5 字节: 湿度整数/小数, 温度整数/小数, 校验和 */
    rh    = DHT11_Rec_Byte();
    rl    = DHT11_Rec_Byte();
    th    = DHT11_Rec_Byte();
    tl    = DHT11_Rec_Byte();
    check = DHT11_Rec_Byte();

    /* 释放总线: 拉低 50us 后释放 */
    DH11_GPIO_Init_OUT();
    HAL_GPIO_WritePin(DHT11_GPIO_PORT, DHT11_PIN, GPIO_PIN_RESET);
    DHT11_Delay_us(55);
    HAL_GPIO_WritePin(DHT11_GPIO_PORT, DHT11_PIN, GPIO_PIN_SET);
    DH11_GPIO_Init_IN();

    /* 校验 */
    if ((rh + rl + th + tl) == check)
    {
        dht11_humidity_int    = rh;
        dht11_humidity_dec    = rl;
        dht11_temperature_int = th;
        dht11_temperature_dec = tl;
        return 0;
    }
    return 1;
}

/* ---------------------------------------------------------------
 * 7. 初始化（CubeMX 已初始化 GPIO + TIM2，此处可留空）
 * --------------------------------------------------------------- */
void DHT11_Init(void)
{
    /* MX_GPIO_Init() 和 MX_TIM2_Init() 已由 CubeMX 自动调用 */
}
