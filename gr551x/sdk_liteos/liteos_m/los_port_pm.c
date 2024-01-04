/*
 * Copyright (c) 2021 GOODIX.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "gr55xx.h"
#include "gr55xx_pwr.h"
#include "gr55xx_sys.h"

#include "gr55xx_ll_pwr.h"

#include "los_pm.h"
#include "los_timer.h"
#include "los_tick.h"
#include "los_task.h"
#include "los_sched.h"

#include "los_port_pm.h"

#define TICK_MS_IN_HUS         (2000)
#define SYS_BLE_SLEEP_ALGO_HUS (580)
#define SLP_WAKUP_ALGO_LP_CNT  (32)
#define DEEPSLEEP_TIME_MIN_MS  (5)

static uint64_t g_tickTimerBaseBeforeSleep = 0;
static uint32_t g_lpCntWhenTickStop = 0;
static uint32_t g_lpCntWhenTickReload = 0;

TINY_RAM_SECTION uint32_t os_sleep_ms_get(void)
{
    g_tickTimerBaseBeforeSleep = OsGetCurrSchedTimeCycle();
    return ((uint32_t)(OsSchedGetNextExpireTime(g_tickTimerBaseBeforeSleep) - g_tickTimerBaseBeforeSleep)) /
           OS_CYCLE_PER_TICK;
}

TINY_RAM_SECTION void sys_tick_reload(void)
{
    SysTick->CTRL &= ~SysTick_CTRL_ENABLE_Msk;
    SysTick->LOAD = (UINT32)((OS_SYS_CLOCK / LOSCFG_BASE_CORE_TICK_PER_SECOND) - 1UL);
    SysTick->VAL = 0UL;
    SysTick->CTRL |= (SysTick_CTRL_ENABLE_Msk | SysTick_CTRL_CLKSOURCE_Msk | SysTick_CTRL_TICKINT_Msk);
}

TINY_RAM_SECTION static void pwr_mgmt_sleep_dur_limit(uint32_t sleepMs)
{
    uint32_t sleepHus = sleepMs * TICK_MS_IN_HUS - SYS_BLE_SLEEP_ALGO_HUS;
    if (get_remain_sleep_dur() > sleepHus) {
        pwr_mgmt_ble_wakeup();
    }
    sys_ble_heartbeat_period_set(sleepHus);
}

TINY_RAM_SECTION static void pwr_mgmt_enter_sleep_with_cond(uint32_t sleepMs)
{
    pwr_mgmt_sleep_dur_limit(sleepMs);

    uint32_t intSave = LOS_IntLock();

    if (DEVICE_BUSY == pwr_mgmt_dev_suspend()) {
        ultra_wfi();
        LOS_IntRestore(intSave);
        return;
    }

    if (PMR_MGMT_SLEEP_MODE != pwr_mgmt_mode_get()) {
        LOS_IntRestore(intSave);
        return;
    }

    pwr_mgmt_mode_t bleState = pwr_mgmt_baseband_state_get();
    switch (bleState) {
        case PMR_MGMT_IDLE_MODE:
            ultra_wfi();
        case PMR_MGMT_ACTIVE_MODE:
            LOS_IntRestore(intSave);
            return;
    }

    g_lpCntWhenTickStop = ll_pwr_get_comm_sleep_duration();

    pwr_mgmt_save_context();

    if (pwr_mgmt_get_wakeup_flag() == COLD_BOOT) {
        if (PMR_MGMT_IDLE_MODE == pwr_mgmt_shutdown()) {
            ultra_wfi();
            LOS_IntRestore(intSave);
            return;
        }
        LOS_IntRestore(intSave);
    } else {
        pwr_mgmt_set_wakeup_flag(COLD_BOOT);

        uint32_t intSaveLocal = LOS_IntLock();

        g_lpCntWhenTickReload = ll_pwr_get_comm_sleep_duration();
        sys_tick_reload();

        uint32_t sleepLpCycles = g_lpCntWhenTickReload - g_lpCntWhenTickStop + SLP_WAKUP_ALGO_LP_CNT;
        uint32_t lpCycles2HusErr = 0;
        uint32_t sleepHus = sys_lpcycles_2_hus(sleepLpCycles, &lpCycles2HusErr);
        uint32_t sleepSystick = sleepHus * (OS_CYCLE_PER_TICK / TICK_MS_IN_HUS);

        OsTickTimerBaseReset(g_tickTimerBaseBeforeSleep + sleepSystick);
        LOS_SchedTickHandler();

        LOS_IntRestore(intSaveLocal);
        warm_boot_second();
    }
}

TINY_RAM_SECTION static void os_pm_enter_handler(void)
{
    uint32_t intSave = LOS_IntLock();

    if ((SCB->ICSR & SCB_ICSR_PENDSTSET_Msk)) {
        LOS_IntRestore(intSave);
        return;
    }

    uint32_t sleepMs = os_sleep_ms_get();
    LOS_IntRestore(intSave);

    if (sleepMs < DEEPSLEEP_TIME_MIN_MS) {
        ultra_wfi();
        return;
    }

    if (PMR_MGMT_SLEEP_MODE != pwr_mgmt_mode_get()) {
        ultra_wfi();
        return;
    }

    LOS_TaskLock();
    pwr_mgmt_enter_sleep_with_cond(sleepMs);
    LOS_TaskUnlock();
}

void GR551xPwrMgmtInit(void)
{
    OsPmEnterHandlerSet(os_pm_enter_handler);
}
