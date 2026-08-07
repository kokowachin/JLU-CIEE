/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "adc.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"
#include "fsmc.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "lcd.h"
#include "dht11.h"
#include "centre_control.h"
#include "light.h"
#include "lock.h"
#include "lcdshow.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
uint8_t g_rx_buffer;           // 接收中断用的单字节缓存
uint8_t g_ble_rx_data[64];     // 存储接收到的完整命令
uint8_t g_ble_rx_index = 0;    // 命令索引
volatile uint8_t g_ble_cmd_received = 0;
extern volatile uint8_t Lockin_Mode;

uint8_t key_pe2_pending = 0;
uint8_t key_pe2_level = 0;
uint32_t key_pe2_tick = 0;

uint8_t key_pe3_pending = 0;
uint8_t key_pe3_level = 0;
uint32_t key_pe3_tick = 0;

uint8_t key_pe4_pending = 0;
uint8_t key_pe4_level = 0;
uint32_t key_pe4_tick = 0;

uint32_t last_dht11_read = 0;   // 上次读取DHT11的时间
#define DHT11_READ_INTERVAL 2000  // 2秒读一次
volatile uint8_t isWrite = 0;
volatile uint16_t watercan = 50;//储水罐50%

static uint32_t buzzer_toggle_tick = 0;   // 上次切换时刻
static uint8_t  buzzer_is_on = 0;         // 当前蜂鸣器状态（1响0关）
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_FSMC_Init();
  MX_USART1_UART_Init();
  MX_TIM2_Init();
  MX_TIM3_Init();
  MX_TIM4_Init();
  MX_USART3_UART_Init();
  MX_ADC3_Init();
  /* USER CODE BEGIN 2 */
	HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_2);//PWM启动
	HAL_TIM_PWM_Start(&htim4, TIM_CHANNEL_3);
	HAL_TIM_Base_Start_IT(&htim4);
	HAL_UART_Receive_IT(&huart1, &g_rx_buffer, 1);//开启蓝牙接收中断

	LCD_Init();
	LCD_WR_REG(0x36);
	LCD_WR_DATA(0x08);

// 这是汉字注释
    POINT_COLOR = RED;      // ������ɫ����ɫ
    BACK_COLOR = WHITE;     // ������ɫ����ɫ
    
    // �������ð�ɫ��䣩
    LCD_Clear(WHITE);
    
    // "Hello World"
    LCD_ShowString(10, 10, 200, 32, 16, (u8*)"Hello World");
		
		HAL_ADCEx_Calibration_Start(&hadc3);
			HAL_ADC_Start(&hadc3);
			HAL_ADC_PollForConversion(&hadc3,HAL_MAX_DELAY);
			
			HAL_Delay(3000);
			
			uint32_t last_update = 0;
			uint8_t ret=1;
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
		char display_buf[30];   // 用于构造字符串
		lighttrasit();
    uint32_t now = HAL_GetTick();
    if (now - last_dht11_read >= DHT11_READ_INTERVAL) {
        last_dht11_read = now;
        ret = DHT11_ReadData();
        if (ret == 0) {   // 读取成功
        BLE_UpdateSensorData(dht11_temperature_int, dht11_humidity_int);
    }
    }
		
		/*
		蓝牙数据处理
		*/
		if (g_ble_cmd_received) {
        g_ble_cmd_received = 0;
        BLE_ProcessCommand(g_ble_rx_data);   // 蓝牙处理
				}
		
				/*
				外部中断部分
				if(PE2KEY){
					PE2KEY = 0;			
				}
				
				if(PE3KEY){
					PE3KEY = 0;					
					}
				*/		
				
if (key_pe2_pending) {
        if (HAL_GetTick() - key_pe2_tick > 20) {    // 20ms 消抖
            key_pe2_pending = 0;
            // 再次读取电平，与记录值比较
            if (HAL_GPIO_ReadPin(GPIOE, GPIO_PIN_2) == key_pe2_level) {
                if (key_pe2_level == GPIO_PIN_SET) {   // 按下有效（低电平）
                    // 水阀模式切换
														switch(g_sys_config.water_mode){
                        case 0: g_sys_config.water_mode = 1; break;
                        case 1: g_sys_config.water_mode = 2; break;
                        default: g_sys_config.water_mode = 0; break;
                    }
												}
                }
            }
        }
    

    if (key_pe3_pending) {
        if (HAL_GetTick() - key_pe3_tick > 20) {
            key_pe3_pending = 0;
            if (HAL_GPIO_ReadPin(GPIOE, GPIO_PIN_3) == key_pe3_level) {
                if (key_pe3_level == GPIO_PIN_SET) {
                    // 风扇模式切换
                    switch(g_sys_config.fan_mode){
                        case 0: g_sys_config.fan_mode = 1; break;
                        case 1: g_sys_config.fan_mode = 2; break;
                        case 2: g_sys_config.fan_mode = 3; break;
                        case 3: g_sys_config.fan_mode = 4; break;
                        default: g_sys_config.fan_mode = 0; break;
                    }
                }
            }
        }
    }				
		
		if (key_pe4_pending) {
        if (HAL_GetTick() - key_pe4_tick > 20) {
            key_pe4_pending = 0;
            if (HAL_GPIO_ReadPin(GPIOE, GPIO_PIN_4) == key_pe4_level) {
                if (key_pe4_level == GPIO_PIN_SET) {
                    // 风扇模式切换
                   watercan += 10;
                    }
                }
            }
        }
    		
		
		static uint32_t water_last_tick = 0;
if (HAL_GetTick() - water_last_tick >= 500) {
    water_last_tick = HAL_GetTick();
    // 若水闸实际打开且容量大于0，则减少容量
    if (g_control_state.water == WATER_OPEN) {
        if (watercan > 0) {
            watercan--;
        }
        // 容量不足10%时强制关闭
        if (watercan <= 10) {
            water(WATER_CLOSE);   
        }
    }
}

// 低水量报警蜂鸣器控制
if (watercan <= 10) {
    uint32_t now = HAL_GetTick();
    // 每 1000ms 切换一次状态（响1秒，停1秒）
    if (now - buzzer_toggle_tick >= 1000) {
        buzzer_toggle_tick = now;
        buzzer_is_on = !buzzer_is_on;
        if (buzzer_is_on) {
            __HAL_TIM_SET_COMPARE(&htim4, TIM_CHANNEL_3, 500);  // 鸣叫（占空比 500）
        } else {
            __HAL_TIM_SET_COMPARE(&htim4, TIM_CHANNEL_3, 0);     // 静音
        }
    }
} else {
    // 容量恢复 >10% 时，确保蜂鸣器关闭
    if (buzzer_is_on || __HAL_TIM_GET_COMPARE(&htim4, TIM_CHANNEL_3) != 0) {
        __HAL_TIM_SET_COMPARE(&htim4, TIM_CHANNEL_3, 0);
        buzzer_is_on = 0;
        buzzer_toggle_tick = HAL_GetTick();  // 重置计时，避免下次立即切换
    }
}

		/*
		屏幕刷新部分
		*/
	if (HAL_GetTick() - last_update >= 100){
		last_update = HAL_GetTick();
		
		Show_RunTime();
		LCD_ShowString(10, 280, 200, 16, 16, (u8*)"Copyright Rhodes Island");
		
		if((Lockin_Mode !=1)&&(isWrite == 0)){
			Startup();
			if (Lockin_Mode == 3) {
    if ((HAL_GetTick() - lock_start_time) >= 5000) {
        reset_lock_state();
    }
}
	}
		
    else if((Lockin_Mode ==1)&&isWrite)    // 读取成功
       Showhumid();  
		/*
    else
    {
        // 读取失败，显示错误信息
        LCD_Fill(10, 60, 230, 100, WHITE);
        LCD_ShowString(10, 80, 200, 16, 16, (u8*)"DHT11 Error!");
    }
		*/
	}
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
  RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_ADC;
  PeriphClkInit.AdcClockSelection = RCC_ADCPCLK2_DIV6;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */
//蓝牙的中断回调函数
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    if (huart->Instance == USART1) {
        g_ble_rx_data[g_ble_rx_index++] = g_rx_buffer;

        // 判断命令结束（换行符）
        if (g_rx_buffer == '\n' || g_rx_buffer == '\r') {
            g_ble_rx_data[g_ble_rx_index - 1] = '\0';  // 替换为字符串结束符
            g_ble_cmd_received = 1;  // 通知主循环
            g_ble_rx_index = 0;      // 复位索引
        }

        if (g_ble_rx_index >= sizeof(g_ble_rx_data)) {
            g_ble_rx_index = 0;      // 溢出保护
        }

        HAL_UART_Receive_IT(&huart1, &g_rx_buffer, 1);
				
    }
}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
    if (htim->Instance == TIM4) {
        auto_control();
    }
}

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
    if (GPIO_Pin == GPIO_PIN_2) {
        if(key_pe2_pending==0)
    {
        key_pe2_pending=1;
        key_pe2_level=HAL_GPIO_ReadPin(GPIOE,GPIO_PIN_2);
        key_pe2_tick=HAL_GetTick();
    }
    }
    if (GPIO_Pin == GPIO_PIN_3) {
			if(key_pe3_pending == 0)
			{   key_pe3_pending = 1;
					key_pe3_level = HAL_GPIO_ReadPin(GPIOE, GPIO_PIN_3);
					key_pe3_tick = HAL_GetTick();
			}
    }
		 if (GPIO_Pin == GPIO_PIN_4) {
			if(key_pe4_pending == 0)
			{   key_pe4_pending = 1;
					key_pe4_level = HAL_GPIO_ReadPin(GPIOE, GPIO_PIN_4);
					key_pe4_tick = HAL_GetTick();
			}
    }
}

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}
#ifdef USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
