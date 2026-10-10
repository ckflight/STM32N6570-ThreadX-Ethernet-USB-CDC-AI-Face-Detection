#include "lcd_app.h"
#include "stm32n6570_discovery_lcd.h"
#include "stm32_lcd.h"
#include "ai_app.h"

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

void LCD_ShowFaceCrop1(uint8_t *cenk_in_buffer) // add cropped image data to fg of lcd
{
	// Take the lcd fg buffer and write face buffer on it.
    uint16_t *dst = (uint16_t *)lcd_fg_buffer[0];

    for (int y = 0; y < STAI_CENK_IN_1_HEIGHT; y++)
    {
        for (int x = 0; x < STAI_CENK_IN_1_WIDTH; x++)
        {
            int i = (y * STAI_CENK_IN_1_WIDTH + x) * 3;
            uint8_t b = cenk_in_buffer[i];
            uint8_t g = cenk_in_buffer[i + 1];
            uint8_t r = cenk_in_buffer[i + 2];

            dst[y * SCREEN_WIDTH + x] = 0xF000 | ((r >> 4) << 8) | ((g >> 4) << 4) | (b >> 4);
        }
    }

    SCB_CleanDCache_by_Addr((uint32_t *)lcd_fg_buffer[0], SCREEN_WIDTH * SCREEN_HEIGHT * 2);
}

void LCD_ShowFaceCrop2(uint8_t *reid_in_buffer) // add cropped image data to fg of lcd
{
    uint16_t *dst = (uint16_t *)lcd_fg_buffer[0];

    for (int y = 0; y < STAI_REID_IN_1_HEIGHT; y++)
    {
        for (int x = 0; x < STAI_REID_IN_1_WIDTH; x++)
        {
            int i = (y * STAI_REID_IN_1_WIDTH + x) * 3;
            uint8_t b = reid_in_buffer[i];
            uint8_t g = reid_in_buffer[i + 1];
            uint8_t r = reid_in_buffer[i + 2];

            dst[(y + 200) * SCREEN_WIDTH + x] = 0xF000 | ((r >> 4) << 8) | ((g >> 4) << 4) | (b >> 4);
        }
    }

    SCB_CleanDCache_by_Addr((uint32_t *)lcd_fg_buffer[0], SCREEN_WIDTH * SCREEN_HEIGHT * 2);
}

