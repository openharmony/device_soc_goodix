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

#include "iot_errno.h"
#include "iot_watchdog.h"
#include "gr55xx_hal_aon_wdt.h"

#define WDT_TIMEOUT_INTVAL   2

static aon_wdt_handle_t g_aon_wdt_handle;

#define AON_WATCHDOG_ERLOAD_TIME            10      /* 10s reload watchdog */
#define AON_WATCHDOG_1S_RELOAD_VALUE        32768
#define AON_WATCHDOG_ALARM_VALUE            0xFF

void IoTWatchDogEnable(void)
{
    /* AON_WDT use SystemCoreLowClock = 32.768KHz */
    g_aon_wdt_handle.init.counter       = AON_WATCHDOG_1S_RELOAD_VALUE * AON_WATCHDOG_ERLOAD_TIME;
    g_aon_wdt_handle.init.alarm_counter = AON_WATCHDOG_ALARM_VALUE;

    hal_aon_wdt_init(&g_aon_wdt_handle);
    return IOT_SUCCESS;
}

void IoTWatchDogKick(void)
{
    hal_aon_wdt_refresh(&g_aon_wdt_handle);
    return IOT_SUCCESS;
}

void IoTWatchDogDisable(void)
{
    hal_aon_wdt_deinit(&g_aon_wdt_handle);    
    return IOT_SUCCESS;
}

