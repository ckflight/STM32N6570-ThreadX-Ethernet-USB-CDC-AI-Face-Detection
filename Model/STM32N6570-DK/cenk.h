/**
  ******************************************************************************
  * @file    cenk.h
  * @author  STEdgeAI
  * @date    2026-10-06 22:19:40
  * @brief   Minimal description of the generated c-implemention of the network
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2025 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  ******************************************************************************
  */
#ifndef LL_ATON_CENK_H
#define LL_ATON_CENK_H

/******************************************************************************/
#define LL_ATON_CENK_C_MODEL_NAME        "cenk"
#define LL_ATON_CENK_ORIGIN_MODEL_NAME   "cenk_classifier_int8"

/************************** USER ALLOCATED IOs ********************************/
// No user allocated inputs
// No user allocated outputs

/************************** INPUTS ********************************************/
#define LL_ATON_CENK_IN_NUM        (1)    // Total number of input buffers
// Input buffer 1 -- Input_0_out_0
#define LL_ATON_CENK_IN_1_ALIGNMENT   (32)
#define LL_ATON_CENK_IN_1_SIZE_BYTES  (37632)

/************************** OUTPUTS *******************************************/
#define LL_ATON_CENK_OUT_NUM        (1)    // Total number of output buffers
// Output buffer 1 -- Quantize_36_out_0
#define LL_ATON_CENK_OUT_1_ALIGNMENT   (32)
#define LL_ATON_CENK_OUT_1_SIZE_BYTES  (1)

#endif /* LL_ATON_CENK_H */
