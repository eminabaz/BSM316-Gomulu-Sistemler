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
#include "tim.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

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
#define STORAGE_ADDR 0x0801FB00
uint16_t blink_count = 7;      // Sayı 4-7 arası olacak 
uint16_t current_blink = 0;    
uint8_t wait_seconds = 0;
uint16_t btn_timer = 0;        // Buton basılı tutma süresi için
uint8_t btn_lock = 0;          // Buton tek basım kilidi

typedef enum { STATE_BLINK_ON, STATE_BLINK_OFF, STATE_WAIT } SystemState;
SystemState current_state = STATE_BLINK_ON;

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
//void SystemClock_Config(void);
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
  //SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_TIM2_Init();
  /* USER CODE BEGIN 2 */
  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_1, GPIO_PIN_RESET);
  uint16_t saved_val = *(volatile uint16_t*)((uint32_t)STORAGE_ADDR);



  if (saved_val >= 4 && saved_val <= 7) {
    blink_count = saved_val; 
  } else {
    blink_count = 4;
  }

  HAL_TIM_Base_Start_IT(&htim2); 
  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_1, GPIO_PIN_RESET);
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    if (HAL_GPIO_ReadPin(buttton_GPIO_Port, buttton_Pin) == GPIO_PIN_RESET) 
    {
        uint32_t start_tick = HAL_GetTick(); 
        
       
        while(HAL_GPIO_ReadPin(buttton_GPIO_Port, buttton_Pin) == GPIO_PIN_RESET);
        
        uint32_t duration = HAL_GetTick() - start_tick; 

        if (duration >= 3000 && duration < 5000) 
        {
            blink_count = 4; 
            Flash_Write(blink_count); 
        }
        if (duration >= 5000) { 
         HAL_NVIC_SystemReset(); 
        }
        else if (duration > 50) 
        {
            blink_count++; 
            if (blink_count > 7) blink_count = 4; 
            Flash_Write(blink_count); 
        }
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

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_HSI;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */
void Flash_Write(uint16_t data) {
    HAL_FLASH_Unlock(); 
    FLASH_EraseInitTypeDef EraseInitStruct;
    uint32_t PageError;
    
    EraseInitStruct.TypeErase = FLASH_TYPEERASE_PAGES;
    EraseInitStruct.PageAddress = STORAGE_ADDR; 
    EraseInitStruct.NbPages = 1;
    
    // Sayfayi sil ve veriyi yaz 
    if (HAL_FLASHEx_Erase(&EraseInitStruct, &PageError) == HAL_OK) {
        HAL_FLASH_Program(FLASH_TYPEPROGRAM_HALFWORD, STORAGE_ADDR, data);
    }
    HAL_FLASH_Lock(); // flash tekrar kilitle 
}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
  
  if (htim->Instance == TIM2)
  {
    switch (current_state) 
    {
      case STATE_BLINK_ON:
        HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_RESET);
        current_state = STATE_BLINK_OFF;
        break;

      case STATE_BLINK_OFF:
        HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_SET);
        current_blink++; 

        if (current_blink >= blink_count) 
        {
          current_state = STATE_WAIT;
          wait_seconds = 0;
        } 
        else 
        {
          current_state = STATE_BLINK_ON;
        }
        break;

      case STATE_WAIT:
        wait_seconds++;
        if (wait_seconds >= 5) 
        {
          current_blink = 0; 
          current_state = STATE_BLINK_ON;
        }
        break;
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
