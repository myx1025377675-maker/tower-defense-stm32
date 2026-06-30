/***
	*******************************************************************************************************
	*	@file  	main.c
	*	@version V1.0
	*  @date    2021-7-20
	*	@author  ??????	
	*	@brief   ??????????????QSPI FLASH	
   *******************************************************************************************************
   *  @description
	*
	*	???????????STM32H743IIT6????? ??????FK743M2-IIT6?? + ????800*480??????RGB???
	*	????????https://shop212360197.taobao.com
	*	QQ???????536665479
	*
>>>>> ?????????
	*
	*  1.??????4?????????QSPI Flash?????????????????1212/1616/2424/3232
	* 	2.???????????QSPI Flash????????????????SD????????????????????????????
	*	3.????????????????PCtoLCD2018
	*	4.????????????????lcd_fonts.h
	*
	*******************************************************************************************************
***/


#include "main.h"
#include "led.h"
#include "usart.h"
#include "sdram.h"  
#include "lcd_rgb.h"
#include "lcd_pwm.h"
#include "touch_800x480.h"
#include "qspi_w25q64.h"
#include "game_tasks.h"
#include "game_state.h"
#include "game_score.h"

/********************************************** ???????? *******************************************/

void SystemClock_Config(void);		// ???????
void MPU_Config(void);					// MPU????
	
/***************************************************************************************************
*	?? ?? ??: main
*	??????: ??
*	?? ?? ?: ??
*	????????: LTDC???????????
*	?    ??: ??
****************************************************************************************************/

int main(void)
{
	
	MPU_Config();				// MPU????
	SCB_EnableICache();		// ???ICache
	SCB_EnableDCache();		// ???DCache
	HAL_Init();					// ?????HAL??  
	SystemClock_Config();	// ?????????????480MHz
                                         
	LED_Init();					// ?????LED????
	USART1_Init();				// USART1?????	
	MX_FMC_Init();				// SDRAM?????

	MX_LTDC_Init();			// LTDC + layers
	Touch_Init();

	QSPI_W25Qxx_Init();

	/* Switch QSPI to memory-mapped mode for reliable font reads */
	QSPI_W25Qxx_MemoryMappedMode();
	GameScore_Init();

	LCD_PWMinit(80);

	GameState_Init();

	LCD_SetLayer(0);
	LCD_SetColor(COLOR_BLACK);
	LCD_SetBackColor(COLOR_BLACK);
	LCD_Clear();

	LCD_SetFont(&Font24);
	LCD_SetColor(COLOR_CYAN);
	{
		char *msg = "Tower Defense";
		LCD_DisplayString(240, 200, msg);
	}
	{
		char *msg2 = "Loading...";
		LCD_DisplayString(300, 250, msg2);
	}

	HAL_Delay(500);

	GameTasks_Start();

	while (1)
	{
		LED1_Toggle;
		HAL_Delay(500);
	}
}
/****************************************************************************************************/
/**
  * @brief  System Clock Configuration
  *         The system Clock is configured as follow : 
  *            System Clock source            = PLL (HSE)
  *            SYSCLK(Hz)                     = 480000000 (CPU Clock)
  *            HCLK(Hz)                       = 240000000 (AXI and AHBs Clock)
  *            AHB Prescaler                  = 2
  *            D1 APB3 Prescaler              = 2 (APB3 Clock  120MHz)
  *            D2 APB1 Prescaler              = 2 (APB1 Clock  120MHz)
  *            D2 APB2 Prescaler              = 2 (APB2 Clock  120MHz)
  *            D3 APB4 Prescaler              = 2 (APB4 Clock  120MHz)
  *            HSE Frequency(Hz)              = 25000000
  *            PLL_M                          = 5
  *            PLL_N                          = 192
  *            PLL_P                          = 2
  *            PLL_Q                          = 2
  *            PLL_R                          = 2
  *            VDD(V)                         = 3.3
  *            Flash Latency(WS)              = 4
  * @param  None
  * @retval None
  */
/****************************************************************************************************/  
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
  RCC_PeriphCLKInitTypeDef PeriphClkInitStruct = {0};

  /** Supply configuration update enable
  */
  HAL_PWREx_ConfigSupply(PWR_LDO_SUPPLY);
  /** Configure the main internal regulator output voltage
  */
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE0);

  while(!__HAL_PWR_GET_FLAG(PWR_FLAG_VOSRDY)) {}
  /** Macro to configure the PLL clock source
  */
  __HAL_RCC_PLL_PLLSOURCE_CONFIG(RCC_PLLSOURCE_HSE);
  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 5;
  RCC_OscInitStruct.PLL.PLLN = 192;
  RCC_OscInitStruct.PLL.PLLP = 2;
  RCC_OscInitStruct.PLL.PLLQ = 2;
  RCC_OscInitStruct.PLL.PLLR = 2;
  RCC_OscInitStruct.PLL.PLLRGE = RCC_PLL1VCIRANGE_2;
  RCC_OscInitStruct.PLL.PLLVCOSEL = RCC_PLL1VCOWIDE;
  RCC_OscInitStruct.PLL.PLLFRACN = 0;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2
                              |RCC_CLOCKTYPE_D3PCLK1|RCC_CLOCKTYPE_D1PCLK1;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.SYSCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB3CLKDivider = RCC_APB3_DIV2;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_APB1_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_APB2_DIV2;
  RCC_ClkInitStruct.APB4CLKDivider = RCC_APB4_DIV2;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_4) != HAL_OK)
  {
    Error_Handler();
  }
  
  /* ????LTDC?????????????33MHz???????????60????????????????????????? */
  /* LCD clock configuration */
  /* PLL3_VCO Input = HSE_VALUE/PLL3M = 1 Mhz */
  /* PLL3_VCO Output = PLL3_VCO Input * PLL3N = 330 Mhz */
  /* PLLLCDCLK = PLL3_VCO Output/PLL3R = 330/10 = 33 Mhz */
  /* LTDC clock frequency = PLLLCDCLK = 33 Mhz */  
   
      
  PeriphClkInitStruct.PLL3.PLL3M = 25;
  PeriphClkInitStruct.PLL3.PLL3N = 330;
  PeriphClkInitStruct.PLL3.PLL3P = 2;
  PeriphClkInitStruct.PLL3.PLL3Q = 2;
  PeriphClkInitStruct.PLL3.PLL3R = 10;
  PeriphClkInitStruct.PLL3.PLL3RGE = RCC_PLL3VCIRANGE_0;
  PeriphClkInitStruct.PLL3.PLL3VCOSEL = RCC_PLL3VCOMEDIUM;
  PeriphClkInitStruct.PLL3.PLL3FRACN = 0;
  
  PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_LTDC|RCC_PERIPHCLK_USART1|RCC_PERIPHCLK_FMC;               
  PeriphClkInitStruct.FmcClockSelection = RCC_FMCCLKSOURCE_D1HCLK;
  PeriphClkInitStruct.Usart16ClockSelection = RCC_USART16CLKSOURCE_D2PCLK2;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
}


//	????MPU
//
void MPU_Config(void)
{
	MPU_Region_InitTypeDef MPU_InitStruct;

	HAL_MPU_Disable();		// ????MPU

	MPU_InitStruct.Enable           = MPU_REGION_ENABLE;
	MPU_InitStruct.BaseAddress      = SDRAM_BANK_ADDR;
	MPU_InitStruct.Size             = MPU_REGION_SIZE_32MB;
	MPU_InitStruct.AccessPermission = MPU_REGION_FULL_ACCESS;
	MPU_InitStruct.IsBufferable     = MPU_ACCESS_NOT_BUFFERABLE;
	MPU_InitStruct.IsCacheable      = MPU_ACCESS_CACHEABLE;
	MPU_InitStruct.IsShareable      = MPU_ACCESS_NOT_SHAREABLE;
	MPU_InitStruct.Number           = MPU_REGION_NUMBER2;
	MPU_InitStruct.TypeExtField     = MPU_TEX_LEVEL0;
	MPU_InitStruct.SubRegionDisable = 0x00;
	MPU_InitStruct.DisableExec      = MPU_INSTRUCTION_ACCESS_ENABLE;

	HAL_MPU_ConfigRegion(&MPU_InitStruct);

	HAL_MPU_Enable(MPU_PRIVILEGED_DEFAULT);	// ???MPU
}

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */

  /* USER CODE END Error_Handler_Debug */
}


/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
