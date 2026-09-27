
#include "app_threadx.h"
#include "main.h"
#include "ucpd.h"
#include "usb_otg.h"
#include "Gpio/gpio.h"
#include "usbpd.h"
#include "gpdma.h"
#include "systimer.h"

#include "ai_app.h"
#include "lcd_app.h"
#include "camera_app.h"
#include "ethernet.h"
#include "sd.h"

#include "stm32n6570_discovery_xspi.h"
#include "stm32n6570_discovery.h"

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MPU_Config(void);
static void SystemIsolation_Config(void);

/*
 * AI application is loaded and started by the FSBL.
 *
 * Flash programming:
 *   FSBL / Application images -> programmed to their configured addresses
 *   network_data.hex           -> must also be programmed for AI network weights/data
 *
 * network_data.hex can be generated with:
 *   python3.12 stm32ai_main.py using user_config.yml
 */

// Directly debugging needs clock and flash memory setup since fsbl is not used while directly debugging this project.
#define DEBUG_MODE		0

volatile uint32_t clock_freq = 0;

int main(void)
{

#if DEBUG_MODE == 1
	/* XSPI2 & XSPIM reset */
	// This was in system_stm32n6xx_fsbl.c. It was resetting memory mapped setup in fsbl so it needs to be commented when fsbl is used.
	RCC->AHB5RSTSR = RCC_AHB5RSTSR_XSPIMRSTS | RCC_AHB5RSTSR_XSPI2RSTS;
	RCC->AHB5RSTCR = RCC_AHB5RSTCR_XSPIMRSTC | RCC_AHB5RSTCR_XSPI2RSTC;
#endif

    HAL_PWREx_EnableVddA();
    HAL_PWREx_EnableVddIO2();
    HAL_PWREx_EnableVddIO3();
    HAL_PWREx_EnableVddIO4();
    HAL_PWREx_EnableVddIO5();

    MPU_Config();
    SCB_EnableICache();
    SCB_EnableDCache();

    HAL_Init();

#if DEBUG_MODE == 1
    // Use this to direct debug the code other case fsbl loader sets the clocks!!!
    SystemClock_Config();
#endif

    clock_freq 	= HAL_RCC_GetCpuClockFreq();
    clock_freq	= HAL_RCC_GetHCLKFreq();
    clock_freq  = HAL_RCC_GetPCLK1Freq();
    clock_freq  = HAL_RCC_GetPCLK2Freq();

    SystemIsolation_Config();

    // FSBL Does not init this part yet.
    BSP_XSPI_RAM_Init(0);
    BSP_XSPI_RAM_EnableMemoryMappedMode(0);

    //system_stm32n6xx_fsbl.c içindeki  XSPI2 & XSPIM reset kısmı fsbl memory mapped bozuyordu bununla ai thread çalışma sorunu çözüldü.

#if DEBUG_MODE == 1
    // Use this to direct debug the code other case fsbl loader sets it.
    BSP_XSPI_NOR_Init_t NOR_Init;
    NOR_Init.InterfaceMode 	= BSP_XSPI_NOR_OPI_MODE;
    NOR_Init.TransferRate 	= BSP_XSPI_NOR_DTR_TRANSFER;
    BSP_XSPI_NOR_Init(0, &NOR_Init);
    BSP_XSPI_NOR_EnableMemoryMappedMode(0);
#endif

    Timer_Init();
	MX_GPIO_Init();

    Ethernet_Init();
    SD_Init();

    MX_GPDMA1_Init();
    MX_UCPD1_Init();
    MX_USB1_OTG_HS_PCD_Init();

    AI_Init();
    Camera_Init();
    LCD_Init();
    Camera_Start();
    USBPD_PreInitOs();

	for(int i = 0; i < 10; i++){
		HAL_GPIO_TogglePin(LED_RED_GPIO_Port, LED_RED_Pin);
		HAL_Delay(25);
	}

    MX_ThreadX_Init();

    while (1){}
}

void SystemClock_Config(void)
{
    RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
    RCC_OscInitTypeDef RCC_OscInitStruct = {0};
    RCC_PeriphCLKInitTypeDef RCC_PeriphCLKInitStruct = {0};

    /*
     * FSBL'de PLL1 aktif ve CPU/SYS clock tarafından kullanılıyor.
     * PLL'leri yeniden configure etmeden önce CPU ve SYSCLK'i
     * geçici olarak HSI'ya geçir.
     */
    HAL_RCC_GetClockConfig(&RCC_ClkInitStruct);

    if ((RCC_ClkInitStruct.CPUCLKSource == RCC_CPUCLKSOURCE_IC1) ||
        (RCC_ClkInitStruct.SYSCLKSource == RCC_SYSCLKSOURCE_IC2_IC6_IC11))
    {
        RCC_ClkInitStruct.ClockType =
            RCC_CLOCKTYPE_CPUCLK |
            RCC_CLOCKTYPE_SYSCLK;

        RCC_ClkInitStruct.CPUCLKSource = RCC_CPUCLKSOURCE_HSI;
        RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_HSI;

        if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct) != HAL_OK)
        {
            Error_Handler();
        }
    }

    /*
     * AI application's original oscillator/PLL configuration
     */
    RCC_OscInitStruct.OscillatorType =
        RCC_OSCILLATORTYPE_HSI |
        RCC_OSCILLATORTYPE_HSE;

    RCC_OscInitStruct.HSIState = RCC_HSI_ON;
    RCC_OscInitStruct.HSIDiv = RCC_HSI_DIV1;
    RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;

    RCC_OscInitStruct.HSEState = RCC_HSE_BYPASS_DIGITAL;

    /* PLL1 = 800 MHz */
    RCC_OscInitStruct.PLL1.PLLState = RCC_PLL_ON;
    RCC_OscInitStruct.PLL1.PLLSource = RCC_PLLSOURCE_HSI;
    RCC_OscInitStruct.PLL1.PLLM = 2;
    RCC_OscInitStruct.PLL1.PLLN = 25;
    RCC_OscInitStruct.PLL1.PLLFractional = 0;
    RCC_OscInitStruct.PLL1.PLLP1 = 1;
    RCC_OscInitStruct.PLL1.PLLP2 = 1;

    /* PLL2 */
    RCC_OscInitStruct.PLL2.PLLState = RCC_PLL_ON;
    RCC_OscInitStruct.PLL2.PLLSource = RCC_PLLSOURCE_HSI;
    RCC_OscInitStruct.PLL2.PLLM = 8;
    RCC_OscInitStruct.PLL2.PLLN = 125;
    RCC_OscInitStruct.PLL2.PLLFractional = 0;
    RCC_OscInitStruct.PLL2.PLLP1 = 1;
    RCC_OscInitStruct.PLL2.PLLP2 = 1;

    /* PLL3 */
    RCC_OscInitStruct.PLL3.PLLState = RCC_PLL_ON;
    RCC_OscInitStruct.PLL3.PLLSource = RCC_PLLSOURCE_HSI;
    RCC_OscInitStruct.PLL3.PLLM = 8;
    RCC_OscInitStruct.PLL3.PLLN = 225;
    RCC_OscInitStruct.PLL3.PLLFractional = 0;
    RCC_OscInitStruct.PLL3.PLLP1 = 1;
    RCC_OscInitStruct.PLL3.PLLP2 = 2;

    /* PLL4 */
    RCC_OscInitStruct.PLL4.PLLState = RCC_PLL_ON;
    RCC_OscInitStruct.PLL4.PLLSource = RCC_PLLSOURCE_HSI;
    RCC_OscInitStruct.PLL4.PLLM = 8;
    RCC_OscInitStruct.PLL4.PLLN = 225;
    RCC_OscInitStruct.PLL4.PLLFractional = 0;
    RCC_OscInitStruct.PLL4.PLLP1 = 6;
    RCC_OscInitStruct.PLL4.PLLP2 = 6;

    if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
    {
        Error_Handler();
    }

    /*
     * Final application clocks
     *
     * CPUCLK = IC1 = PLL1 / 1 = 800 MHz
     * SYSCLK = IC2/IC6/IC11
     * HCLK   = SYSCLK / 2 = 200 MHz
     */
    RCC_ClkInitStruct.ClockType =
        RCC_CLOCKTYPE_CPUCLK |
        RCC_CLOCKTYPE_SYSCLK |
        RCC_CLOCKTYPE_HCLK |
        RCC_CLOCKTYPE_PCLK1 |
        RCC_CLOCKTYPE_PCLK2 |
        RCC_CLOCKTYPE_PCLK4 |
        RCC_CLOCKTYPE_PCLK5;

    RCC_ClkInitStruct.CPUCLKSource =
        RCC_CPUCLKSOURCE_IC1;

    RCC_ClkInitStruct.SYSCLKSource =
        RCC_SYSCLKSOURCE_IC2_IC6_IC11;

    /* IC1 = PLL1 / 1 -> CPU = 800 MHz */
    RCC_ClkInitStruct.IC1Selection.ClockSelection =
        RCC_ICCLKSOURCE_PLL1;
    RCC_ClkInitStruct.IC1Selection.ClockDivider = 1;

    /* IC2 = PLL1 / 2 -> 400 MHz */
    RCC_ClkInitStruct.IC2Selection.ClockSelection =
        RCC_ICCLKSOURCE_PLL1;
    RCC_ClkInitStruct.IC2Selection.ClockDivider = 2;

    /* IC6 = PLL2 / 1 */
    RCC_ClkInitStruct.IC6Selection.ClockSelection =
        RCC_ICCLKSOURCE_PLL2;
    RCC_ClkInitStruct.IC6Selection.ClockDivider = 1;

    /* IC11 = PLL3 / 1 */
    RCC_ClkInitStruct.IC11Selection.ClockSelection =
        RCC_ICCLKSOURCE_PLL3;
    RCC_ClkInitStruct.IC11Selection.ClockDivider = 1;

    /* HCLK = 400 / 2 = 200 MHz */
    RCC_ClkInitStruct.AHBCLKDivider = RCC_HCLK_DIV2;

    RCC_ClkInitStruct.APB1CLKDivider = RCC_APB1_DIV1;
    RCC_ClkInitStruct.APB2CLKDivider = RCC_APB2_DIV1;
    RCC_ClkInitStruct.APB4CLKDivider = RCC_APB4_DIV1;
    RCC_ClkInitStruct.APB5CLKDivider = RCC_APB5_DIV1;

    if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct) != HAL_OK)
    {
        Error_Handler();
    }

    /*
     * XSPI clocks
     */
    RCC_PeriphCLKInitStruct.PeriphClockSelection =
        RCC_PERIPHCLK_XSPI1 |
        RCC_PERIPHCLK_XSPI2;

    RCC_PeriphCLKInitStruct.Xspi1ClockSelection =
        RCC_XSPI1CLKSOURCE_HCLK;

    RCC_PeriphCLKInitStruct.Xspi2ClockSelection =
        RCC_XSPI2CLKSOURCE_HCLK;

    if (HAL_RCCEx_PeriphCLKConfig(&RCC_PeriphCLKInitStruct) != HAL_OK)
    {
        Error_Handler();
    }
}

// Resource Isolation Frameword defines master and peripheral access type
static void SystemIsolation_Config(void)
{
	// Enable RIF CLock
	__HAL_RCC_RIFSC_CLK_ENABLE();

	/*RIMC configuration*/
	RIMC_MasterConfig_t RIMC_master = {0};
	RIMC_master.MasterCID = RIF_CID_1;
	RIMC_master.SecPriv = RIF_ATTRIBUTE_SEC | RIF_ATTRIBUTE_PRIV;

	/* RIMC - Bus Masters */
	HAL_RIF_RIMC_ConfigMasterAttributes(RIF_MASTER_INDEX_NPU,   &RIMC_master);
	HAL_RIF_RIMC_ConfigMasterAttributes(RIF_MASTER_INDEX_DCMIPP,&RIMC_master);
	HAL_RIF_RIMC_ConfigMasterAttributes(RIF_MASTER_INDEX_LTDC1, &RIMC_master);
	HAL_RIF_RIMC_ConfigMasterAttributes(RIF_MASTER_INDEX_LTDC2, &RIMC_master);
	HAL_RIF_RIMC_ConfigMasterAttributes(RIF_MASTER_INDEX_OTG1,  &RIMC_master);
	HAL_RIF_RIMC_ConfigMasterAttributes(RIF_MASTER_INDEX_DMA2D, &RIMC_master);
	HAL_RIF_RIMC_ConfigMasterAttributes(RIF_MASTER_INDEX_ETH1, &RIMC_master);
	HAL_RIF_RIMC_ConfigMasterAttributes(RIF_MASTER_INDEX_SDMMC2, &RIMC_master);

	/* RISC / RISUP - Peripheral Resources */
	HAL_RIF_RISC_SetSlaveSecureAttributes(RIF_RISC_PERIPH_INDEX_NPU,    RIF_ATTRIBUTE_SEC | RIF_ATTRIBUTE_PRIV);
	HAL_RIF_RISC_SetSlaveSecureAttributes(RIF_RISC_PERIPH_INDEX_CSI,    RIF_ATTRIBUTE_SEC | RIF_ATTRIBUTE_PRIV);
	HAL_RIF_RISC_SetSlaveSecureAttributes(RIF_RISC_PERIPH_INDEX_DCMIPP, RIF_ATTRIBUTE_SEC | RIF_ATTRIBUTE_PRIV);
	HAL_RIF_RISC_SetSlaveSecureAttributes(RIF_RISC_PERIPH_INDEX_LTDC,   RIF_ATTRIBUTE_SEC | RIF_ATTRIBUTE_PRIV);
	HAL_RIF_RISC_SetSlaveSecureAttributes(RIF_RISC_PERIPH_INDEX_LTDCL1, RIF_ATTRIBUTE_SEC | RIF_ATTRIBUTE_PRIV);
	HAL_RIF_RISC_SetSlaveSecureAttributes(RIF_RISC_PERIPH_INDEX_LTDCL2, RIF_ATTRIBUTE_SEC | RIF_ATTRIBUTE_PRIV);
	HAL_RIF_RISC_SetSlaveSecureAttributes(RIF_RISC_PERIPH_INDEX_DMA2D,  RIF_ATTRIBUTE_SEC | RIF_ATTRIBUTE_PRIV);
	HAL_RIF_RISC_SetSlaveSecureAttributes(RIF_RISC_PERIPH_INDEX_OTG1HS, RIF_ATTRIBUTE_SEC | RIF_ATTRIBUTE_PRIV);
	HAL_RIF_RISC_SetSlaveSecureAttributes(RIF_RISC_PERIPH_INDEX_ADC12,  RIF_ATTRIBUTE_SEC | RIF_ATTRIBUTE_PRIV);
	HAL_RIF_RISC_SetSlaveSecureAttributes(RIF_RISC_PERIPH_INDEX_ETH1, 	RIF_ATTRIBUTE_SEC | RIF_ATTRIBUTE_PRIV);
	HAL_RIF_RISC_SetSlaveSecureAttributes(RIF_RISC_PERIPH_INDEX_SDMMC2, RIF_ATTRIBUTE_SEC | RIF_ATTRIBUTE_PRIV);

}

// Memory configuration cachable noncachable areas are defined
void MPU_Config(void)
{
    MPU_Region_InitTypeDef MPU_InitStruct = {0};
    MPU_Attributes_InitTypeDef MPU_AttributesInit = {0};
    uint32_t primask_bit = __get_PRIMASK();

    __disable_irq();
    HAL_MPU_Disable();

    /* Region 0: ETH RX/TX descriptors - NON-CACHEABLE 0x341EAE80 - 0x341EAFFF = 384 bytes */
    MPU_InitStruct.Enable = MPU_REGION_ENABLE;
    MPU_InitStruct.Number = MPU_REGION_NUMBER0;
    MPU_InitStruct.BaseAddress = 0x341EAE80;
    MPU_InitStruct.LimitAddress = 0x341EAFFF;
    MPU_InitStruct.AttributesIndex = MPU_ATTRIBUTES_NUMBER0;
    MPU_InitStruct.AccessPermission = MPU_REGION_ALL_RW;
    MPU_InitStruct.DisableExec = MPU_INSTRUCTION_ACCESS_ENABLE;
    MPU_InitStruct.DisablePrivExec = MPU_PRIV_INSTRUCTION_ACCESS_ENABLE;
    MPU_InitStruct.IsShareable = MPU_ACCESS_NOT_SHAREABLE;
    HAL_MPU_ConfigRegion(&MPU_InitStruct);

    /* Region 1: NetX pool - CACHEABLE 0x341EB000 - 0x341F7FFF = 52 KB */
    MPU_InitStruct.Number = MPU_REGION_NUMBER1;
    MPU_InitStruct.BaseAddress = 0x341EB000;
    MPU_InitStruct.LimitAddress = 0x341F7FFF;
    MPU_InitStruct.AttributesIndex = MPU_ATTRIBUTES_NUMBER1;
    HAL_MPU_ConfigRegion(&MPU_InitStruct);

    /* Region 2: USB RAM - NON-CACHEABLE 0x341F8000 - 0x341FFFFF = 32 KB */
    MPU_InitStruct.Number = MPU_REGION_NUMBER2;
    MPU_InitStruct.BaseAddress = 0x341F8000;
    MPU_InitStruct.LimitAddress = 0x341FFFFF;
    MPU_InitStruct.AttributesIndex = MPU_ATTRIBUTES_NUMBER0;
    MPU_InitStruct.AccessPermission = MPU_REGION_PRIV_RW;
    HAL_MPU_ConfigRegion(&MPU_InitStruct);

    /* Attribute 0: Non-cacheable */
    MPU_AttributesInit.Number = MPU_ATTRIBUTES_NUMBER0;
    MPU_AttributesInit.Attributes = INNER_OUTER(MPU_NOT_CACHEABLE);
    HAL_MPU_ConfigMemoryAttributes(&MPU_AttributesInit);

    /* Attribute 1: Cacheable Write-Back + Read/Write Allocate */
    MPU_AttributesInit.Number = MPU_ATTRIBUTES_NUMBER1;
    MPU_AttributesInit.Attributes = INNER_OUTER(MPU_WRITE_BACK | MPU_RW_ALLOCATE);
    HAL_MPU_ConfigMemoryAttributes(&MPU_AttributesInit);

    HAL_MPU_Enable(MPU_PRIVILEGED_DEFAULT);
    __set_PRIMASK(primask_bit);
}

/**
  * @brief  Period elapsed callback in non blocking mode
  * @note   This function is called  when TIM6 interrupt took place, inside
  * HAL_TIM_IRQHandler(). It makes a direct call to HAL_IncTick() to increment
  * a global variable "uwTick" used as application time base.
  * @param  htim : TIM handle
  * @retval None
  */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{

  if (htim->Instance == TIM6)
  {
    HAL_IncTick();
  }
  USBPD_DPM_TimerCounter();

}

void Error_Handler(void)
{
  /* User can add his own implementation to report the HAL error return state */
  while (1)
  {
    HAL_GPIO_TogglePin(LED_RED_GPIO_Port, LED_RED_Pin);
    HAL_Delay(200);
  }
}
