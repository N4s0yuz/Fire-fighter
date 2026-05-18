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
#include "string.h"
#include "stdio.h"
#include "math.h"
/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
  #define IN1_PORT GPIOB
  #define IN1_PIN  GPIO_PIN_12
  #define IN2_PORT GPIOB
  #define IN2_PIN  GPIO_PIN_13
  #define IN3_PORT GPIOB
  #define IN3_PIN  GPIO_PIN_14
  #define IN4_PORT GPIOB
  #define IN4_PIN  GPIO_PIN_15

  #define RELAY_PORT GPIOA
  #define RELAY_PIN  GPIO_PIN_11

  #define TRIG_PORT GPIOB
  #define TRIG_PIN  GPIO_PIN_5

  #define ECHO1_PORT GPIOB
  #define ECHO1_PIN  GPIO_PIN_11  

  #define ECHO2_PORT GPIOB
  #define ECHO2_PIN  GPIO_PIN_10  

  #define ECHO3_PORT GPIOB
  #define ECHO3_PIN  GPIO_PIN_9  

  #define ECHO4_PORT GPIOB
  #define ECHO4_PIN  GPIO_PIN_8
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */
  #define NGUONG_DAP_LUA   4000
  #define NGUONG_THEO_LUA  1500
  #define LUA_ADC_GIAM_KHI_SANG 1
  #define LUA_DELTA_MIN    150
  uint16_t adc_tinh[5] = {0};
  float lua_loc[5] = {0};
  float TRONG_SO[5] = {-2.0, -1.0, 0.0, 1.0, 2.0};
  uint16_t gia_tri_adc[5];
  float goc_lech = 0;
  float cuong_do = 0;

  uint16_t kc_truoc = 0;
  uint16_t kc_trai = 0;
  uint16_t kc_phai = 0;
  uint16_t kc_sau = 0;
  int pwm_motor_trai = 0;
  int pwm_motor_phai = 0;
/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
ADC_HandleTypeDef hadc1;
DMA_HandleTypeDef hdma_adc1;

TIM_HandleTypeDef htim2;
TIM_HandleTypeDef htim3;
TIM_HandleTypeDef htim4;

UART_HandleTypeDef huart1;

/* USER CODE BEGIN PV */
volatile char lenh_dieu_khien = '1';
uint32_t flame_sensor = 0;
uint32_t sieu_am = 0;
uint32_t gui_esp32 = 0;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_DMA_Init(void);
static void MX_ADC1_Init(void);
static void MX_TIM2_Init(void);
static void MX_TIM3_Init(void);
static void MX_TIM4_Init(void);
static void MX_USART1_UART_Init(void);
  /* USER CODE BEGIN PFP */
  void do_lech(void);
  void Cap_nhat_servo_lua(void);
  void Di_chuyen(int v_L, int v_R);
  void Send_ESP32_Data(void);
  uint16_t HCSR04_Read_PB5(uint8_t id);
  void delay_us(uint16_t us);
  void Thuat_toan_vat_can(uint16_t d_trai, uint16_t d_truoc, uint16_t d_phai, uint16_t d_sau);
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
  MX_DMA_Init();
  MX_ADC1_Init();
  MX_TIM2_Init();
  MX_TIM3_Init();
  MX_TIM4_Init();
  MX_USART1_UART_Init();
  /* USER CODE BEGIN 2 */
  HAL_TIM_Base_Start(&htim3); 
  HAL_ADC_Start_DMA(&hadc1, (uint32_t*)gia_tri_adc, 5); 
  __HAL_AFIO_REMAP_SWJ_NOJTAG();
  HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_2); 
  HAL_TIM_PWM_Start(&htim4, TIM_CHANNEL_1); 
  HAL_TIM_PWM_Start(&htim4, TIM_CHANNEL_2);

  HAL_UART_Receive_IT(&huart1, (uint8_t*)&lenh_dieu_khien, 1);
  /* USER CODE END 2 */
  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  uint32_t thoi_gian_phun_nuoc = 0;
  uint32_t thoi_gian_cho_phun_lai = 0;
  uint8_t dang_phun_nuoc = 0;
  HAL_Delay(500);
  uint32_t sum[5] = {0};
  for (int s = 0; s < 100; s++) {
      for (int i = 0; i < 5; i++)
          sum[i] += gia_tri_adc[i];
      HAL_Delay(100);
  }
  for (int i = 0; i < 5; i++)
      adc_tinh[i] = sum[i] / 100;
    while (1)
    {
    uint32_t hien_tai = HAL_GetTick();
    if(hien_tai-flame_sensor >=20){
     do_lech();
     flame_sensor=hien_tai;
    }
    if(hien_tai-sieu_am >= 40)
    {
      static uint8_t step=1;
      if (step==1) kc_truoc = HCSR04_Read_PB5(1);  
      else if (step==2) kc_trai = HCSR04_Read_PB5(2); 
      else if (step==3) kc_phai = HCSR04_Read_PB5(3);  
      else if (step==4) kc_sau = HCSR04_Read_PB5(4); 
      step++; if (step > 4) step = 1;
      sieu_am = hien_tai;
    }
    if(hien_tai-gui_esp32 >= 150){
      Send_ESP32_Data();
      gui_esp32 = hien_tai;
     }
    if (lenh_dieu_khien == '1' )
      {
      if (cuong_do > NGUONG_THEO_LUA)
          {
              Cap_nhat_servo_lua();
          }

      if (dang_phun_nuoc)
          {
              Di_chuyen(0, 0);
              if (hien_tai - thoi_gian_phun_nuoc >= 1500)
              {
                  HAL_GPIO_WritePin(RELAY_PORT, RELAY_PIN, GPIO_PIN_RESET);
                  dang_phun_nuoc = 0;
                  thoi_gian_phun_nuoc = 0;
                  thoi_gian_cho_phun_lai = hien_tai;
              }
          }
      else if (cuong_do > NGUONG_DAP_LUA &&
               hien_tai - thoi_gian_cho_phun_lai >= 700)
          {
              Di_chuyen(0, 0);
              HAL_GPIO_WritePin(RELAY_PORT, RELAY_PIN, GPIO_PIN_SET);
              thoi_gian_phun_nuoc = hien_tai;
              dang_phun_nuoc = 1;
          }
      else if (cuong_do > NGUONG_DAP_LUA)
          {
              HAL_GPIO_WritePin(RELAY_PORT, RELAY_PIN, GPIO_PIN_RESET);
              Di_chuyen(0, 0);
          }
      else if (cuong_do > NGUONG_THEO_LUA)
          {
            HAL_GPIO_WritePin(RELAY_PORT, RELAY_PIN, GPIO_PIN_RESET);

              int chenh_lech = (int)(goc_lech * 9.0f);
              if (chenh_lech > 320) chenh_lech = 320;
              if (chenh_lech < -320) chenh_lech = -320;

              int v_L = 580 + chenh_lech;
              int v_R = 580 - chenh_lech;
              if (v_L > 850) v_L = 850;
              if (v_L < 250) v_L = 250;
              if (v_R > 850) v_R = 850;
              if (v_R < 250) v_R = 250;
              Di_chuyen(v_L, v_R);

          }
          else 
          {

              HAL_GPIO_WritePin(RELAY_PORT, RELAY_PIN, GPIO_PIN_RESET);
              __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_2, 1500);

              Thuat_toan_vat_can(kc_trai, kc_truoc, kc_phai, kc_sau); 
          }
    }
    else
    {
      dang_phun_nuoc = 0;
      thoi_gian_phun_nuoc = 0;
      if (lenh_dieu_khien != 'P') HAL_GPIO_WritePin(RELAY_PORT, RELAY_PIN, GPIO_PIN_RESET);

      if (lenh_dieu_khien == 'F')
      {
          Di_chuyen(600, 600);
      }
      else if (lenh_dieu_khien == 'B') 
      {
          Di_chuyen(-600, -600);
      }
      else if (lenh_dieu_khien == 'L') 
      {
          Di_chuyen(-600, 600);
      }
      else if (lenh_dieu_khien == 'R') 
      {
          Di_chuyen(600, -600); 
      }
      else if (lenh_dieu_khien == 'P') 
      {
          Di_chuyen(0, 0);
          HAL_GPIO_WritePin(RELAY_PORT, RELAY_PIN, GPIO_PIN_SET);
      }
      else if (lenh_dieu_khien == 'Q') 
      {
          Di_chuyen(0, 0);
          uint32_t pwm = __HAL_TIM_GET_COMPARE(&htim2, TIM_CHANNEL_2);
          if (pwm <= 2450) __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_2, pwm + 50);
          lenh_dieu_khien = '0';
      }
      else if (lenh_dieu_khien == 'E') 
      {
          Di_chuyen(0, 0);
          uint32_t pwm = __HAL_TIM_GET_COMPARE(&htim2, TIM_CHANNEL_2);
          if (pwm >= 550) __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_2, pwm - 50);
          lenh_dieu_khien = '0';
      }
      else
      {
          Di_chuyen(0, 0);
          HAL_GPIO_WritePin(RELAY_PORT, RELAY_PIN, GPIO_PIN_RESET);
      }
    }
  }

    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
    }
  /* USER CODE END 3 */
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

/**
  * @brief ADC1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_ADC1_Init(void)
{

  /* USER CODE BEGIN ADC1_Init 0 */

  /* USER CODE END ADC1_Init 0 */

  ADC_ChannelConfTypeDef sConfig = {0};

  /* USER CODE BEGIN ADC1_Init 1 */

  /* USER CODE END ADC1_Init 1 */

  /** Common config
  */
  hadc1.Instance = ADC1;
  hadc1.Init.ScanConvMode = ADC_SCAN_ENABLE;
  hadc1.Init.ContinuousConvMode = ENABLE;
  hadc1.Init.DiscontinuousConvMode = DISABLE;
  hadc1.Init.ExternalTrigConv = ADC_SOFTWARE_START;
  hadc1.Init.DataAlign = ADC_DATAALIGN_RIGHT;
  hadc1.Init.NbrOfConversion = 5;
  if (HAL_ADC_Init(&hadc1) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Regular Channel
  */
  sConfig.Channel = ADC_CHANNEL_0;
  sConfig.Rank = ADC_REGULAR_RANK_1;
  sConfig.SamplingTime = ADC_SAMPLETIME_71CYCLES_5;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Regular Channel
  */
  sConfig.Channel = ADC_CHANNEL_2;
  sConfig.Rank = ADC_REGULAR_RANK_2;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Regular Channel
  */
  sConfig.Channel = ADC_CHANNEL_3;
  sConfig.Rank = ADC_REGULAR_RANK_3;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Regular Channel
  */
  sConfig.Channel = ADC_CHANNEL_4;
  sConfig.Rank = ADC_REGULAR_RANK_4;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Regular Channel
  */
  sConfig.Channel = ADC_CHANNEL_5;
  sConfig.Rank = ADC_REGULAR_RANK_5;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN ADC1_Init 2 */

  /* USER CODE END ADC1_Init 2 */

}

/**
  * @brief TIM2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM2_Init(void)
{

  /* USER CODE BEGIN TIM2_Init 0 */

  /* USER CODE END TIM2_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};
  TIM_OC_InitTypeDef sConfigOC = {0};

  /* USER CODE BEGIN TIM2_Init 1 */

  /* USER CODE END TIM2_Init 1 */
  htim2.Instance = TIM2;
  htim2.Init.Prescaler = 71;
  htim2.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim2.Init.Period = 19999;
  htim2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim2.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim2) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim2, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_TIM_PWM_Init(&htim2) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim2, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sConfigOC.OCMode = TIM_OCMODE_PWM1;
  sConfigOC.Pulse = 0;
  sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
  sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
  if (HAL_TIM_PWM_ConfigChannel(&htim2, &sConfigOC, TIM_CHANNEL_2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM2_Init 2 */

  /* USER CODE END TIM2_Init 2 */
  HAL_TIM_MspPostInit(&htim2);

}

/**
  * @brief TIM3 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM3_Init(void)
{

  /* USER CODE BEGIN TIM3_Init 0 */

  /* USER CODE END TIM3_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};

  /* USER CODE BEGIN TIM3_Init 1 */

  /* USER CODE END TIM3_Init 1 */
  htim3.Instance = TIM3;
  htim3.Init.Prescaler = 71;
  htim3.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim3.Init.Period = 65535;
  htim3.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim3.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim3) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim3, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim3, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM3_Init 2 */

  /* USER CODE END TIM3_Init 2 */

}

/**
  * @brief TIM4 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM4_Init(void)
{

  /* USER CODE BEGIN TIM4_Init 0 */

  /* USER CODE END TIM4_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};
  TIM_OC_InitTypeDef sConfigOC = {0};

  /* USER CODE BEGIN TIM4_Init 1 */

  /* USER CODE END TIM4_Init 1 */
  htim4.Instance = TIM4;
  htim4.Init.Prescaler = 71;
  htim4.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim4.Init.Period = 999;
  htim4.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim4.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim4) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim4, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_TIM_PWM_Init(&htim4) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim4, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sConfigOC.OCMode = TIM_OCMODE_PWM1;
  sConfigOC.Pulse = 0;
  sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
  sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
  if (HAL_TIM_PWM_ConfigChannel(&htim4, &sConfigOC, TIM_CHANNEL_1) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_TIM_PWM_ConfigChannel(&htim4, &sConfigOC, TIM_CHANNEL_2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM4_Init 2 */

  /* USER CODE END TIM4_Init 2 */
  HAL_TIM_MspPostInit(&htim4);

}

/**
  * @brief USART1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART1_UART_Init(void)
{

  /* USER CODE BEGIN USART1_Init 0 */

  /* USER CODE END USART1_Init 0 */

  /* USER CODE BEGIN USART1_Init 1 */

  /* USER CODE END USART1_Init 1 */
  huart1.Instance = USART1;
  huart1.Init.BaudRate = 115200;
  huart1.Init.WordLength = UART_WORDLENGTH_8B;
  huart1.Init.StopBits = UART_STOPBITS_1;
  huart1.Init.Parity = UART_PARITY_NONE;
  huart1.Init.Mode = UART_MODE_TX_RX;
  huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart1.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART1_Init 2 */

  /* USER CODE END USART1_Init 2 */

}

/**
  * Enable DMA controller clock
  */
static void MX_DMA_Init(void)
{

  /* DMA controller clock enable */
  __HAL_RCC_DMA1_CLK_ENABLE();

  /* DMA interrupt init */
  /* DMA1_Channel1_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA1_Channel1_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(DMA1_Channel1_IRQn);

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  /* USER CODE BEGIN MX_GPIO_Init_1 */

  /* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOD_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12|GPIO_PIN_13|GPIO_PIN_14|GPIO_PIN_15
                          |GPIO_PIN_5, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_11, GPIO_PIN_RESET);

  /*Configure GPIO pins : PB10 PB11 PB8 PB9 */
  GPIO_InitStruct.Pin = GPIO_PIN_10|GPIO_PIN_11|GPIO_PIN_8|GPIO_PIN_9;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pins : PB12 PB13 PB14 PB15
                           PB5 */
  GPIO_InitStruct.Pin = GPIO_PIN_12|GPIO_PIN_13|GPIO_PIN_14|GPIO_PIN_15
                          |GPIO_PIN_5;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pin : PA11 */
  GPIO_InitStruct.Pin = GPIO_PIN_11;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /* USER CODE BEGIN MX_GPIO_Init_2 */

  /* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */
  void do_lech(void) 
  {
      float tong = 0;
      float tong_trong_so = 0;

      for (int i = 0; i < 5; i++) {
        int32_t delta;

        if (LUA_ADC_GIAM_KHI_SANG) {
            delta = (int32_t)adc_tinh[i] - (int32_t)gia_tri_adc[i];
        } else {
            delta = (int32_t)gia_tri_adc[i] - (int32_t)adc_tinh[i];
        }

        if (delta < LUA_DELTA_MIN) delta = 0;
        lua_loc[i] = lua_loc[i] * 0.65f + (float)delta * 0.35f;

        if (lua_loc[i] > LUA_DELTA_MIN) {
            tong += lua_loc[i];
            tong_trong_so += lua_loc[i] * TRONG_SO[i];
        }
      }

      cuong_do = cuong_do * 0.7f + tong * 0.3f;
      if (tong > NGUONG_THEO_LUA) {
          float goc_moi = (tong_trong_so / tong) * 30.0f;
          goc_lech = goc_lech * 0.65f + goc_moi * 0.35f;
      } else {
          goc_lech *= 0.8f;
      }
      }

  void Cap_nhat_servo_lua(void)
  {
      int servo_pwm = 1500 + (int)(goc_lech * 11.0f);
      if (servo_pwm > 1850) servo_pwm = 1850;
      if (servo_pwm < 1150) servo_pwm = 1150;
      __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_2, servo_pwm);
  }
      
  void Di_chuyen(int v_L, int v_R) 
  {
      pwm_motor_trai = v_L;
      pwm_motor_phai = v_R;

      if (v_R >= 0) {
          HAL_GPIO_WritePin(IN1_PORT, IN1_PIN, GPIO_PIN_SET);
          HAL_GPIO_WritePin(IN2_PORT, IN2_PIN, GPIO_PIN_RESET);
          __HAL_TIM_SET_COMPARE(&htim4, TIM_CHANNEL_1, v_R);
      } else {
          HAL_GPIO_WritePin(IN1_PORT, IN1_PIN, GPIO_PIN_RESET);
          HAL_GPIO_WritePin(IN2_PORT, IN2_PIN, GPIO_PIN_SET);
          __HAL_TIM_SET_COMPARE(&htim4, TIM_CHANNEL_1, -v_R);
      }

      if (v_L >= 0) {
          HAL_GPIO_WritePin(IN3_PORT, IN3_PIN, GPIO_PIN_SET);
          HAL_GPIO_WritePin(IN4_PORT, IN4_PIN, GPIO_PIN_RESET);
          __HAL_TIM_SET_COMPARE(&htim4, TIM_CHANNEL_2, v_L);
      } else {
          HAL_GPIO_WritePin(IN3_PORT, IN3_PIN, GPIO_PIN_RESET);
          HAL_GPIO_WritePin(IN4_PORT, IN4_PIN, GPIO_PIN_SET);
          __HAL_TIM_SET_COMPARE(&htim4, TIM_CHANNEL_2, -v_L);
      }
  }
  void Send_ESP32_Data(void) {
      char data[160];
      sprintf(data, "{\"C\":%d,\"G\":%d,\"F\":%d,\"L\":%d,\"R\":%d,\"B\":%d}\n", 
              (int)cuong_do, (int)goc_lech, kc_truoc, kc_trai, kc_phai, kc_sau);
      HAL_UART_Transmit(&huart1, (uint8_t*)data, strlen(data), 100);
  }
  void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
  {
    if (huart->Instance == USART1)
    {
        HAL_UART_Receive_IT(&huart1, (uint8_t*)&lenh_dieu_khien, 1);
    }
  }
  uint16_t HCSR04_Read_PB5(uint8_t id) 
  {
      uint16_t echo_pin;
      if (id == 1) echo_pin = ECHO1_PIN;
      else if (id == 2) echo_pin = ECHO2_PIN;
      else if (id == 3) echo_pin = ECHO3_PIN;
      else echo_pin = ECHO4_PIN;

      HAL_GPIO_WritePin(TRIG_PORT, TRIG_PIN, GPIO_PIN_SET);
      delay_us(10);
      HAL_GPIO_WritePin(TRIG_PORT, TRIG_PIN, GPIO_PIN_RESET);

      uint32_t timeout = 0;
      while (!(HAL_GPIO_ReadPin(GPIOB, echo_pin))) {
          timeout++; if (timeout > 5000) return 999;
      }
      __HAL_TIM_SET_COUNTER(&htim3, 0);
      while (HAL_GPIO_ReadPin(GPIOB, echo_pin)) {
          if (__HAL_TIM_GET_COUNTER(&htim3) > 30000) break;
      }
      return (uint16_t)(__HAL_TIM_GET_COUNTER(&htim3) * 0.017);
  }
  void delay_us(uint16_t us) 
  {
      __HAL_TIM_SET_COUNTER(&htim3, 0);
      while (__HAL_TIM_GET_COUNTER(&htim3) < us);
  }


float gioi_han(float x, float min, float max) {
    if (x < min) return min;
    if (x > max) return max;
    return x;
}

uint16_t loc_khoang_cach(uint16_t d, uint16_t gia_tri_cu) {
    if (d == 0 || d > 250) return gia_tri_cu;
    return d;
}

float cap_nhat_khoang_cach_loc(float gia_tri_cu, uint16_t gia_tri_moi) {
    if ((float)gia_tri_moi < gia_tri_cu) {
        return gia_tri_cu * 0.25f + (float)gia_tri_moi * 0.75f;
    }
    return gia_tri_cu * 0.7f + (float)gia_tri_moi * 0.3f;
}

int gioi_han_toc_do(int hien_tai, int muc_tieu, int buoc) {
    if (muc_tieu > hien_tai + buoc) return hien_tai + buoc;
    if (muc_tieu < hien_tai - buoc) return hien_tai - buoc;
    return muc_tieu;
}

float tinh_do_gan_truoc(float d) {
    if (d <= 20.0f) return 1.0f;
    if (d >= 44.0f) return 0.0f;
    return (44.0f - d) / 24.0f;
}

float tinh_do_gan_ben(float d) {
    if (d <= 20.0f) return 1.0f;
    if (d >= 36.0f) return 0.0f;
    return (36.0f - d) / 16.0f;
}

float tinh_do_gan_sau(float d) {
    if (d <= 20.0f) return 1.0f;
    if (d >= 36.0f) return 0.0f;
    return (36.0f - d) / 16.0f;
}

void Thuat_toan_vat_can(uint16_t d_trai, uint16_t d_truoc, uint16_t d_phai, uint16_t d_sau)
{
    static float d_trai_loc  = 60.0f;
    static float d_truoc_loc = 60.0f;
    static float d_phai_loc  = 60.0f;
    static float d_sau_loc   = 60.0f;
    static int v_L_cu = 0;
    static int v_R_cu = 0;
    static int huong_ne = 1;

    d_trai  = loc_khoang_cach(d_trai,  (uint16_t)d_trai_loc);
    d_truoc = loc_khoang_cach(d_truoc, (uint16_t)d_truoc_loc);
    d_phai  = loc_khoang_cach(d_phai,  (uint16_t)d_phai_loc);
    d_sau   = loc_khoang_cach(d_sau,   (uint16_t)d_sau_loc);

    d_trai_loc  = cap_nhat_khoang_cach_loc(d_trai_loc,  d_trai);
    d_truoc_loc = cap_nhat_khoang_cach_loc(d_truoc_loc, d_truoc);
    d_phai_loc  = cap_nhat_khoang_cach_loc(d_phai_loc,  d_phai);
    d_sau_loc   = cap_nhat_khoang_cach_loc(d_sau_loc,   d_sau);

    // Chốt hướng né chống lắc
    float sai_so = d_phai_loc - d_trai_loc;
    if (d_truoc_loc >= 40.0f) 
    {
        if (fabsf(sai_so) > 8.0f) 
        {
            huong_ne = (sai_so > 0) ? 1 : -1;
        }
    }

    // --- XỬ LÝ LỆNH CỨNG (SÁT VẬT CẢN) ---
    
    if (d_truoc_loc <= 21.0f)
    {
        v_L_cu = -600; // Tăng lại lực lùi để thoát lẹ
        v_R_cu = -600;
        Di_chuyen(v_L_cu, v_R_cu);
        return;
    }

    if (d_truoc_loc <= 32.0f)
    {
        if (huong_ne < 0) { v_L_cu = -600; v_R_cu =  600; }
        else              { v_L_cu =  600; v_R_cu = -600; }
        Di_chuyen(v_L_cu, v_R_cu);
        return;
    }

    if (d_trai_loc <= 20.0f && d_phai_loc <= 20.0f)
    {
        v_L_cu = -550;
        v_R_cu = -550;
        Di_chuyen(v_L_cu, v_R_cu);
        return;
    }

    if (d_trai_loc <= 14.0f || d_phai_loc <= 14.0f)
    {
        // Vẫn giữ số dương để chạy tới nhưng tăng lực
        if (d_trai_loc < d_phai_loc) { v_L_cu = 650; v_R_cu = 250; }
        else                         { v_L_cu = 250; v_R_cu = 650; }
        Di_chuyen(v_L_cu, v_R_cu);
        return;
    }

    if (d_trai_loc <= 25.0f && d_phai_loc > d_trai_loc + 6.0f)
    {
        v_L_cu = 650;
        v_R_cu = 350;
        Di_chuyen(v_L_cu, v_R_cu);
        return;
    }

    if (d_phai_loc <= 25.0f && d_trai_loc > d_phai_loc + 6.0f)
    {
        v_L_cu = 350;
        v_R_cu = 650;
        Di_chuyen(v_L_cu, v_R_cu);
        return;
    }

    // --- FUZZY LOGIC (NE MƯỢT TỪ XA) ---
    
    float L_gan = tinh_do_gan_ben(d_trai_loc);
    float L_xa  = 1.0f - L_gan;
    float F_gan = tinh_do_gan_truoc(d_truoc_loc);
    float F_xa  = 1.0f - F_gan;
    float R_gan = tinh_do_gan_ben(d_phai_loc);
    float R_xa  = 1.0f - R_gan;
    float B_gan = tinh_do_gan_sau(d_sau_loc);
    float B_xa  = 1.0f - B_gan;

    float W1 = L_xa * F_xa * R_xa;
    float W2 = L_gan * F_xa * R_xa;
    float W3 = L_xa * F_xa * R_gan;
    float W4 = L_xa * F_gan * R_xa;
    float W5 = L_gan * F_gan * R_xa;
    float W6 = L_xa * F_gan * R_gan;
    float W7 = L_gan * F_xa * R_gan;
    float W8 = L_gan * F_gan * R_gan * B_xa;  
    float W9 = L_gan * F_gan * R_gan * B_gan; 

    // Đẩy PWM base lên 650, PWM rẽ tối thiểu là 250-280 để bánh đủ lực lăn
    float L_pwm[] = {650,  680,  280,  650,  680,  250,  500, -550,    0};
    float R_pwm[] = {650,  280,  680, -650,  250,  680,  500, -550,    0};

    if (huong_ne < 0) { // Né TRÁI
        L_pwm[3] =  250; R_pwm[3] =  680;
        L_pwm[7] = -400; R_pwm[7] = -550;
        L_pwm[8] = -550; R_pwm[8] =  550;
    } else {            // Né PHẢI
        L_pwm[3] =  680; R_pwm[3] =  250;
        L_pwm[7] = -550; R_pwm[7] = -400;
        L_pwm[8] =  550; R_pwm[8] = -550;
    }

    float tong_W = W1+W2+W3+W4+W5+W6+W7+W8+W9;
    if (tong_W == 0) tong_W = 1;

    float v_L = (W1*L_pwm[0] + W2*L_pwm[1] + W3*L_pwm[2] + W4*L_pwm[3] +
                 W5*L_pwm[4] + W6*L_pwm[5] + W7*L_pwm[6] + W8*L_pwm[7] +
                 W9*L_pwm[8]) / tong_W;
    float v_R = (W1*R_pwm[0] + W2*R_pwm[1] + W3*R_pwm[2] + W4*R_pwm[3] +
                 W5*R_pwm[4] + W6*R_pwm[5] + W7*R_pwm[6] + W8*R_pwm[7] +
                 W9*R_pwm[8]) / tong_W;

    // Giới hạn xả lại lên 680
    v_L = gioi_han(v_L, -680.0f, 680.0f);
    v_R = gioi_han(v_R, -680.0f, 680.0f);

    // Kéo bước gia tốc lên 150 để robot vọt nhanh hơn, không bị lề mề
    v_L_cu = gioi_han_toc_do(v_L_cu, (int)v_L, 150);
    v_R_cu = gioi_han_toc_do(v_R_cu, (int)v_R, 150);
    
    Di_chuyen(v_L_cu, v_R_cu);
}
/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
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
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
