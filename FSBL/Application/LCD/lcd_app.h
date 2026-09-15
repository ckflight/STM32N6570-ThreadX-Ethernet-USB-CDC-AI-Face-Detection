#ifndef LCD_APP_H
#define LCD_APP_H

#include <stdint.h>
#include "app_config.h"
#include "app_camerapipeline.h"

#define LCD_FG_WIDTH SCREEN_WIDTH
#define LCD_FG_HEIGHT SCREEN_HEIGHT
#define LCD_FG_FRAMEBUFFER_SIZE (LCD_FG_WIDTH * LCD_FG_HEIGHT * 2)

typedef struct
{
    uint32_t X0;
    uint32_t Y0;
    uint32_t XSize;
    uint32_t YSize;
} Rectangle_TypeDef;

extern Rectangle_TypeDef lcd_bg_area;
extern uint8_t lcd_fg_buffer[2][LCD_FG_WIDTH * LCD_FG_HEIGHT * 2];

void LCD_Init(void);
uint8_t *LCD_GetBackgroundBuffer(void);

#endif
