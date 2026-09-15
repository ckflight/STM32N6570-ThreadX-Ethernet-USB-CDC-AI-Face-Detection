#include "sd.h"
#include "main.h"

SD_HandleTypeDef hsd2;

void SD_Init(void)
{
    hsd2.Instance = SDMMC2;
    hsd2.Init.ClockEdge = SDMMC_CLOCK_EDGE_RISING;
    hsd2.Init.ClockPowerSave = SDMMC_CLOCK_POWER_SAVE_DISABLE;
    hsd2.Init.BusWide = SDMMC_BUS_WIDE_4B;
    hsd2.Init.HardwareFlowControl = SDMMC_HARDWARE_FLOW_CONTROL_DISABLE;
    hsd2.Init.ClockDiv = 0;

    if (HAL_SD_Init(&hsd2) != HAL_OK) Error_Handler();
}
