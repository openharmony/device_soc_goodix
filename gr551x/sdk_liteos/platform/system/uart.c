/*
 * Copyright (c) 2024 GOODIX.
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

#include "uart.h"
#include "board_SK.h"
#include "los_sem.h"
#include "custom_config.h"

#if LOSCFG_USE_SHELL

#define SHELL_UART_ID APP_UART_ID

static UINT32 s_rx_sem = 0xFFFFFFFFUL;

void app_uart_evt_handler(app_uart_evt_t *p_evt)
{
    if (p_evt->type == APP_UART_EVT_RX_DATA || p_evt->type == APP_UART_EVT_ERROR)
    {
        LOS_SemPost(s_rx_sem);
        LOS_EventWrite(&g_shellInputEvent, 0x1);
    }
}

uint8_t UartGetc(void)
{
    if (s_rx_sem == 0xFFFFFFFFUL)
    {
        LOS_BinarySemCreate(0, &s_rx_sem);
    }

    uint8_t ch = 0;

    app_uart_receive_async(SHELL_UART_ID, &ch, 1);
    LOS_SemPend(s_rx_sem, LOS_WAIT_FOREVER);

    return ch;
}

#endif // LOSCFG_USE_SHELL