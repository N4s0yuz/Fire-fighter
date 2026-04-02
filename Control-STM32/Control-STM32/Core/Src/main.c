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
  #include "stdio.h"
  #include "string.h"
  /* Private includes ----------------------------------------------------------*/
  /* USER CODE BEGIN Includes */

  /* USER CODE END Includes */

  /* Private typedef -----------------------------------------------------------*/
  /* USER CODE BEGIN PTD */

  /* USER CODE END PTD */

  /* Private define ------------------------------------------------------------*/
  /* USER CODE BEGIN PD */
  #define IN1_PORT GPIOB            // Cấu hình chân cho L298N
  #define IN1_PIN  GPIO_PIN_12
  #define IN2_PORT GPIOB
  #define IN2_PIN  GPIO_PIN_13
  #define IN3_PORT GPIOB
  #define IN3_PIN  GPIO_PIN_14
  #define IN4_PORT GPIOB
  #define IN4_PIN  GPIO_PIN_15

  #define RELAY_PORT GPIOA        // Cấu hình chân cho relay
  #define RELAY_PIN  GPIO_PIN_11

  #define TRIG_PORT GPIOB           // Cấu hình chân Trig chung cho cảm biến siêu âm
  #define TRIG_PIN  GPIO_PIN_5

  #define ECHO1_PORT GPIOB          // Chân echo đằng trước
  #define ECHO1_PIN  GPIO_PIN_8  

  #define ECHO2_PORT GPIOB         // Chân echo bên trái  
  #define ECHO2_PIN  GPIO_PIN_9  

  #define ECHO3_PORT GPIOB        // Chân echo bên phải 
  #define ECHO3_PIN  GPIO_PIN_10  

  #define ECHO4_PORT GPIOB      // Chân echo đằng sau
  #define ECHO4_PIN  GPIO_PIN_11
  /* USER CODE END PD */

  /* Private macro -------------------------------------------------------------*/
  /* USER CODE BEGIN PM */
  volatile uint16_t gia_tri_adc[5]; 
  float goc_lech = 0;
  float cuong_do = 0;

  uint16_t kc_truoc = 0;
  uint16_t kc_trai = 0;
  uint16_t kc_phai = 0;
  uint16_t kc_sau = 0;
  /* USER CODE END PM */

  /* Private variables ---------------------------------------------------------*/
  ADC_HandleTypeDef hadc1;
  DMA_HandleTypeDef hdma_adc1;

  TIM_HandleTypeDef htim2;
  TIM_HandleTypeDef htim3;
  TIM_HandleTypeDef htim4;

  UART_HandleTypeDef huart1;

  /* USER CODE BEGIN PV */

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
  void Di_chuyen(int speed_L, int speed_R);
  void Send_ESP32_Data(void);
  uint16_t HCSR04_Read_PB5(uint8_t id);
  void delay_us(uint16_t us);
  void D(uint16_t d_trai, uint16_t d_truoc, uint16_t d_phai);
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
  HAL_TIM_Base_Start(&htim4); 
  HAL_ADC_Start_DMA(&hadc1, (uint32_t*)gia_tri_adc, 5); 
  __HAL_AFIO_REMAP_SWJ_NOJTAG();
  HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_2); 
  HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_1); 
  HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_2); 
    /* USER CODE END 2 */

    /* Infinite loop */
    /* USER CODE BEGIN WHILE */
    while (1)
    {
      /* USER CODE END WHILE */
  do_lech();

      kc_truoc = HCSR04_Read_PB5(1); HAL_Delay(40);
      kc_trai  = HCSR04_Read_PB5(2); HAL_Delay(40);
      kc_phai  = HCSR04_Read_PB5(3); HAL_Delay(40);

      if(cuong_do > 200)
      {
        int servo_pwm = 1500 + (int)(goc_lech * 11);
        if (servo_pwm > 2500) servo_pwm = 2500;
        if (servo_pwm < 500)  servo_pwm = 500;
          __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_2, servo_pwm);
        if (khoang_cach_truoc < 25) 
          {
              Di_chuyen(0, 0); 
              HAL_GPIO_WritePin(RELAY_PORT, RELAY_PIN, GPIO_PIN_SET); 
          } 
          else 
          {
              HAL_GPIO_WritePin(RELAY_PORT, RELAY_PIN, GPIO_PIN_RESET);
              if (goc_lech > 15) {
                  Di_chuyen(600, 200); 
              } 
              else if (goc_lech < -15) 
              {
                  Di_chuyen(200, 600); 
              } 
              else 
              {
                  Di_chuyen(700, 700); 
              }
          }
      }
      else 
      {
        HAL_GPIO_WritePin(RELAY_PORT, RELAY_PIN, GPIO_PIN_RESET);
          __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_2, 1500);
          
          D(kc_trai, kc_truoc, kc_phai);
      }
      Send_ESP32_Data();
      HAL_Delay(20);
      }
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
    TIM_OC_InitTypeDef sConfigOC = {0};

    /* USER CODE BEGIN TIM3_Init 1 */

    /* USER CODE END TIM3_Init 1 */
    htim3.Instance = TIM3;
    htim3.Init.Prescaler = 71;
    htim3.Init.CounterMode = TIM_COUNTERMODE_UP;
    htim3.Init.Period = 999;
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
    if (HAL_TIM_PWM_Init(&htim3) != HAL_OK)
    {
      Error_Handler();
    }
    sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
    sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
    if (HAL_TIMEx_MasterConfigSynchronization(&htim3, &sMasterConfig) != HAL_OK)
    {
      Error_Handler();
    }
    sConfigOC.OCMode = TIM_OCMODE_PWM1;
    sConfigOC.Pulse = 0;
    sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
    sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
    if (HAL_TIM_PWM_ConfigChannel(&htim3, &sConfigOC, TIM_CHANNEL_1) != HAL_OK)
    {
      Error_Handler();
    }
    if (HAL_TIM_PWM_ConfigChannel(&htim3, &sConfigOC, TIM_CHANNEL_2) != HAL_OK)
    {
      Error_Handler();
    }
    /* USER CODE BEGIN TIM3_Init 2 */

    /* USER CODE END TIM3_Init 2 */
    HAL_TIM_MspPostInit(&htim3);

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

    /* USER CODE BEGIN TIM4_Init 1 */

    /* USER CODE END TIM4_Init 1 */
    htim4.Instance = TIM4;
    htim4.Init.Prescaler = 71;
    htim4.Init.CounterMode = TIM_COUNTERMODE_UP;
    htim4.Init.Period = 65535;
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
    sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
    sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
    if (HAL_TIMEx_MasterConfigSynchronization(&htim4, &sMasterConfig) != HAL_OK)
    {
      Error_Handler();
    }
    /* USER CODE BEGIN TIM4_Init 2 */

    /* USER CODE END TIM4_Init 2 */

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
      float trong_so[] = {-2, -1, 0, 1, 2}; 

      for (int i = 0; i < 5; i++) {
          float f = 4095 - (float)gia_tri_adc[i];
          if (f < 500) f = 0; 
          
          tong += f;
          tong_trong_so += f * trong_so[i];
      }
      cuong_do = tong;
      if (tong > 200) {
          goc_lech = (tong_trong_so / tong) * 45; 
      } else {
          goc_lech = 0;
      }
  }
  void Di_chuyen(int speed_L, int speed_R) 
  {
      if (speed_L >= 0) {
          HAL_GPIO_WritePin(IN1_PORT, IN1_PIN, GPIO_PIN_SET);
          HAL_GPIO_WritePin(IN2_PORT, IN2_PIN, GPIO_PIN_RESET);
          __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_1, speed_L);
      } else {
          HAL_GPIO_WritePin(IN1_PORT, IN1_PIN, GPIO_PIN_RESET);
          HAL_GPIO_WritePin(IN2_PORT, IN2_PIN, GPIO_PIN_SET);
          __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_1, -speed_L);
      }
      if (speed_R >= 0) {
          HAL_GPIO_WritePin(IN3_PORT, IN3_PIN, GPIO_PIN_SET);
          HAL_GPIO_WritePin(IN4_PORT, IN4_PIN, GPIO_PIN_RESET);
          __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_2, speed_R);
      } else {
          HAL_GPIO_WritePin(IN3_PORT, IN3_PIN, GPIO_PIN_RESET);
          HAL_GPIO_WritePin(IN4_PORT, IN4_PIN, GPIO_PIN_SET);
          __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_2, -speed_R);
      }
  }
  void Send_ESP32_Data(void) {
      char data[120];
      sprintf(data, "{\"C\":%.1f,\"G\":%.1f,\"F\":%d,\"L\":%d,\"R\":%d}\n", 
              cuong_do, goc_lech, kc_truoc, kc_trai, kc_phai);
      HAL_UART_Transmit(&huart1, (uint8_t*)data, strlen(data), 100);
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
      __HAL_TIM_SET_COUNTER(&htim4, 0);
      while (HAL_GPIO_ReadPin(GPIOB, echo_pin)) {
          if (__HAL_TIM_GET_COUNTER(&htim4) > 30000) break;
      }
      return (uint16_t)(__HAL_TIM_GET_COUNTER(&htim4) * 0.017);
  }
  void delay_us(uint16_t us) 
  {
      __HAL_TIM_SET_COUNTER(&htim4, 0);
      while (__HAL_TIM_GET_COUNTER(&htim4) < us);
  }


  float tinh_do_gan(uint16_t d) {
      if (d <= 20) return 1;
      if (d >= 40) return 0;
      return (40 - (float)d) / 20; 
  }

  float tinh_do_xa(uint16_t d) {
      return 1 - tinh_do_gan(d); 
  }

  void D(uint16_t d_trai, uint16_t d_truoc, uint16_t d_phai) {
      float L_gan = tinh_do_gan(d_trai);  
      float L_xa = tinh_do_xa(d_trai);
      float F_gan = tinh_do_gan(d_truoc); 
      float F_xa = tinh_do_xa(d_truoc);
      float R_gan = tinh_do_gan(d_phai);  
      float R_xa = tinh_do_xa(d_phai);

      float W1 = L_xa * F_xa * R_xa; // Trống trải -> Đi thẳng nhanh
      float W2 = L_gan * F_xa * R_xa; // Vướng trái -> Cua phải nhẹ
      float W3 = L_xa * F_xa * R_gan; // Vướng phải -> Cua trái nhẹ
      float W4 = L_xa * F_gan * R_xa; // Vật cản trước mặt -> Xoay tại chỗ rẽ hướng
      float W5 = L_gan * F_gan * R_xa; // Vướng trái + trước -> Xoay phải mạnh
      float W6 = L_xa * F_gan * R_gan; // Vướng phải + trước -> Xoay trái mạnh
      float W7 = L_gan * F_xa * R_gan; // Kẹt 2 bên (hẻm) -> Đi thẳng chậm
      float W8 = L_gan * F_gan * R_gan; // Kẹt cả 3 -> Lùi lại

      float L_out[] = {650, 650, 100,  550,  650, -650, 350, -550};
      float R_out[] = {650, 100, 650, -200, -650,  650, 350, -550};

      float tong_W = W1 + W2 + W3 + W4 + W5 + W6 + W7 + W8;
      if (tong_W == 0) tong_W = 1;

      float speed_L = (W1*L_out[0] + W2*L_out[1] + W3*L_out[2] + W4*L_out[3] + 
                      W5*L_out[4] + W6*L_out[5] + W7*L_out[6] + W8*L_out[7]) / tong_W;
      float speed_R = (W1*R_out[0] + W2*R_out[1] + W3*R_out[2] + W4*R_out[3] + 
                      W5*R_out[4] + W6*R_out[5] + W7*R_out[6] + W8*R_out[7]) / tong_W;

      Di_chuyen((int)speed_L, (int)speed_R);
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
