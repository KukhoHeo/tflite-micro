/*
 * SPDX-FileCopyrightText: Copyright 2010-2022 Arm Limited and/or its affiliates <open-source-office@arm.com>
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Licensed under the Apache License, Version 2.0 (the License); you may
 * not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an AS IS BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/* ----------------------------------------------------------------------
 * Project:      CMSIS NN Library
 * Title:        arm_fully_connected_mat_q7_vec_q15.c
 * Description:  Mixed Q15-Q7 fully-connected layer function
 *
 * $Date:        4 Aug 2022
 * $Revision:    V.2.0.1
 *
 * Target Processor:  Cortex-M cores
 *
 * -------------------------------------------------------------------- */

#include "arm_nnfunctions.h"
#include "arm_nnsupportfunctions.h"

/**
 *  @ingroup groupNN
 */

/**
 * @addtogroup FC
 * @{
 */

/*
 * @brief Mixed Q15-Q7 fully-connected layer function
 * Refer function header for details
 */

arm_cmsis_nn_status arm_fully_connected_mat_q7_vec_q15(const q15_t *pV,
                                                       const q7_t *pM,
                                                       const uint16_t dim_vec,
                                                       const uint16_t num_of_rows,
                                                       const uint16_t bias_shift,
                                                       const uint16_t out_shift,
                                                       const q7_t *bias,
                                                       q15_t *pOut,
                                                       q15_t *vec_buffer)
{
    (void)vec_buffer;
#if defined(ARM_MATH_DSP) && !defined(ARM_MATH_MVEI)
    /* Run the following code for Cortex-M4 and Cortex-M7 */

    const q7_t *pB = pM;
    const q7_t *pB2;
    q15_t *pO = pOut;
    const q7_t *pBias = bias;
    const q15_t *pA = pV;

    uint16_t rowCnt = num_of_rows >> 1;

    while (rowCnt)
    {
        q31_t sum = ((q31_t)(*pBias++) << bias_shift) + NN_ROUND(out_shift);
        q31_t sum2 = ((q31_t)(*pBias++) << bias_shift) + NN_ROUND(out_shift);
        uint16_t colCnt = dim_vec >> 2;

        pA = pV;
        pB2 = pB + dim_vec;

        while (colCnt)
        {
            q31_t inV, inM11, inM12, inM21, inM22;
            pB = read_and_pad(pB, &inM11, &inM12);
            pB2 = read_and_pad(pB2, &inM21, &inM22);

            inV = arm_nn_read_q15x2_ia(&pA);

            sum = __SMLAD(inV, inM11, sum);
            sum2 = __SMLAD(inV, inM21, sum2);

            inV = arm_nn_read_q15x2_ia(&pA);

            sum = __SMLAD(inV, inM12, sum);
            sum2 = __SMLAD(inV, inM22, sum2);

            colCnt--;
        }
        colCnt = dim_vec & 0x3;
        while (colCnt)
        {
            q15_t inV = *pA++;
            q7_t inM = *pB++;
            q7_t inM2 = *pB2++;

            sum += inV * inM;
            sum2 += inV * inM2;
            colCnt--;
        } /* while over colCnt */
        *pO++ = (q15_t)(__SSAT((sum >> out_shift), 16));
        *pO++ = (q15_t)(__SSAT((sum2 >> out_shift), 16));

        /*adjust the pointers and counters */
        pB += dim_vec;
        rowCnt--;
    }

    /* left-over part of the rows */
    rowCnt = num_of_rows & 0x1;

    while (rowCnt)
    {
        q31_t sum = ((q31_t)(*pBias++) << bias_shift) + NN_ROUND(out_shift);
        uint16_t colCnt = dim_vec >> 2;

        pA = pV;

        while (colCnt)
        {
            q31_t inV1, inV2, inM11, inM12;

            pB = read_and_pad(pB, &inM11, &inM12);

            inV1 = arm_nn_read_q15x2_ia(&pA);
            sum = __SMLAD(inV1, inM11, sum);

            inV2 = arm_nn_read_q15x2_ia(&pA);
            sum = __SMLAD(inV2, inM12, sum);

            colCnt--;
        }

        /* left-over of the vector */
        colCnt = dim_vec & 0x3;
        while (colCnt)
        {
            q15_t inV = *pA++;
            q7_t inM = *pB++;
            sum += inV * inM;
            colCnt--;
        }

        *pO++ = (q15_t)(__SSAT((sum >> out_shift), 16));

        rowCnt--;
    }

#else
    int i, j;
    /* Run the following code as reference implementation for Cortex-M0 and Cortex-M3 */
    for (i = 0; i < num_of_rows; i++)
    {
        int ip_out = ((q31_t)(bias[i]) << bias_shift) + NN_ROUND(out_shift);
        for (j = 0; j < dim_vec; j++)
        {
            ip_out += pV[j] * pM[i * dim_vec + j];
        }
        pOut[i] = (q15_t)__SSAT((ip_out >> out_shift), 16);
    }

#endif /* ARM_MATH_DSP */

    /* Return to ARM_CMSIS_NN_SUCCESS */
    return (ARM_CMSIS_NN_SUCCESS);
}

/**
 * @} end of FC group
 */
