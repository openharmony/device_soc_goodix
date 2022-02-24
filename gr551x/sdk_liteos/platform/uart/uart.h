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

#ifndef __UART_H
#define __UART_H

#include "los_event.h"
#include "los_compiler.h"
#include "app_io.h"
#include "app_uart.h"

#ifdef __cplusplus
extern "C" {
#endif

#define LOG_UART_ID                     APP_UART_ID_0
#define LOG_UART_BAUDRATE               115200
#define LOG_UART_TX_IO_TYPE             APP_IO_TYPE_NORMAL
#define LOG_UART_RX_IO_TYPE             APP_IO_TYPE_NORMAL
#define LOG_UART_TX_PIN                 APP_IO_PIN_10
#define LOG_UART_RX_PIN                 APP_IO_PIN_11
#define LOG_UART_TX_PINMUX              APP_IO_MUX_2
#define LOG_UART_RX_PINMUX              APP_IO_MUX_2
#define LOG_UART_TX_PULL                APP_IO_PULLUP
#define LOG_UART_RX_PULL                APP_IO_PULLUP

/**< Size of app uart tx buffer. */
#define UART_TX_BUFF_SIZE               0x2000

extern EVENT_CB_S g_shellInputEvent;

void bsp_log_init(void);
uint8_t UartGetc(void);
void _putchar(char character);

#ifdef __cplusplus
}
#endif

#endif /* __UART_H */
