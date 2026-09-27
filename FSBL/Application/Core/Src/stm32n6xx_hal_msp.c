
#include "main.h"

void HAL_MspInit(void)
{

  HAL_PWREx_EnableVddIO2();

  HAL_PWREx_EnableVddIO3();

  HAL_PWREx_EnableVddIO4();

  HAL_PWREx_EnableVddIO5();
}

void HAL_ETH_MspInit(ETH_HandleTypeDef* heth)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  RCC_PeriphCLKInitTypeDef PeriphClkInitStruct = {0};
  if(heth->Instance==ETH1)
  {

  /** Initializes the peripherals clock
  */
    PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_ETH1;
    PeriphClkInitStruct.Eth1ClockSelection = RCC_ETH1CLKSOURCE_HCLK;

    if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct) != HAL_OK)
    {
      Error_Handler();
    }

    /* Peripheral clock enable */
    __HAL_RCC_ETH1_CLK_ENABLE();
    __HAL_RCC_ETH1MAC_CLK_ENABLE();
    __HAL_RCC_ETH1TX_CLK_ENABLE();
    __HAL_RCC_ETH1RX_CLK_ENABLE();

    __HAL_RCC_GPIOD_CLK_ENABLE();
    __HAL_RCC_GPIOF_CLK_ENABLE();
    __HAL_RCC_GPIOG_CLK_ENABLE();
    /**ETH1 GPIO Configuration
    PD1     ------> ETH1_MDC
    PD12     ------> ETH1_MDIO
    PD3     ------> ETH1_PHY_INTN
    PF10     ------> ETH1_RGMII_RX_CTL
    PF7     ------> ETH1_RGMII_RX_CLK
    PF5     ------> ETH1_CLK
    PF15     ------> ETH1_RGMII_RXD1
    PF14     ------> ETH1_RGMII_RXD0
    PF8     ------> ETH1_RGMII_RXD2
    PF2     ------> ETH1_RGMII_CLK125
    PF9     ------> ETH1_RGMII_RXD3
    PG4     ------> ETH1_RGMII_TXD3
    PF11     ------> ETH1_RGMII_TX_CTL
    PG3     ------> ETH1_RGMII_TXD2
    PF13     ------> ETH1_RGMII_TXD1
    PF0     ------> ETH1_RGMII_GTX_CLK
    PF12     ------> ETH1_RGMII_TXD0
    */
    GPIO_InitStruct.Pin = GPIO_PIN_1|GPIO_PIN_12|GPIO_PIN_3;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF11_ETH1;
    HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = GPIO_PIN_10|GPIO_PIN_7|GPIO_PIN_5|GPIO_PIN_15
                          |GPIO_PIN_14|GPIO_PIN_8|GPIO_PIN_2|GPIO_PIN_9
                          |GPIO_PIN_11|GPIO_PIN_13|GPIO_PIN_12;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF11_ETH1;
    HAL_GPIO_Init(GPIOF, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = GPIO_PIN_4|GPIO_PIN_3;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF11_ETH1;
    HAL_GPIO_Init(GPIOG, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = GPIO_PIN_0;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_MEDIUM;
    GPIO_InitStruct.Alternate = GPIO_AF12_ETH1;
    HAL_GPIO_Init(GPIOF, &GPIO_InitStruct);

    /* ETH1 interrupt Init */
    HAL_NVIC_SetPriority(ETH1_IRQn, 7, 0);
    HAL_NVIC_EnableIRQ(ETH1_IRQn);

  }

}

void HAL_ETH_MspDeInit(ETH_HandleTypeDef* heth)
{
  if(heth->Instance==ETH1)
  {

    /* Peripheral clock disable */
    __HAL_RCC_ETH1_CLK_DISABLE();
    __HAL_RCC_ETH1MAC_CLK_DISABLE();
    __HAL_RCC_ETH1TX_CLK_DISABLE();
    __HAL_RCC_ETH1RX_CLK_DISABLE();

    /**ETH1 GPIO Configuration
    PD1     ------> ETH1_MDC
    PD12     ------> ETH1_MDIO
    PD3     ------> ETH1_PHY_INTN
    PF10     ------> ETH1_RGMII_RX_CTL
    PF7     ------> ETH1_RGMII_RX_CLK
    PF5     ------> ETH1_CLK
    PF15     ------> ETH1_RGMII_RXD1
    PF14     ------> ETH1_RGMII_RXD0
    PF8     ------> ETH1_RGMII_RXD2
    PF2     ------> ETH1_RGMII_CLK125
    PF9     ------> ETH1_RGMII_RXD3
    PG4     ------> ETH1_RGMII_TXD3
    PF11     ------> ETH1_RGMII_TX_CTL
    PG3     ------> ETH1_RGMII_TXD2
    PF13     ------> ETH1_RGMII_TXD1
    PF0     ------> ETH1_RGMII_GTX_CLK
    PF12     ------> ETH1_RGMII_TXD0
    */
    HAL_GPIO_DeInit(GPIOD, GPIO_PIN_1|GPIO_PIN_12|GPIO_PIN_3);

    HAL_GPIO_DeInit(GPIOF, GPIO_PIN_10|GPIO_PIN_7|GPIO_PIN_5|GPIO_PIN_15
                          |GPIO_PIN_14|GPIO_PIN_8|GPIO_PIN_2|GPIO_PIN_9
                          |GPIO_PIN_11|GPIO_PIN_13|GPIO_PIN_0|GPIO_PIN_12);

    HAL_GPIO_DeInit(GPIOG, GPIO_PIN_4|GPIO_PIN_3);

    /* ETH1 interrupt DeInit */
    HAL_NVIC_DisableIRQ(ETH1_IRQn);

  }

}

void HAL_SD_MspInit(SD_HandleTypeDef* hsd)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  RCC_PeriphCLKInitTypeDef PeriphClkInitStruct = {0};
  if(hsd->Instance==SDMMC2)
  {

  /** Initializes the peripherals clock
  */
    PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_SDMMC2;
    PeriphClkInitStruct.Sdmmc2ClockSelection = RCC_SDMMC2CLKSOURCE_IC4;
    PeriphClkInitStruct.ICSelection[RCC_IC4].ClockSelection = RCC_ICCLKSOURCE_PLL1;
    PeriphClkInitStruct.ICSelection[RCC_IC4].ClockDivider = 6;
    if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct) != HAL_OK)
    {
      Error_Handler();
    }

    /* Peripheral clock enable */
    __HAL_RCC_SDMMC2_CLK_ENABLE();

    __HAL_RCC_GPIOC_CLK_ENABLE();
    __HAL_RCC_GPIOE_CLK_ENABLE();
    /**SDMMC2 GPIO Configuration
    PC4     ------> SDMMC2_D0
    PC5     ------> SDMMC2_D1
    PC0     ------> SDMMC2_D2
    PC2     ------> SDMMC2_CK
    PE4     ------> SDMMC2_D3
    PC3     ------> SDMMC2_CMD
    */
    GPIO_InitStruct.Pin = GPIO_PIN_4|GPIO_PIN_5|GPIO_PIN_0|GPIO_PIN_2
                          |GPIO_PIN_3;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF11_SDMMC2;
    HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = GPIO_PIN_4;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF11_SDMMC2;
    HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);

    /* SDMMC2 interrupt Init */
    HAL_NVIC_SetPriority(SDMMC2_IRQn, 5, 0);
    HAL_NVIC_EnableIRQ(SDMMC2_IRQn);

  }

}

void HAL_SD_MspDeInit(SD_HandleTypeDef* hsd)
{
  if(hsd->Instance==SDMMC2)
  {

    /* Peripheral clock disable */
    __HAL_RCC_SDMMC2_CLK_DISABLE();

    /**SDMMC2 GPIO Configuration
    PC4     ------> SDMMC2_D0
    PC5     ------> SDMMC2_D1
    PC0     ------> SDMMC2_D2
    PC2     ------> SDMMC2_CK
    PE4     ------> SDMMC2_D3
    PC3     ------> SDMMC2_CMD
    */
    HAL_GPIO_DeInit(GPIOC, GPIO_PIN_4|GPIO_PIN_5|GPIO_PIN_0|GPIO_PIN_2
                          |GPIO_PIN_3);

    HAL_GPIO_DeInit(GPIOE, GPIO_PIN_4);

    /* SDMMC2 interrupt DeInit */
    HAL_NVIC_DisableIRQ(SDMMC2_IRQn);

  }

}
/* USER CODE END 1 */
