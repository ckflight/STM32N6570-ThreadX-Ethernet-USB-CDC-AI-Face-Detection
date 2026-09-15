#include "ai_app.h"
#include <assert.h>
#include "stm32n6xx_hal.h"

/* AI shared data */
stai_ptr nn_in;
stai_size number_output = 0;
stai_ptr nn_out[STAI_NETWORK_OUT_NUM] = {0};

STAI_NETWORK_CONTEXT_DECLARE(network_context, STAI_NETWORK_CONTEXT_SIZE)

fd_blazeface_pp_static_param_t pp_params;
fd_pp_out_t pp_output;

/* Private functions */
static void NeuralNetwork_Init(uint32_t *nn_in_length, stai_ptr *nn_out, stai_size *number_output, int32_t nn_out_len[]);
static void NPURam_Enable(void);
static void NPUCache_Config(void);

/* Public API */
void AI_Init(void)
{
    uint32_t nn_in_len = 0;
    int32_t nn_out_len[STAI_NETWORK_OUT_NUM] = {0};

    NPURam_Enable();
    NPUCache_Config();

    NeuralNetwork_Init(&nn_in_len, nn_out, &number_output, nn_out_len);

    stai_network_info info;
    int ret = stai_network_get_info(network_context, &info);
    assert(ret == STAI_SUCCESS);

    app_postprocess_init(&pp_params, &info);
}

void AI_Run(void)
{
    int ret = stai_network_run(network_context, STAI_MODE_SYNC);
    assert(ret == STAI_SUCCESS);
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
