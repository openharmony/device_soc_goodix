/**
 *****************************************************************************************
 *
 * @file pmu_calibration.c
 *
 * @brief auto calibration function Implementation.
 *
 *****************************************************************************************
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
 *****************************************************************************************
 */

/*
 * INCLUDE FILES
 *****************************************************************************************
 */
#include "pmu_calibration.h"
#include "platform_sdk.h"

#define INTERVAL_MIN 2000
#define STACKDEPTH   512
/*
 * LOCAL VARIABLE DEFINITIONS
 *****************************************************************************************
 */
#ifdef ENV_USE_FREERTOS
static TimerHandle_t timer_handle = NULL;
static void system_pmu_calibration_task(void *p_arg);
#else
static app_timer_id_t s_pmu_calibration_timer_id = 0;
#endif

#if CFG_LPCLK_INTERNAL_EN
#error "INTERNAL LPCLK is not supported by OHOS"
#endif

/*
 * GLOBAL FUNCTION DEFINITIONS
 *****************************************************************************************
 */
void system_pmu_calibration_init(uint32_t interval)
{
    if (interval) {
        uint32_t interval_new;
#if CFG_LPCLK_INTERNAL_EN
        pmu_interval_init = interval;
        interval_new = pmu_interval_get(1);
#else
        interval_new = interval;
#endif

#ifdef ENV_USE_FREERTOS
        timer_handle = xTimerCreate(NULL, interval_new, pdTRUE, NULL,
#if CFG_LPCLK_INTERNAL_EN
                                    pmu_timer_handler
#else
                                    pmu_calibration_handler
#endif // CFG_LPCLK_INTERNAL_EN
                                   );

        xTaskCreate(system_pmu_calibration_task, "pmu_calibration_task",
                    STACKDEPTH, NULL, configMAX_PRIORITIES - 1, NULL);
#else
        app_timer_delete(&s_pmu_calibration_timer_id);
        app_timer_create(&s_pmu_calibration_timer_id, ATIMER_REPEAT,
#if CFG_LPCLK_INTERNAL_EN
                         pmu_timer_handler
#else
                         pmu_calibration_handler
#endif  // CFG_LPCLK_INTERNAL_EN
                        );
        app_timer_start(s_pmu_calibration_timer_id, interval_new, NULL);
#endif  // ENV_USE_FREERTOS

#if CFG_LPCLK_INTERNAL_EN
        pmu_interval_prev = interval_new;
#endif
    }
    return;
}

#ifdef ENV_USE_FREERTOS
void system_pmu_calibration_start(void)
{
    if (timer_handle != NULL) {
        if (xTimerIsTimerActive(timer_handle) == pdFALSE) {
            xTimerStart(timer_handle, 0);
        }
    }

    return;
}

static void system_pmu_calibration_task(void *p_arg)
{
    system_pmu_calibration_start();
    vTaskDelete(NULL);
}
#endif

void system_pmu_calibration_stop(void)
{
#ifdef ENV_USE_FREERTOS
    if (timer_handle != NULL) {
        xTimerDelete(timer_handle, 0);
        timer_handle = NULL;
    }
#else
    app_timer_delete(&s_pmu_calibration_timer_id);
#endif
    return;
}

