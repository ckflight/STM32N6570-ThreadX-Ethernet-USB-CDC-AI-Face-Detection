#include "systimer.h"

static uint32_t cycles_per_us;

void Timer_Init(void)
{
    SystemCoreClockUpdate();
    cycles_per_us = SystemCoreClock / 1000000U;
    CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
    DWT->CYCCNT = 0;
    DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;
}

uint32_t Timer_GetMicros(void)
{
    return DWT->CYCCNT / cycles_per_us;
}

uint32_t Timer_GetMillis(void)
{
    return Timer_GetMicros() / 1000U;
}

void Timer_DelayUs(uint32_t us)
{
    uint32_t start = DWT->CYCCNT;
    uint32_t cycles = us * cycles_per_us;
    while ((uint32_t)(DWT->CYCCNT - start) < cycles);
}

void Timer_DelayMs(uint32_t ms)
{
    while (ms--) Timer_DelayUs(1000U);
}
