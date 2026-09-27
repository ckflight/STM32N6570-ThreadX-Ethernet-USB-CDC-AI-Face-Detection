#ifndef SYSTIMER_H
#define SYSTIMER_H

#include "main.h"

void Timer_Init(void);
uint32_t Timer_GetMicros(void);
uint32_t Timer_GetMillis(void);
void Timer_DelayUs(uint32_t us);
void Timer_DelayMs(uint32_t ms);

#endif
