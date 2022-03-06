/**
  ****************************************************************************************
  * @file    app_dma.c
  * @author  BLE Driver Team
  * @brief   HAL APP module driver.
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

/*
 * INCLUDE FILES
 *****************************************************************************************
 */
#include <string.h>
#include <stdbool.h>
#include "app_pwr_mgmt.h"
#include "platform_sdk.h"
#include "app_dma.h"

/*
 * DEFINES
 *****************************************************************************************
 */
#define DMA_HANDLE_MAX            8

/*
 * STRUCT DEFINE
 *****************************************************************************************
 */

/**@brief App dma state types. */
typedef enum {
    APP_DMA_INVALID = 0,
    APP_DMA_ENABLE,
#ifdef APP_DRIVER_WAKEUP_CALL_FUN
    APP_DMA_SLEEP,
#endif
} app_dma_state_t;

struct dma_env_t {
    app_dma_state_t       dma_state;
    dma_handle_t          handle;
    app_dma_evt_handler_t evt_handler;
};

/*
 * LOCAL FUNCTION DECLARATION
 *****************************************************************************************
 */
static bool dma_prepare_for_sleep(void);
static void dma_sleep_canceled(void);
static void dma_wake_up_ind(void);

/*
 * LOCAL VARIABLE DEFINITIONS
 *****************************************************************************************
 */
static bool             s_sleep_cb_registered_flag = false;
static struct dma_env_t s_dma_env[DMA_HANDLE_MAX];
static int16_t          s_dma_pwr_id;

static const app_sleep_callbacks_t dma_sleep_cb = {
    .app_prepare_for_sleep  = dma_prepare_for_sleep,
    .app_sleep_canceled     = dma_sleep_canceled,
    .app_wake_up_ind        = dma_wake_up_ind,
};

/*
 * LOCAL FUNCTION DEFINITIONS
 *****************************************************************************************
 */
static bool dma_prepare_for_sleep(void)
{
    hal_dma_state_t state;

    for (uint8_t i = 0; i < DMA_HANDLE_MAX; i++) {
        if (s_dma_env[i].dma_state == APP_DMA_ENABLE) {
            state = hal_dma_get_state(&s_dma_env[i].handle);
            if ((state != HAL_DMA_STATE_RESET) && (state != HAL_DMA_STATE_READY)) {
                return false;
            }
            hal_dma_suspend_reg(&s_dma_env[i].handle);
#ifdef APP_DRIVER_WAKEUP_CALL_FUN
            s_dma_env[i].dma_state = APP_DMA_SLEEP;
#endif
        }
    }

    return true;
}

static void dma_sleep_canceled(void)
{
}

SECTION_RAM_CODE static void dma_wake_up_ind(void)
{
#ifndef APP_DRIVER_WAKEUP_CALL_FUN
    bool find = false;

    for (uint8_t i = 0; i < DMA_HANDLE_MAX; i++) {
        if (s_dma_env[i].dma_state == APP_DMA_ENABLE) {
            hal_dma_resume_reg(&s_dma_env[i].handle);
            find = true;
        }
    }

    if (find) {
        hal_nvic_clear_pending_irq(DMA_IRQn);
        hal_nvic_enable_irq(DMA_IRQn);
    }
#endif
}

#ifdef APP_DRIVER_WAKEUP_CALL_FUN
void dma_wake_up(int16_t id)
{
    if (id<0 || id >= DMA_HANDLE_MAX) {
        return;
    }

    if (s_dma_env[id].dma_state == APP_DMA_SLEEP) {
        hal_dma_resume_reg(&s_dma_env[id].handle);
        s_dma_env[id].dma_state = APP_DMA_ENABLE;

        if (!NVIC_GetEnableIRQ(DMA_IRQn)) {
            hal_nvic_clear_pending_irq(DMA_IRQn);
            hal_nvic_enable_irq(DMA_IRQn);
        }
    }
}
#endif

/*
 * GLOBAL FUNCTION DEFINITIONS
 ****************************************************************************************
 */
void dma_tfr_callback(struct _dma_handle *hdma)
{
    uint8_t i;

    for (i = 0; i < DMA_HANDLE_MAX; i++) {
        if ((s_dma_env[i].dma_state == APP_DMA_ENABLE) &&
                (s_dma_env[i].handle.channel == hdma->channel)) {
            if (s_dma_env[i].evt_handler != NULL) {
                s_dma_env[i].evt_handler(APP_DMA_EVT_TFR);
            }
            break;
        }
    }
}

void dma_err_callback(struct _dma_handle * hdma)
{
    uint8_t i;

    for (i = 0; i < DMA_HANDLE_MAX; i++) {
        if ((s_dma_env[i].dma_state == APP_DMA_ENABLE) &&
                (s_dma_env[i].handle.channel == hdma->channel)) {
            if (s_dma_env[i].evt_handler != NULL) {
                s_dma_env[i].evt_handler(APP_DMA_EVT_ERROR);
            }
            break;
        }
    }
}


static void dma_handle_config(uint8_t *p_i, int16_t *p_id, app_dma_params_t *p_params)
{
    GLOBAL_EXCEPTION_DISABLE();
    for (*p_i = 0; (*p_i) < DMA_HANDLE_MAX; (*p_i)++) {
        if (s_dma_env[*p_i].dma_state == APP_DMA_INVALID || \
            s_dma_env[*p_i].handle.channel == p_params->channel_number) {
            if (HAL_DMA_STATE_BUSY == s_dma_env[*p_i].handle.state) {
                *p_i = DMA_HANDLE_MAX;
                break;
            } else {
                *p_id = *p_i;
                s_dma_env[*p_i].dma_state = APP_DMA_ENABLE;
                break;
            }
        }
    }
    GLOBAL_EXCEPTION_ENABLE();
}

int16_t app_dma_init(app_dma_params_t *p_params, app_dma_evt_handler_t evt_handler)
{
    uint8_t      i  = 0;
    int16_t     id = -1;
    hal_status_t status = HAL_ERROR;

    if (p_params != NULL) {
        if (!IS_DMA_ALL_INSTANCE(p_params->channel_number)) {
            return -1;
        }

        dma_handle_config(&i, &id, p_params);

        if (i < DMA_HANDLE_MAX) {
            if (s_sleep_cb_registered_flag == false) { // register sleep callback
                s_sleep_cb_registered_flag = true;
                s_dma_pwr_id = pwr_register_sleep_cb(&dma_sleep_cb, APP_DRIVER_DMA_WAPEUP_PRIORITY);
            }
            s_dma_env[i].handle.channel = p_params->channel_number;
            memcpy_s(&s_dma_env[i].handle.init, sizeof(s_dma_env[i].handle.init), &p_params->init, sizeof(dma_init_t));
            s_dma_env[i].handle.xfer_tfr_callback   = dma_tfr_callback;
            s_dma_env[i].handle.xfer_error_callback = dma_err_callback;
            s_dma_env[i].handle.xfer_abort_callback = NULL;
            s_dma_env[i].evt_handler = evt_handler;

            hal_nvic_clear_pending_irq(DMA_IRQn);
            hal_nvic_enable_irq(DMA_IRQn);
            status = hal_dma_init(&s_dma_env[i].handle);
        }
    }

    if (HAL_OK != status) {
        id = -1;
    }

    return id;
}


uint16_t app_dma_deinit(int16_t id)
{
    uint8_t i;

    if ((id < 0) || (id >= DMA_HANDLE_MAX) || (s_dma_env[id].dma_state == APP_DMA_INVALID)) {
        return APP_DRV_ERR_INVALID_ID;
    }

    GLOBAL_EXCEPTION_DISABLE();
    hal_dma_deinit(&s_dma_env[id].handle);
    s_dma_env[id].dma_state = APP_DMA_INVALID;
    s_dma_env[id].handle.channel = (dma_channel_t)(-1);

    for (i = 0; i < DMA_HANDLE_MAX; i++) {
        if (s_dma_env[i].dma_state == APP_DMA_ENABLE) {
            break;
        }
    }

    if (i == DMA_HANDLE_MAX) {
        pwr_unregister_sleep_cb(s_dma_pwr_id);
        s_sleep_cb_registered_flag = false;
        hal_nvic_disable_irq(DMA_IRQn);
    }
    GLOBAL_EXCEPTION_ENABLE();

    return APP_DRV_SUCCESS;
}

dma_handle_t *app_dma_get_handle(int16_t id)
{
    if (id < 0 || id >= DMA_HANDLE_MAX || s_dma_env[id].dma_state == APP_DMA_INVALID) {
        return NULL;
    }

#ifdef APP_DRIVER_WAKEUP_CALL_FUN
    dma_wake_up(id);
#endif

    return &s_dma_env[id].handle;
}

uint16_t app_dma_start(int16_t id, uint32_t src_address, uint32_t dst_address, uint32_t data_length)
{
    hal_status_t status = HAL_ERROR;

    if (id < 0 || id >= DMA_HANDLE_MAX || s_dma_env[id].dma_state == APP_DMA_INVALID) {
        return APP_DRV_ERR_INVALID_PARAM;
    }

#ifdef APP_DRIVER_WAKEUP_CALL_FUN
    dma_wake_up(id);
#endif

    status = hal_dma_start_it(&s_dma_env[id].handle, src_address, dst_address, data_length);
    if (HAL_OK != status) {
        return (uint16_t)status;
    }

    return APP_DRV_SUCCESS;
}

SECTION_RAM_CODE void DMA_IRQHandler(void)
{
#if FLASH_PROTECT_PRIORITY
    platform_interrupt_protection_push();
#endif
    uint8_t i;
    for (i = 0; i < DMA_HANDLE_MAX; i++) {
        if (s_dma_env[i].dma_state == APP_DMA_ENABLE) {
            hal_dma_irq_handler(&s_dma_env[i].handle);
        }
    }
#if FLASH_PROTECT_PRIORITY
    platform_interrupt_protection_pop();
#endif
}

