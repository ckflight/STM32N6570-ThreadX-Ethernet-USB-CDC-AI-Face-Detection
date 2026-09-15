#include "ethernet.h"
#include "main.h"

ETH_DMADescTypeDef DMARxDscrTab[ETH_DMA_RX_CH_CNT][ETH_RX_DESC_CNT] __attribute__((section(".RxDecripSection")));
ETH_DMADescTypeDef DMATxDscrTab[ETH_DMA_TX_CH_CNT][ETH_TX_DESC_CNT] __attribute__((section(".TxDecripSection")));

ETH_HandleTypeDef heth1;

void Ethernet_Init(void)
{
    static uint8_t MACAddr[6];

    heth1.Instance = ETH1;

    MACAddr[0] = 0x00;
    MACAddr[1] = 0x80;
    MACAddr[2] = 0xE0;
    MACAddr[3] = 0x00;
    MACAddr[4] = 0x10;
    MACAddr[5] = 0x00;

    heth1.Init.MACAddr = &MACAddr[0];
    heth1.Init.MediaInterface = HAL_ETH_RGMII_MODE;

    for (int ch = 0; ch < ETH_DMA_CH_CNT; ch++)
    {
        heth1.Init.TxDesc[ch] = DMATxDscrTab[ch];
        heth1.Init.RxDesc[ch] = DMARxDscrTab[ch];
    }

    heth1.Init.RxBuffLen = 1536;

    if (HAL_ETH_Init(&heth1) != HAL_OK) Error_Handler();
}
