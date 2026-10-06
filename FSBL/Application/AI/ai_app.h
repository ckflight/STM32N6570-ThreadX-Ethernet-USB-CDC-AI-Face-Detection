#ifndef APP_AI_H
#define APP_AI_H

#include "stai.h"
#include "stai_network.h"
#include "app_postprocess.h"

// Second NN parameters
#define FACE_WIDTH 112
#define FACE_HEIGHT 112

extern stai_ptr nn_in;
extern stai_size number_output;
extern stai_ptr nn_out[STAI_NETWORK_OUT_NUM];

extern fd_blazeface_pp_static_param_t pp_params;
extern fd_pp_out_t pp_output;

extern uint8_t face_nn_in[FACE_WIDTH * FACE_HEIGHT * 3];

void AI_Init(void);
void AI_Run(void);
float Cenk_Run(void);
void Face_Crop(fd_pp_outBuffer_t *face);

#endif
