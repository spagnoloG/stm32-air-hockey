/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file           : main.c
 * @brief          : Main program body
 ******************************************************************************
 * @attention
 *
 * Copyright (c) 2021 STMicroelectronics.
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

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "board.h"
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

/* Definitions for defaultTask */
osThreadId_t gameTaskHandle;
osThreadId_t transferDataTaskHandle;

const osThreadAttr_t gameTask_attributes = { .name = "gameTask", .stack_size =
		128 * 4, .priority = (osPriority_t) osPriorityNormal, };

const osThreadAttr_t transferDataTask_attributes = { .name = "transferDataTask",
		.stack_size = 128 * 4, .priority = (osPriority_t) osPriorityNormal, };

/* USER CODE BEGIN PV */
UART_HandleTypeDef uart_structure;
uint8_t recv_buff[16];
osMessageQueueId_t event_driven_mq;
osMessageQueueId_t task_driven_mq;
RNG_HandleTypeDef rng;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
void StartGameTask(void *argument);
void StartTransferDataTask(void *argument);
void UART_SETUP(void);
void init_random_num_generator(void);
/* USER CODE BEGIN PFP */

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
	/* USER CODE BEGIN 2 */
	UART_SETUP();
    init_random_num_generator();
	init_display();
	draw_main_menu();
	/* USER CODE END 2 */

	/* Init scheduler */
	osKernelInitialize();

	/* USER CODE BEGIN RTOS_MUTEX */
	/* add mutexes, ... */
	/* USER CODE END RTOS_MUTEX */

	/* USER CODE BEGIN RTOS_SEMAPHORES */
	/* add semaphores, ... */
	/* USER CODE END RTOS_SEMAPHORES */

	/* USER CODE BEGIN RTOS_TIMERS */
	/* start timers, add new ones, ... */
	/* USER CODE END RTOS_TIMERS */

	/* USER CODE BEGIN RTOS_QUEUES */
	/* add queues, ... */
	event_driven_mq = osMessageQueueNew(MSG_QUEUE_SIZE, sizeof(Mssg), NULL);
	task_driven_mq = osMessageQueueNew(MSG_QUEUE_SIZE, sizeof(Mssg), NULL);
	/* USER CODE END RTOS_QUEUES */

	/* Create the thread(s) */
	/* creation of defaultTask */
	gameTaskHandle = osThreadNew(StartGameTask, NULL, &gameTask_attributes);

	transferDataTaskHandle = osThreadNew(StartTransferDataTask, NULL,
			&transferDataTask_attributes);

	/* USER CODE BEGIN RTOS_THREADS */
	/* add threads, ... */
	/* USER CODE END RTOS_THREADS */

	/* USER CODE BEGIN RTOS_EVENTS */
	/* add events, ... */
	/* USER CODE END RTOS_EVENTS */
	//HAL_UART_Receive_IT(&uart_structure, recv_buff, sizeof(recv_buff));

	 uint8_t stm_initialized_message[11] = "BOOTED:)))";
	 send_message_to_PC(stm_initialized_message, 11);

	/* Start scheduler */
	osKernelStart();

	/* We should never get here as control is now taken by the scheduler */
	/* Infinite loop */
	/* USER CODE BEGIN WHILE */
	while (1) {
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
	RCC_ClkInitTypeDef RCC_ClkInitStruct;
	RCC_OscInitTypeDef RCC_OscInitStruct;
	//RCC_PeriphCLKInitTypeDef PeriphClkInitStruct = {0};
	HAL_StatusTypeDef ret = HAL_OK;

	/* Enable Power Control clock */
	__HAL_RCC_PWR_CLK_ENABLE();

	/* The voltage scaling allows optimizing the power consumption when the device is
	 clocked below the maximum system frequency, to update the voltage scaling value
	 regarding system frequency refer to product datasheet.  */
	__HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

	/* Enable HSE Oscillator and activate PLL with HSE as source */
	RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
	RCC_OscInitStruct.HSEState = RCC_HSE_ON;
	RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
	RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
	RCC_OscInitStruct.PLL.PLLM = 25;
	RCC_OscInitStruct.PLL.PLLN = 432;
	RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
	RCC_OscInitStruct.PLL.PLLQ = 9;
	RCC_OscInitStruct.PLL.PLLR = 7;

	ret = HAL_RCC_OscConfig(&RCC_OscInitStruct);
	if (ret != HAL_OK) {
		while (1) {
			;
		}
	}

	/* Activate the OverDrive to reach the 216 MHz Frequency */
	ret = HAL_PWREx_EnableOverDrive();
	if (ret != HAL_OK) {
		while (1) {
			;
		}
	}

	/* Select PLL as system clock source and configure the HCLK, PCLK1 and PCLK2 clocks dividers */
	RCC_ClkInitStruct.ClockType = (RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_HCLK
			| RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2);
	RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
	RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
	RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
	RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

	ret = HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_7);
	if (ret != HAL_OK) {
		while (1) {
			;
		}
	}

//	PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_USART1;
//	PeriphClkInitStruct.Usart1ClockSelection = RCC_USART1CLKSOURCE_PCLK2;
//	if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct) != HAL_OK) {
//		Error_Handler();
//	}
}

/* USER CODE BEGIN 4 */

void UART_SETUP(void) {
    __HAL_RCC_USART1_CLK_ENABLE();
	uart_structure.Instance = USART1;
	uart_structure.Init.BaudRate = 115200;
	uart_structure.Init.WordLength = UART_WORDLENGTH_8B;
	uart_structure.Init.StopBits = UART_STOPBITS_1;
	uart_structure.Init.Parity = UART_PARITY_NONE;
	uart_structure.Init.Mode = UART_MODE_TX_RX;
	uart_structure.Init.HwFlowCtl = UART_HWCONTROL_NONE;
	uart_structure.Init.OverSampling = UART_OVERSAMPLING_16;
	uart_structure.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
	uart_structure.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
	if (HAL_UART_Init(&uart_structure) != HAL_OK) {
		Error_Handler();
	}

	__HAL_RCC_GPIOA_CLK_ENABLE();
	GPIO_InitTypeDef init_structure;
	init_structure.Pin = GPIO_PIN_9 | GPIO_PIN_10;
	init_structure.Pull = GPIO_NOPULL;
	init_structure.Speed = GPIO_SPEED_FREQ_LOW;
	init_structure.Mode = GPIO_MODE_AF_PP;
	init_structure.Alternate = GPIO_AF7_USART1;

	HAL_GPIO_Init(GPIOA, &init_structure);

	//__HAL_UART_ENABLE_IT(&uart_structure, UART_IT_RXNE);
	HAL_NVIC_SetPriority(USART1_IRQn, 2, 5);
	HAL_NVIC_EnableIRQ(USART1_IRQn);
}

void init_random_num_generator(void) {
    __HAL_RCC_RNG_CLK_ENABLE();
    rng.Instance = RNG;
    HAL_RNG_Init(&rng);
}


void StartGameTask(void *argument) {
	Mssg mssg;
	osStatus_t status;

	for (;;) {
		status = osMessageQueueGet(event_driven_mq, &mssg, NULL, 0U);
		if (status == osOK) {
            mssg.size=2;
		}
		BSP_TS_GetState(&TS_State);
		process_display_event(TS_State, &game);
		osDelay(2);
	}

}

void StartTransferDataTask(void *argument) {
	Mssg mssg;
	osStatus_t status;

	for (;;) {
		status = osMessageQueueGet(task_driven_mq, &mssg, NULL, 0U); // wait for message
		if (status == osOK) {
			HAL_UART_Transmit_IT(&uart_structure, mssg.contents, mssg.size);
		}
		osDelay(2);
	}
}

// Callback function, it is called when there is some message received from UART
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart) {
	//Mssg mssg;

	//HAL_UART_Transmit(&uart_structure, "accepted", 8, HAL_MAX_DELAY);
	//HAL_UART_Receive_IT(&uart_structure, recv_buff, sizeof(recv_buff));
//	// copy data
//	memcpy(mssg.contents, recv_buff, sizeof(recv_buff));
//	// put message into mssgQueue for game task to process
//	osMessageQueuePut(event_driven_mq, &mssg, 0U, 0U);
}


/* USER CODE END 4 */

/**
 * @brief  This function is executed in case of error occurrence.
 * @retval None
 */
void Error_Handler(void) {
	/* USER CODE BEGIN Error_Handler_Debug */
	/* User can add his own implementation to report the HAL error return state */
	__disable_irq();
	while (1) {
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

