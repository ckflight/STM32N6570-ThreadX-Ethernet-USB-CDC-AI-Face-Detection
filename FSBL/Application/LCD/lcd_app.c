#include "lcd_app.h"
#include "stm32n6570_discovery_lcd.h"
#include "stm32_lcd.h"

Rectangle_TypeDef lcd_bg_area = {
#if ASPECT_RATIO_MODE == ASPECT_RATIO_CROP || ASPECT_RATIO_MODE == ASPECT_RATIO_FIT
    .X0 = (LCD_FG_WIDTH - LCD_FG_HEIGHT) / 2,
#else
    .X0 = 0,
#endif
    .Y0 = 0,
    .XSize = 0,
    .YSize = 0,
};

static Rectangle_TypeDef lcd_fg_area = { .X0 = 0, .Y0 = 0, .XSize = LCD_FG_WIDTH, .YSize = LCD_FG_HEIGHT };

__attribute__((section(".psram_bss")))
__attribute__((aligned(32)))
static uint8_t lcd_bg_buffer[800 * 480 * 2];

__attribute__((section(".psram_bss")))
__attribute__((aligned(32)))
uint8_t lcd_fg_buffer[2][LCD_FG_WIDTH * LCD_FG_HEIGHT * 2];

static BSP_LCD_LayerConfig_t LayerConfig = {0};

void LCD_Init(void)
{
    BSP_LCD_Init(0, LCD_ORIENTATION_LANDSCAPE);

    LayerConfig.X0 = lcd_bg_area.X0;
    LayerConfig.Y0 = lcd_bg_area.Y0;
    LayerConfig.X1 = lcd_bg_area.X0 + lcd_bg_area.XSize;
    LayerConfig.Y1 = lcd_bg_area.Y0 + lcd_bg_area.YSize;
    LayerConfig.PixelFormat = LCD_PIXEL_FORMAT_RGB565;
    LayerConfig.Address = (uint32_t)lcd_bg_buffer;
    BSP_LCD_ConfigLayer(0, LTDC_LAYER_1, &LayerConfig);

    LayerConfig.X0 = lcd_fg_area.X0;
    LayerConfig.Y0 = lcd_fg_area.Y0;
    LayerConfig.X1 = lcd_fg_area.X0 + lcd_fg_area.XSize;
    LayerConfig.Y1 = lcd_fg_area.Y0 + lcd_fg_area.YSize;
    LayerConfig.PixelFormat = LCD_PIXEL_FORMAT_ARGB4444;
    LayerConfig.Address = (uint32_t)lcd_fg_buffer;
    BSP_LCD_ConfigLayer(0, LTDC_LAYER_2, &LayerConfig);

    UTIL_LCD_SetFuncDriver(&LCD_Driver);
    UTIL_LCD_SetLayer(LTDC_LAYER_2);
    UTIL_LCD_Clear(0x00000000);
    UTIL_LCD_SetFont(&Font20);
    UTIL_LCD_SetTextColor(UTIL_LCD_COLOR_WHITE);
}

uint8_t *LCD_GetBackgroundBuffer(void)
{
    return lcd_bg_buffer;
}
