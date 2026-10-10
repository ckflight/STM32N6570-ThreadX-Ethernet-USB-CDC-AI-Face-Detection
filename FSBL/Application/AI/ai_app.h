#ifndef APP_AI_H
#define APP_AI_H

#include "stai.h"
#include "stai_network.h"
#include "app_postprocess.h"
#include "stai_cenk.h"
#include "stai_reid.h"

extern stai_ptr 	nn_in;
extern stai_size 	number_output;
extern stai_ptr 	nn_out[STAI_NETWORK_OUT_NUM];

extern fd_blazeface_pp_static_param_t pp_params;
extern fd_pp_out_t pp_output;


extern stai_ptr cenk_in;


extern stai_ptr reid_in;
extern stai_ptr reid_out[STAI_REID_OUT_NUM];


void AI_Init(void);

void AI_Run(void);
float Cenk_Run(void);
int ReID_Run(void);

void Face_Crop(fd_pp_outBuffer_t *face);
void Face_Crop2(fd_pp_outBuffer_t *face);

void npu_cache_enable_clocks_and_reset(void);
void npu_cache_disable_clocks_and_reset(void);

#endif
