#include "camera_app.h"
#include "app_camerapipeline.h"
#include "lcd_app.h"
#include "ai_app.h"

void Camera_Init(void)
{
    uint32_t pitch_nn = 0;
    CameraPipeline_Init(&lcd_bg_area.XSize, &lcd_bg_area.YSize, &pitch_nn);
}

void Camera_Start(void)
{
    CameraPipeline_DisplayPipe_Start(LCD_GetBackgroundBuffer(), DCMIPP_MODE_CONTINUOUS);
    CameraPipeline_IspUpdate();
    CameraPipeline_NNPipe_Start((uint8_t *)nn_in, DCMIPP_MODE_SNAPSHOT);
}

HAL_StatusTypeDef MX_DCMIPP_ClockConfig(DCMIPP_HandleTypeDef *hdcmipp)
{
    RCC_PeriphCLKInitTypeDef RCC_PeriphCLKInitStruct = {0};
    HAL_StatusTypeDef ret = HAL_OK;

    RCC_PeriphCLKInitStruct.PeriphClockSelection = RCC_PERIPHCLK_DCMIPP;
    RCC_PeriphCLKInitStruct.DcmippClockSelection = RCC_DCMIPPCLKSOURCE_IC17;
    RCC_PeriphCLKInitStruct.ICSelection[RCC_IC17].ClockSelection = RCC_ICCLKSOURCE_PLL2;
    RCC_PeriphCLKInitStruct.ICSelection[RCC_IC17].ClockDivider = 3;

    ret = HAL_RCCEx_PeriphCLKConfig(&RCC_PeriphCLKInitStruct);
    if (ret)
    {
        return ret;
    }

    RCC_PeriphCLKInitStruct.PeriphClockSelection = RCC_PERIPHCLK_CSI;
    RCC_PeriphCLKInitStruct.ICSelection[RCC_IC18].ClockSelection = RCC_ICCLKSOURCE_PLL1;
    RCC_PeriphCLKInitStruct.ICSelection[RCC_IC18].ClockDivider = 40;

    ret = HAL_RCCEx_PeriphCLKConfig(&RCC_PeriphCLKInitStruct);

    return ret;
}
