#include "ai_app.h"
#include <assert.h>
#include "stm32n6xx_hal.h"
#include "stm32ipl.h"
#include "lcd_app.h"

#define IPL_MEM_POOL_SIZE (64 * 1024)
static uint8_t ipl_mem_pool[IPL_MEM_POOL_SIZE];

__attribute__((section(".psram_bss")))
__attribute__((aligned(32))) static uint8_t camera_rgb888[480 * 480 * 3];

// Face Detection Network
stai_ptr 	nn_in;
stai_ptr 	nn_out[STAI_NETWORK_OUT_NUM] = {0};
stai_size 	number_output;
STAI_NETWORK_CONTEXT_DECLARE(network_context, STAI_NETWORK_CONTEXT_SIZE)

fd_blazeface_pp_static_param_t pp_params;
fd_pp_out_t pp_output;

// Cenk Network
stai_ptr cenk_in;
stai_ptr cenk_out[STAI_CENK_OUT_NUM] = {0};
STAI_NETWORK_CONTEXT_DECLARE(cenk_context, STAI_CENK_CONTEXT_SIZE)

// Reid Network
stai_ptr reid_in;
stai_ptr reid_out[STAI_REID_OUT_NUM] = {0};
STAI_NETWORK_CONTEXT_DECLARE(reid_context, STAI_REID_CONTEXT_SIZE)

static void NeuralNetwork_Init(void);
static void CenkNetwork_Init(void);
static void ReID_Init(void);

static void Face_GetROI(fd_pp_outBuffer_t *face, rectangle_t *roi);

static void NPURam_Enable(void);
static void NPUCache_Config(void);


void AI_Init(void)
{
    STM32Ipl_InitLib(ipl_mem_pool, sizeof(ipl_mem_pool));

    NPURam_Enable();

    NPUCache_Config();

    NeuralNetwork_Init();
    CenkNetwork_Init();
    //ReID_Init();

    stai_network_info info;
    int ret = stai_network_get_info(network_context, &info);
    assert(ret == STAI_SUCCESS);

    app_postprocess_init(&pp_params, &info);
}

static void NeuralNetwork_Init(void)
{
    stai_network_info info;
    int ret;

    ret = stai_runtime_init();
    assert(ret == STAI_SUCCESS);

    ret = stai_network_init(network_context);
    assert(ret == STAI_SUCCESS);

    ret = stai_network_get_info(network_context, &info);
    assert(ret == STAI_SUCCESS);

    assert(info.n_inputs == 1);

    stai_size number_inputs = STAI_NETWORK_IN_NUM;
    number_output = STAI_NETWORK_OUT_NUM;

    ret = stai_network_get_inputs(network_context, &nn_in, &number_inputs);
    assert(ret == STAI_SUCCESS);

    ret = stai_network_get_outputs(network_context, nn_out, &number_output);
    assert(ret == STAI_SUCCESS);

}

static void CenkNetwork_Init(void)
{
    stai_network_info info;
    int ret;

    ret = stai_cenk_init(cenk_context);
    assert(ret == STAI_SUCCESS);

    ret = stai_cenk_get_info(cenk_context, &info);
    assert(ret == STAI_SUCCESS);

    stai_size cenk_number_inputs 	= STAI_CENK_IN_NUM;
    stai_size cenk_number_outputs 	= STAI_CENK_OUT_NUM;

    ret = stai_cenk_get_inputs(cenk_context, &cenk_in, &cenk_number_inputs);
    assert(ret == STAI_SUCCESS);

    ret = stai_cenk_get_outputs(cenk_context, cenk_out, &cenk_number_outputs);
    assert(ret == STAI_SUCCESS);

}

static void ReID_Init(void)
{
    stai_network_info info;
    int ret;

	// init model instance
	ret = stai_reid_init(reid_context);
	assert(ret == STAI_SUCCESS);

    ret = stai_reid_get_info(reid_context, &info);
    assert(ret == STAI_SUCCESS);

    stai_size reid_number_inputs 	= STAI_REID_IN_NUM;
    stai_size reid_number_outputs 	= STAI_REID_OUT_NUM;

	// Fetch input location
	ret = stai_reid_get_inputs(reid_context, &reid_in, &reid_number_inputs);
	assert(ret == STAI_SUCCESS);

    ret = stai_reid_get_outputs(reid_context, reid_out, &reid_number_outputs);
    assert(ret == STAI_SUCCESS);
}

void AI_Run(void)
{
	// Init ile set edilen nn_in nn_out fonksiyonları ile networkü çalıştır.
    int ret = stai_network_run(network_context, STAI_MODE_SYNC);
    assert(ret == STAI_SUCCESS);
}

int Cenk_Run(float *percentage)
{
	// CleanDCache: CPU cache'indeki değiştirilmiş input verisini RAM'e yazar. Böylece NPU güncel veriyi okur.
    //SCB_CleanDCache_by_Addr((uint32_t *)cenk_in, STAI_CENK_IN_1_WIDTH * STAI_CENK_IN_1_HEIGHT * STAI_CENK_IN_1_CHANNEL);

    // stai_cenk_run: NPU inference yapar ve sonucu cenk_out[0] adresindeki output buffer'a yazar.
    int ret = stai_cenk_run(cenk_context, STAI_MODE_SYNC);
    assert(ret == STAI_SUCCESS);

    // InvalidateDCache: CPU cache'indeki eski output verisini geçersiz kılar. CPU daha sonra output'u okuduğunda güncel veriyi RAM'den cache'e getirir.
    //SCB_InvalidateDCache_by_Addr((uint32_t *)cenk_out[0], STAI_CENK_OUT_1_WIDTH * STAI_CENK_OUT_1_HEIGHT * STAI_CENK_OUT_1_CHANNEL);

    // Modelin 1 outputu var yüzde kaç Cenk'e benizyor sonucu uint8_t olduğundan 0 ile 255 arası değer çıkıyor bunu normalize edip yolaldık.
    uint8_t raw = *(uint8_t *)cenk_out[0];

    *percentage = raw * (1.0f/256.0f);

    return ret;
}

int ReID_Run(void)
{
	// CleanDCache: CPU cache'indeki değiştirilmiş input verisini RAM'e yazar. Böylece NPU güncel veriyi okur.
    SCB_CleanDCache_by_Addr((uint32_t *)reid_in, STAI_REID_IN_1_WIDTH * STAI_REID_IN_1_HEIGHT * STAI_REID_IN_1_CHANNEL);

    // NPU inference yapar ve sonucu cenk_out[0] adresindeki output buffer'a yazar.
    int ret = stai_reid_run(reid_context, STAI_MODE_SYNC);
    assert(ret == STAI_SUCCESS);

    // InvalidateDCache: CPU cache'indeki eski output verisini geçersiz kılar. CPU daha sonra output'u okuduğunda güncel veriyi RAM'den cache'e getirir.
    SCB_InvalidateDCache_by_Addr((uint32_t *)reid_out[0], STAI_REID_OUT_1_WIDTH * STAI_REID_OUT_1_HEIGHT * STAI_REID_OUT_1_CHANNEL);

    return ret;
}

static void Face_GetROI(fd_pp_outBuffer_t *face, rectangle_t *roi)
{
    int x0 = (int)((face->x_center - face->width * 0.5f) * 480.0f);
    int y0 = (int)((face->y_center - face->height * 0.5f) * 480.0f);
    int x1 = (int)((face->x_center + face->width * 0.5f) * 480.0f);
    int y1 = (int)((face->y_center + face->height * 0.5f) * 480.0f);

    if (x0 < 0) x0 = 0;
    if (y0 < 0) y0 = 0;
    if (x1 > 479) x1 = 479;
    if (y1 > 479) y1 = 479;

    STM32Ipl_RectInit(roi, x0, y0, x1 - x0 + 1, y1 - y0 + 1);
}

void Face_Crop(fd_pp_outBuffer_t *face_coordinates)
{
    image_t src;
    image_t src_rgb888;
    image_t dst;
    rectangle_t roi;

    stm32ipl_err_t ret;
    UNUSED(ret);

    Face_GetROI(face_coordinates, &roi);

    // Define image descriptors for camera, RGB888 conversion buffer, and final face buffer
    STM32Ipl_Init(&src, 480, 480, IMAGE_BPP_RGB565, LCD_GetBackgroundBuffer());
    STM32Ipl_Init(&src_rgb888, 480, 480, IMAGE_BPP_RGB888, camera_rgb888);
    STM32Ipl_Init(&dst, STAI_CENK_IN_1_WIDTH, STAI_CENK_IN_1_HEIGHT, IMAGE_BPP_RGB888, cenk_in);

    // Convert the original camera image from RGB565 to RGB888
    ret = STM32Ipl_Convert(&src, &src_rgb888);

    // Crop the face ROI from the RGB888 image, resize it, and write it to face_nn_in
    ret = STM32Ipl_Resize_Roi(&src_rgb888, &roi, &dst, NULL, RESIZE_BILINEAR);
}

void Face_Crop2(fd_pp_outBuffer_t *face_coordinates)
{
    image_t src;
    image_t src_rgb888;
    image_t dst;
    rectangle_t roi;

    stm32ipl_err_t ret;
    UNUSED(ret);

    Face_GetROI(face_coordinates, &roi);

    STM32Ipl_Init(&src, 480, 480, IMAGE_BPP_RGB565, LCD_GetBackgroundBuffer());
    STM32Ipl_Init(&src_rgb888, 480, 480, IMAGE_BPP_RGB888, camera_rgb888);
    STM32Ipl_Init(&dst, STAI_REID_IN_1_WIDTH, STAI_REID_IN_1_HEIGHT, IMAGE_BPP_RGB888, reid_in);

    ret = STM32Ipl_Convert(&src, &src_rgb888);

    ret = STM32Ipl_Resize_Roi(&src_rgb888, &roi, &dst, NULL, RESIZE_BILINEAR);
}

static void NPURam_Enable(void)
{

    __HAL_RCC_NPU_CLK_ENABLE();

    __HAL_RCC_NPU_FORCE_RESET();
    __HAL_RCC_NPU_RELEASE_RESET();

    __HAL_RCC_AXISRAM3_MEM_CLK_ENABLE();
    __HAL_RCC_AXISRAM4_MEM_CLK_ENABLE();
    __HAL_RCC_AXISRAM5_MEM_CLK_ENABLE();
    __HAL_RCC_AXISRAM6_MEM_CLK_ENABLE();

    __HAL_RCC_RAMCFG_CLK_ENABLE();

    RAMCFG_HandleTypeDef hramcfg = {0};

    hramcfg.Instance = RAMCFG_SRAM3_AXI;
    HAL_RAMCFG_EnableAXISRAM(&hramcfg);

    hramcfg.Instance = RAMCFG_SRAM4_AXI;
    HAL_RAMCFG_EnableAXISRAM(&hramcfg);

    hramcfg.Instance = RAMCFG_SRAM5_AXI;
    HAL_RAMCFG_EnableAXISRAM(&hramcfg);

    hramcfg.Instance = RAMCFG_SRAM6_AXI;
    HAL_RAMCFG_EnableAXISRAM(&hramcfg);

}

static void NPUCache_Config(void)
{
    npu_cache_enable();
}

void npu_cache_enable_clocks_and_reset(void)
{
    __HAL_RCC_CACHEAXIRAM_MEM_CLK_ENABLE();
    __HAL_RCC_CACHEAXI_CLK_ENABLE();
    __HAL_RCC_CACHEAXI_FORCE_RESET();
    __HAL_RCC_CACHEAXI_RELEASE_RESET();
}

void npu_cache_disable_clocks_and_reset(void)
{
    __HAL_RCC_CACHEAXIRAM_MEM_CLK_DISABLE();
    __HAL_RCC_CACHEAXI_CLK_DISABLE();
    __HAL_RCC_CACHEAXI_FORCE_RESET();
}

