/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    ux_device_cdc_acm.c
  * @author  MCD Application Team
  * @brief   USBX Device applicative file
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2024 STMicroelectronics.
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
#include "ux_device_cdc_acm.h"

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

UX_SLAVE_CLASS_CDC_ACM  *cdc_acm;

/* Data received over uart are stored in this buffer */
/* Set UserRxBufferFS at the end of non-cacheable */
#if defined ( __ICCARM__ ) /* IAR Compiler */
#pragma location = ".UsbxAppSection"
#else /* GNU and AC6 Compiler */
__attribute__((section(".UsbxAppSection")))
#endif

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN PFP */
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
UX_SLAVE_CLASS_CDC_ACM_LINE_CODING_PARAMETER CDC_VCP_LineCoding =
{
    115200,
    0x00,
    0x00,
    0x08
};
/* USER CODE END 0 */

/**
  * @brief  USBD_CDC_ACM_Activate
  *         This function is called when insertion of a CDC ACM device.
  * @param  cdc_acm_instance: Pointer to the cdc acm class instance.
  * @retval none
  */
VOID USBD_CDC_ACM_Activate(VOID *cdc_acm_instance)
{
  /* USER CODE BEGIN USBD_CDC_ACM_Activate */

  /* Save the CDC instance */
  cdc_acm = (UX_SLAVE_CLASS_CDC_ACM*) cdc_acm_instance;

  /* USER CODE END USBD_CDC_ACM_Activate */

  return;
}

/**
  * @brief  USBD_CDC_ACM_Deactivate
  *         This function is called when extraction of a CDC ACM device.
  * @param  cdc_acm_instance: Pointer to the cdc acm class instance.
  * @retval none
  */
VOID USBD_CDC_ACM_Deactivate(VOID *cdc_acm_instance)
{
  /* USER CODE BEGIN USBD_CDC_ACM_Deactivate */
  UX_PARAMETER_NOT_USED(cdc_acm_instance);

  /* Reset the cdc acm instance */
  cdc_acm = UX_NULL;

  /* USER CODE END USBD_CDC_ACM_Deactivate */

  return;
}

/**
  * @brief  USBD_CDC_ACM_ParameterChange
  *         This function is invoked to manage the CDC ACM class requests.
  * @param  cdc_acm_instance: Pointer to the cdc acm class instance.
  * @retval none
  */
VOID USBD_CDC_ACM_ParameterChange(VOID *cdc_acm_instance)
{
    ULONG request;
    UX_SLAVE_TRANSFER *transfer_request;
    UX_SLAVE_DEVICE *device;

    UX_PARAMETER_NOT_USED(cdc_acm_instance);

    device = &_ux_system_slave->ux_system_slave_device;

    transfer_request =
        &device->ux_slave_device_control_endpoint
               .ux_slave_endpoint_transfer_request;

    request = *(transfer_request->ux_slave_transfer_request_setup
                + UX_SETUP_REQUEST);

    switch (request)
    {
        case UX_SLAVE_CLASS_CDC_ACM_SET_LINE_CODING:

            ux_device_class_cdc_acm_ioctl(
                cdc_acm,
                UX_SLAVE_CLASS_CDC_ACM_IOCTL_GET_LINE_CODING,
                &CDC_VCP_LineCoding
            );

            break;


        case UX_SLAVE_CLASS_CDC_ACM_GET_LINE_CODING:

            ux_device_class_cdc_acm_ioctl(
                cdc_acm,
                UX_SLAVE_CLASS_CDC_ACM_IOCTL_SET_LINE_CODING,
                &CDC_VCP_LineCoding
            );

            break;


        case UX_SLAVE_CLASS_CDC_ACM_SET_CONTROL_LINE_STATE:
        default:
            break;
    }
}

/* USER CODE BEGIN 1 */


/* USER CODE END 1 */
