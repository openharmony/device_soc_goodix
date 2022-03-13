/**
 ****************************************************************************************
 *
 * @file    gr55xx_hal_def.h
 * @author  BLE Driver Team
 * @brief   This file contains HAL common definitions, enumeration, macros and structures definitions.
 *
 ****************************************************************************************
 * @attention
  #####Copyright (c) 2019 GOODIX
  All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are met:
  * Redistributions of source code must retain the above copyright
    notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright
    notice, this list of conditions and the following disclaimer in the
    documentation and/or other materials provided with the distribution.
  * Neither the name of GOODIX nor the names of its contributors may be used
    to endorse or promote products derived from this software without
    specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
  ARE DISCLAIMED. IN NO EVENT SHALL COPYRIGHT HOLDERS AND CONTRIBUTORS BE
  LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
  CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
  SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
  INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
  CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
  ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
  POSSIBILITY OF SUCH DAMAGE.
 ****************************************************************************************
 */

/** @addtogroup PERIPHERAL Peripheral Driver
  * @{
  */

/** @addtogroup HAL_DRIVER HAL Driver
  * @{
  */

/** @defgroup HAL_DEF HAL DEFINE
  * @brief HAL common definitions.
  * @{
  */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef GR55xx_HAL_DEF
#define GR55xx_HAL_DEF

/* Includes ------------------------------------------------------------------*/
#include <stdio.h>
#include "gr55xx.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Exported types ------------------------------------------------------------*/
/** @addtogroup HAL_ENUMERATIONS Enumerations
 * @{ */
/**
  * @brief  HAL Status structures definition
  */
typedef enum {
    HAL_OK       = 0x00U,    /**< Operation is OK. */
    HAL_ERROR    = 0x01U,    /**< Parameter error or operation is not supported. */
    HAL_BUSY     = 0x02U,    /**< Driver is busy. */
    HAL_TIMEOUT  = 0x03      /**< Timeout occurred. */
} hal_status_t;

/**
  * @brief  HAL Lock structures definition
  */
typedef enum {
    HAL_UNLOCKED = 0x00U,    /**< Object is unlocked. */
    HAL_LOCKED   = 0x01      /**< Object is locked. */
} hal_lock_t;
/** @} */

/**
  * @defgroup  HAL_DEF_MACRO Defines
  * @{
  */

/* Exported macro ------------------------------------------------------------*/
/**
  * @brief  HAL max delay definition
  */
#define HAL_MAX_DELAY                       (0xFFFFFFFFU)

/**
  * @brief  Check whether the bits of register are set.
  * @param  REG specifies the register.
  * @param  BIT specifies the bits will be checked.
  * @retval SET (BIT is set) or RESET (BIT is not set)
  */
#define HAL_IS_BIT_SET(REG, BIT)            (((REG) & (BIT)) != RESET)
/**
  * @brief  Check whether the bits of register are clear.
  * @param  REG specifies the register.
  * @param  BIT specifies the bits will be checked.
  * @retval SET (BIT is clear) or RESET (BIT is not clear)
  */
#define HAL_IS_BIT_CLR(REG, BIT)            (((REG) & (BIT)) == RESET)

/**
  * @brief  Link DMA handle and peripheral handle.
  * @param  __HANDLE__ specifies the peripheral handle.
  * @param  __PPP_DMA_FIELD_ specifies the DMA pointer in struction of peripheral handle.
  * @param  __DMA_HANDLE_ specifies the DMA handle.
  * @retval None
  */
#define HAL_LINKDMA(__HANDLE__, __PPP_DMA_FIELD_, __DMA_HANDLE_)                 \
do {                                                                               \
    (__HANDLE__)->__PPP_DMA_FIELD_ = &(__DMA_HANDLE_);                             \
    (__DMA_HANDLE_).p_parent = (__HANDLE__);                                       \
} while (0U)

/** @brief Reset the Handle's State field.
  * @param __HANDLE__ specifies the Peripheral Handle.
  * @retval None
  */
#define HAL_RESET_HANDLE_STATE(__HANDLE__) ((__HANDLE__)->state = 0U)

/**
  * @brief  Unlock peripheral handle.
  * @param  __HANDLE__ specifies the peripheral handle.
  * @retval None
  */
#define HAL_UNLOCK(__HANDLE__)                                            \
do {                                                                        \
    (__HANDLE__)->lock = HAL_UNLOCKED;                                      \
} while (0U)


#if defined ( __GNUC__ ) && !defined (__CC_ARM) /* GNU Compiler */
#ifndef __weak
#define __weak   __attribute__((weak))
#endif /* __weak */
#ifndef __packed
#define __packed __attribute__((__packed__))
#endif /* __packed */
#endif /* __GNUC__ */


/** @} */

#ifdef __cplusplus
}
#endif

#endif /* ___GR55xx_HAL_DEF__ */
/** @} */

/** @} */

/** @} */
