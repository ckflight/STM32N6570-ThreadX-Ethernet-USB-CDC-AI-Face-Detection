#include "camera_app.h"
#include "app_camerapipeline.h"
#include "lcd_app.h"
#include "ai_app.h"

void Camera_Init(void)
{
    // Init camera and pipe lines that will be used by lcd and nn.
	// Original camera size is 2592×1944
	// LCD crops and uses 480x480x2 format for LCD compatibility. DCMIPP has this crop and pixel format capability.
	// NN uses 128x128x3 format
    CameraPipeline_Init(&lcd_bg_area.XSize, &lcd_bg_area.YSize);
}

void Camera_Start(void)
{

	// Start the camera and send pipe data (pipe1) to LCD_GetBackgroundBuffer() buffer in continous mode
    CameraPipeline_DisplayPipe_Start(LCD_GetBackgroundBuffer(), DCMIPP_MODE_CONTINUOUS);

    CameraPipeline_IspUpdate();

    // Start the camera and send pipe data (pipe2) to nn_in buffer in snapshot mode
    // Snap mod uses this function in CameraPipeline_NNPipe_Start to get new data from camera as it completes processing the current camera data.
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
