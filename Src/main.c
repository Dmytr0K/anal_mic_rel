/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2020 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
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

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "settings.h"
#include "mic_direction.h"
#include <math.h>
#include <stdbool.h>
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
enum Modes {
    SHIFT = 0,
    ANGLE,
    REAL_TIME,
    SAMPLE
};
/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
volatile uint16_t MIC_SAMPLES[SAMPLE_NUM][2];
volatile uint16_t CURRENT_SAMPLE = 0;
char MIC_STR[32];
uint8_t shift = 0;
float time = 0;
float angle = 0;
enum Modes mode = SHIFT;
bool change_mode = false;
bool sample_ready = false;
uint8_t MAX_MODE = 3;
int period_value = 2099;
uint16_t out_samples_counter = 0;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);

/* USER CODE BEGIN PFP */
void blink(uint8_t num);

void print_message(char *text);
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void) {
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
    MX_ADC1_Init();
    MX_USART2_UART_Init();
    MX_TIM10_Init();
    MX_TIM11_Init();
    /* USER CODE BEGIN 2 */

    /* USER CODE END 2 */

    /* Infinite loop */
    /* USER CODE BEGIN WHILE */
    while (1) {
        if (mode == SHIFT || mode == ANGLE) {
            print_message("SHIFT/ANGLE MODE");
            period_value = 2099;
            MX_TIM10_Init();
            HAL_TIM_Base_Start_IT(&htim10);
            while (1) {
                if (CURRENT_SAMPLE == SAMPLE_NUM) {
                    HAL_TIM_Base_Stop_IT(&htim10);
                    shift = findShift(MIC_SAMPLES);
                    if (mode == SHIFT) {
                        sprintf(MIC_STR, "%d\r\n", shift);
                    }
                    if (mode == ANGLE) {
                        time = (float) shift / 40000;
                        angle = asin(time * 343 / 0.18);
                        sprintf(MIC_STR, "%f\r\n", angle);
                    }
                    HAL_UART_Transmit(&huart2, MIC_STR, strlen(MIC_STR), 0xFFFF);
                    CURRENT_SAMPLE = 0;
                    HAL_TIM_Base_Start_IT(&htim10);
                }
                if (change_mode) {
                    change_mode = false;
                    break;
                }
            }
        } else if (mode == REAL_TIME) {
            print_message("REAL TIME MODE");
            period_value = 2099;
            MX_TIM10_Init();
            HAL_TIM_Base_Start_IT(&htim10);
            while (1) {
                if (CURRENT_SAMPLE) {
                    sprintf(MIC_STR, "%d,%d\r\n", MIC_SAMPLES[0][0], MIC_SAMPLES[0][1]);
                    HAL_UART_Transmit(&huart2, MIC_STR, strlen(MIC_STR), 0xFFFF);
                    CURRENT_SAMPLE = 0;
                }
                if (change_mode) {
                    change_mode = false;
                    break;
                }
            }
        } else if (mode == SAMPLE) {
            print_message("SAMPLE MODE");
            period_value = 16799;
//            period_value = 2099;
            MX_TIM10_Init();
start_rec:
            out_samples_counter = 0;
            HAL_TIM_Base_Start_IT(&htim11);
            HAL_TIM_Base_Start_IT(&htim10);
            while (!sample_ready) {}
            sample_ready = false;
            while (1) {
                if (CURRENT_SAMPLE && !sample_ready) {
                    sprintf(MIC_STR, "%d,%d\r\n", MIC_SAMPLES[0][0], MIC_SAMPLES[0][1]);
                    HAL_UART_Transmit(&huart2, MIC_STR, strlen(MIC_STR), 0xFFFF);
                    CURRENT_SAMPLE = 0;
                    out_samples_counter++;
                } else if (sample_ready) {
                    HAL_TIM_Base_Stop_IT(&htim10);
                    HAL_TIM_Base_Stop_IT(&htim11);
                    sprintf(MIC_STR, "Total: %d\r\n", out_samples_counter);
                    HAL_UART_Transmit(&huart2, MIC_STR, strlen(MIC_STR), 0xFFFF);
                    print_message("END");
                    HAL_Delay(3000);
                    goto start_rec;
                }
                if (change_mode) {
                    change_mode = false;
                    break;
                }
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
void SystemClock_Config(void) {
    RCC_OscInitTypeDef RCC_OscInitStruct = {0};
    RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

    /** Configure the main internal regulator output voltage
    */
    __HAL_RCC_PWR_CLK_ENABLE();
    __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE2);
    /** Initializes the CPU, AHB and APB busses clocks
    */
    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
    RCC_OscInitStruct.HSIState = RCC_HSI_ON;
    RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
    RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
    RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
    RCC_OscInitStruct.PLL.PLLM = 8;
    RCC_OscInitStruct.PLL.PLLN = 84;
    RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
    RCC_OscInitStruct.PLL.PLLQ = 4;
    if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK) {
        Error_Handler();
    }
    /** Initializes the CPU, AHB and APB busses clocks
    */
    RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK
                                  | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
    RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
    RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
    RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
    RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

    if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK) {
        Error_Handler();
    }
}

/* USER CODE BEGIN 4 */
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin) {
    if (GPIO_Pin == GPIO_PIN_13) {
        HAL_TIM_Base_Stop_IT(&htim10);
        HAL_TIM_Base_Stop_IT(&htim11);
        if (mode < MAX_MODE) {
            mode += 1;
        } else {
            mode = 0;
        }
        blink(mode + 1);
    }
    change_mode = true;
}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim) {
    if (htim->Instance == TIM11) {
        sample_ready = true;
    }
    if (htim->Instance == TIM10) {
        HAL_ADCEx_InjectedStart(&hadc1);
        MIC_SAMPLES[CURRENT_SAMPLE][0] = HAL_ADCEx_InjectedGetValue(&hadc1, ADC_INJECTED_RANK_1);
        MIC_SAMPLES[CURRENT_SAMPLE][1] = HAL_ADCEx_InjectedGetValue(&hadc1, ADC_INJECTED_RANK_2);
        CURRENT_SAMPLE++;
    }
}

void HAL_TIM_TIM11_Callback() {

}

void blink(uint8_t num) {
    for (int i = 0; i < num; i++) {
        HAL_GPIO_WritePin(GPIOA, GPIO_PIN_7, 1);
        HAL_Delay(200);
        HAL_GPIO_WritePin(GPIOA, GPIO_PIN_7, 0);
        HAL_Delay(200);
    }
}

void print_message(char *text) {
    sprintf(MIC_STR, "\r\n****************\r\n");
    HAL_UART_Transmit(&huart2, MIC_STR, strlen(MIC_STR), 1);
    sprintf(MIC_STR, "%s\r\n", text);
    HAL_UART_Transmit(&huart2, MIC_STR, strlen(MIC_STR), 1);
    sprintf(MIC_STR, "****************\r\n\n");
    HAL_UART_Transmit(&huart2, MIC_STR, strlen(MIC_STR), 1);
}
/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void) {
    /* USER CODE BEGIN Error_Handler_Debug */
    /* User can add his own implementation to report the HAL error return state */

    /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
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
     tex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
