#include "ai_app.h"
#include <assert.h>
#include "stm32n6xx_hal.h"
#include "stm32ipl.h"
#include "lcd_app.h"
#include "stai_cenk.h"

/* AI shared data */
stai_ptr nn_in;
stai_size number_output = 0;
stai_ptr nn_out[STAI_NETWORK_OUT_NUM] = {0};

STAI_NETWORK_CONTEXT_DECLARE(network_context, STAI_NETWORK_CONTEXT_SIZE)

fd_blazeface_pp_static_param_t pp_params;
fd_pp_out_t pp_output;

__attribute__((section(".psram_bss")))
__attribute__((aligned(32)))
uint8_t face_nn_in[FACE_WIDTH * FACE_HEIGHT * 3];

__attribute__((section(".psram_bss")))
__attribute__((aligned(32)))
static uint8_t camera_rgb888[480 * 480 * 3];

/* Private functions */
static void NeuralNetwork_Init(uint32_t *nn_in_length, stai_ptr *nn_out, stai_size *number_output, int32_t nn_out_len[]);
static void NPURam_Enable(void);
static void NPUCache_Config(void);

static void Face_GetROI(fd_pp_outBuffer_t *face, rectangle_t *roi);

#define IPL_MEM_POOL_SIZE (64 * 1024)
static uint8_t ipl_mem_pool[IPL_MEM_POOL_SIZE];

stai_ptr cenk_in;
stai_ptr cenk_out[1] = {0};
stai_size cenk_number_output = 0;

STAI_NETWORK_CONTEXT_DECLARE(cenk_context, STAI_CENK_CONTEXT_SIZE)
static void CenkNetwork_Init(void);

/* Public API */
void AI_Init(void)
{
    uint32_t nn_in_len = 0;
    int32_t nn_out_len[STAI_NETWORK_OUT_NUM] = {0};

    STM32Ipl_InitLib(ipl_mem_pool, sizeof(ipl_mem_pool));

    NPURam_Enable();

    NPUCache_Config();

    NeuralNetwork_Init(&nn_in_len, nn_out, &number_output, nn_out_len);
    CenkNetwork_Init();

    stai_network_info info;
    int ret = stai_network_get_info(network_context, &info);
    assert(ret == STAI_SUCCESS);

    app_postprocess_init(&pp_params, &info);
}

static void CenkNetwork_Init(void)
{
    stai_network_info info;
    int ret;

    ret = stai_cenk_init(cenk_context);
    assert(ret == STAI_SUCCESS);

    ret = stai_cenk_get_info(cenk_context, &info);
    assert(ret == STAI_SUCCESS);

    assert(info.n_inputs == 1);
    assert(info.n_outputs == 1);
    assert(info.inputs[0].size_bytes == FACE_WIDTH * FACE_HEIGHT * 3);
    assert(info.outputs[0].size_bytes == sizeof(uint8_t));

    stai_size n_inputs = 1;
    cenk_number_output = 1;

    ret = stai_cenk_get_inputs(cenk_context, &cenk_in, &n_inputs);
    assert(ret == STAI_SUCCESS);

    ret = stai_cenk_get_outputs(cenk_context, cenk_out, &cenk_number_output);
    assert(ret == STAI_SUCCESS);
}

void AI_Run(void)
{
    int ret = stai_network_run(network_context, STAI_MODE_SYNC);
    assert(ret == STAI_SUCCESS);
}

float Cenk_Run(void)
{
    memcpy(cenk_in, face_nn_in, FACE_WIDTH * FACE_HEIGHT * 3);

    int ret = stai_cenk_run(cenk_context, STAI_MODE_SYNC);
    assert(ret == STAI_SUCCESS);

    uint8_t raw = *(uint8_t *)cenk_out[0];

    return raw * 0.00390625f;
}

/* Private functions */
static void NeuralNetwork_Init(uint32_t *nn_in_length, stai_ptr *nn_out, stai_size *number_output, int32_t nn_out_len[])
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

    *number_output = STAI_NETWORK_OUT_NUM;
    *nn_in_length = info.inputs[0].size_bytes;

    ret = stai_network_get_inputs(network_context, &nn_in, (stai_size *)&info.n_inputs);
    assert(ret == STAI_SUCCESS);

    ret = stai_network_get_outputs(network_context, nn_out, number_output);
    assert(ret == STAI_SUCCESS);

    for (int i = 0; i < *number_output; i++) nn_out_len[i] = info.outputs[i].size_bytes;
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

volatile int crop_step = 0;
volatile stm32ipl_err_t resize_ret;
volatile stm32ipl_err_t convert_ret;

void Face_Crop(fd_pp_outBuffer_t *face)
{
    image_t src;
    image_t src_rgb888;
    image_t dst;
    rectangle_t roi;

    Face_GetROI(face, &roi);

    // Define image descriptors for camera, RGB888 conversion buffer, and final face buffer
    STM32Ipl_Init(&src, 480, 480, IMAGE_BPP_RGB565, LCD_GetBackgroundBuffer());
    STM32Ipl_Init(&src_rgb888, 480, 480, IMAGE_BPP_RGB888, camera_rgb888);
    STM32Ipl_Init(&dst, FACE_WIDTH, FACE_HEIGHT, IMAGE_BPP_RGB888, face_nn_in);

    // Convert the original camera image from RGB565 to RGB888
    convert_ret = STM32Ipl_Convert(&src, &src_rgb888);

    // Crop the face ROI from the RGB888 image, resize it, and write it to face_nn_in
    resize_ret = STM32Ipl_Resize_Roi(&src_rgb888, &roi, &dst, NULL, RESIZE_BILINEAR);
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
