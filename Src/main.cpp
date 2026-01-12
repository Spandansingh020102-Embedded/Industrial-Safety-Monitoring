
/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2025 STMicroelectronics.
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
extern "C" {
	#include "main.h"
	#include "gpio.h"
	#include "tim.h"
	#include "adc.h"
	#include "lcd.h"
	#include "4x4_keypad.h"
	#include "FreeRTOS.h"
	#include "task.h"
	#include <stdio.h>
	#include <string.h>

}

#include "LEDBarGraph.hpp"
#include "MotorDriver.hpp"
#include "LDRSensor.hpp"
#include "Potentiometer.hpp"
#include "Thermistor.hpp"

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

char key;

/*
 *  ------ Gas Sensor ------
 */
ADCDevice GasSensor(&hadc1);

/*
 * ------ Temperature Sensor ------
 */
ThermistorModule TempSensor(&hadc2, GPIOA, GPIO_PIN_4);

/*
 * ------ LED Bargraph ------
 */
GPIO_TypeDef* led_ports[10] = {
		led_0_GPIO_Port, led_1_GPIO_Port, led_2_GPIO_Port, led_3_GPIO_Port, led_4_GPIO_Port,
		led_5_GPIO_Port, led_6_GPIO_Port, led_7_GPIO_Port, led_8_GPIO_Port, led_9_GPIO_Port
};

uint16_t led_pins[10] = {
		led_0_Pin, led_1_Pin, led_2_Pin, led_3_Pin, led_4_Pin,
		led_5_Pin, led_6_Pin, led_7_Pin, led_8_Pin, led_9_Pin
};

LEDBarGraph led_bargraph(led_ports, led_pins);
/*
 *  ------ Fire Sensor ------
 */
LDRSensor FireSensor(fire_sensor_GPIO_Port, fire_sensor_Pin);

/*
 *  ------ Water Pump ------
 */
MotorDriver WaterPump(&htim3, TIM_CHANNEL_1, GPIOA, GPIO_PIN_1);

/*
 *  ------ Fan Motor ------
 */
MotorDriver FanMotor(&htim3, TIM_CHANNEL_2, GPIOA, GPIO_PIN_1);

/*
 *  ------ Smoke Sensor ------
 */
uint8_t isSmokeDetected = 0;

/*
 *  ------ Keypad ------
 */
Keypad_TypeDef keypad1 = {
    .row_ports = {GPIOA, GPIOA, GPIOA, GPIOA},
    .row_pins  = {GPIO_PIN_8, GPIO_PIN_9, GPIO_PIN_10, GPIO_PIN_11},
    .col_ports = {GPIOC, GPIOC, GPIOC, GPIOC},
    .col_pins  = {GPIO_PIN_4, GPIO_PIN_5, GPIO_PIN_6, GPIO_PIN_7}
};

// Global flags
volatile bool fire_detected = false;
volatile bool smoke_detected = false;
volatile bool gas_detected = false;

volatile float temperature = 0.0;
volatile float gas_percent = 0.0;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */
bool SmokeDetect(void);
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */


void SensorTask(void *params) {
    while (1) {
        fire_detected = FireSensor.isLightDetected();
        smoke_detected = SmokeDetect();
        gas_percent = GasSensor.readPercentage();

        if(gas_percent >= 99)
        {
        	gas_percent = 99;
        }

        if (gas_percent > 20) {
			gas_detected = true;
		}else
		{
			gas_detected = false;
		}
        temperature = TempSensor.getTemperatureCelsius();
        vTaskDelay(pdMS_TO_TICKS(200));
    }
}

void ActuatorTask(void *params) {
    while (1) {

    	led_bargraph.setLevel(gas_percent);
        if (fire_detected) {
            WaterPump.setSpeed(60);
            HAL_GPIO_TogglePin(red_warning_GPIO_Port, red_warning_Pin);

        } else {
            WaterPump.setSpeed(0);
            HAL_GPIO_WritePin(red_warning_GPIO_Port, red_warning_Pin, GPIO_PIN_RESET);
        }

        if (smoke_detected) {
            FanMotor.setSpeed(60);
            HAL_GPIO_TogglePin(yellow_warning_GPIO_Port, yellow_warning_Pin);
        } else {
            FanMotor.setSpeed(0);
            HAL_GPIO_WritePin(yellow_warning_GPIO_Port, yellow_warning_Pin, GPIO_PIN_RESET);
        }

        if(fire_detected || smoke_detected)
        {
        	HAL_GPIO_TogglePin(buzzer_GPIO_Port, buzzer_Pin);
        }
        else
        {
        	HAL_GPIO_WritePin(buzzer_GPIO_Port, buzzer_Pin, GPIO_PIN_RESET);
        }
        if (!fire_detected && !smoke_detected && !gas_detected) {
            HAL_GPIO_WritePin(green_safe_GPIO_Port, green_safe_Pin, GPIO_PIN_SET);
        } else {
        	HAL_GPIO_WritePin(green_safe_GPIO_Port, green_safe_Pin, GPIO_PIN_RESET);
        }

        vTaskDelay(pdMS_TO_TICKS(200));
    }
}

void DisplayTask(void *params) {
    char upper_line[17];
    char lower_line[17];

    while (1) {
        // Upper line status
        if (fire_detected && gas_detected && smoke_detected)
            snprintf(upper_line, 17, "F,G,S detected  ");
        else if (fire_detected && gas_detected)
            snprintf(upper_line, 17, "F,G detected    ");
        else if (fire_detected && smoke_detected)
            snprintf(upper_line, 17, "F,S detected    ");
        else if (gas_detected && smoke_detected)
            snprintf(upper_line, 17, "G,S detected    ");
        else if (fire_detected)
            snprintf(upper_line, 17, "F detected      ");
        else if (gas_detected)
            snprintf(upper_line, 17, "G detected      ");
        else if (smoke_detected)
            snprintf(upper_line, 17, "S detected      ");
        else
            snprintf(upper_line, 17, "Safe            ");

        snprintf(lower_line, 17, "T:%d G:%d", (int)temperature, (int)gas_percent);

        lprint(0x80, upper_line);
        lprint(0xC0, lower_line);

        vTaskDelay(pdMS_TO_TICKS(500));
    }
}

void PasswordTask(void *pv)
{
	char passkey[5] = {0};
	const char password[] = "2255";

	LcdFxn(0, 0x01); // Clear screen
	lprint(0x80, "Welcome");

	while (1) {
		char key = Keypad_Scan(&keypad1);
		if (key == '*') {
			LcdFxn(0, 0x01);
			lprint(0x80, "Enter Passkey:");
			memset(passkey, 0, sizeof(passkey));
			int i = 0;

			while (1) {
				key = Keypad_Scan(&keypad1);
				if (key) {
					if (key == '#') {
						passkey[i] = '\0';
						LcdFxn(0, 0x01);

						if (strcmp(passkey, password) == 0) {
							lprint(0x80, "Please come in");
							xTaskCreate(SensorTask, "sensor task", 128, NULL, 2, NULL);
							xTaskCreate(ActuatorTask, "actuator task", 128, NULL, 3, NULL);
							xTaskCreate(DisplayTask, "display task", 128, NULL, 1, NULL);
						} else {
							lprint(0x80, "Wrong");
						}

						vTaskDelay(pdMS_TO_TICKS(1000));
						LcdFxn(0, 0x01);
						lprint(0x80, "Welcome");
						break;
					}

					if (i < 4) {
						passkey[i++] = key;
						lprint(0xC0 + i - 1, "*");
					}
					while (Keypad_Scan(&keypad1)); // Wait for release
					vTaskDelay(pdMS_TO_TICKS(100)); // Debounce
				}

				vTaskDelay(pdMS_TO_TICKS(10));
			}
		}

		vTaskDelay(pdMS_TO_TICKS(50)); // Polling delay
	}
}

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
  MX_ADC1_Init();
  MX_TIM3_Init();
  MX_ADC2_Init();
  /* USER CODE BEGIN 2 */
	WaterPump.begin();
	FanMotor.begin();
	LcdInit();
	Keypad_Init(&keypad1);


	xTaskCreate(PasswordTask, "pwd", 128, NULL , 1, NULL);
	vTaskStartScheduler();
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {

	  key = Keypad_Scan(&keypad1);
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

  /** Configure the main internal regulator output voltage
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLM = 8;
  RCC_OscInitStruct.PLL.PLLN = 168;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 4;
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
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */

bool SmokeDetect(void)
{
	return HAL_GPIO_ReadPin(smoke_sensor_GPIO_Port, smoke_sensor_Pin);
}


/* USER CODE END 4 */

/**
  * @brief  Period elapsed callback in non blocking mode
  * @note   This function is called  when TIM1 interrupt took place, inside
  * HAL_TIM_IRQHandler(). It makes a direct call to HAL_IncTick() to increment
  * a global variable "uwTick" used as application time base.
  * @param  htim : TIM handle
  * @retval None
  */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
  /* USER CODE BEGIN Callback 0 */

  /* USER CODE END Callback 0 */
  if (htim->Instance == TIM1) {
    HAL_IncTick();
  }
  /* USER CODE BEGIN Callback 1 */

  /* USER CODE END Callback 1 */
}

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
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
