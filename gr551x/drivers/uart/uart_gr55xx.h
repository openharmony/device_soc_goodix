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

#ifndef UART_GR55XX_H
#define UART_GR55XX_H

#include "poll.h"
#include "uart_if.h"
#include "app_uart.h"
#include "app_io.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif /* __cplusplus */
#endif /* __cplusplus */

struct UartDriverData;
typedef int32_t (*app_uart_cfg_handler_t)(struct UartDriverData *udd);

#define DEFAULT_BAUDRATE        115200
#define DEFAULT_DATABITS        UART_ATTR_DATABIT_8
#define DEFAULT_STOPBITS        UART_ATTR_STOPBIT_1
#define DEFAULT_PARITY          UART_ATTR_PARITY_NONE
#define CONFIG_MAX_BAUDRATE     921600

#define TX_BUF_SIZE             0X100
struct UartDriverData {
    uint32_t id;
    uint32_t baudrate;
    int32_t count;
    struct UartAttribute attr;
    app_uart_params_t params;
    app_uart_evt_handler_t eventCallback;
    app_uart_cfg_handler_t config;
    app_uart_tx_buf_t txBuffer;
    int32_t state;
#define UART_STATE_NOT_OPENED 0
#define UART_STATE_OPENING    1
#define UART_STATE_USEABLE    2
#define UART_STATE_SUSPENED   3
    uint32_t flags;
#define UART_FLG_DMA_RX       (1 << 0)
#define UART_FLG_DMA_TX       (1 << 1)
#define UART_FLG_RD_BLOCK     (1 << 2)
};


#ifdef __cplusplus
#if __cplusplus
}
#endif /* __cplusplus */
#endif /* __cplusplus */

#endif /* UART_PL011_H */
