/**
 ****************************************************************************************
 *
 * @file scatter_common.h
 *
 * @brief decalare the symbols in scatter_common.sct.
 *
 *
 ****************************************************************************************
 */
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

#ifndef __SCATTER_COMMON_H__
#define __SCATTER_COMMON_H__
#include <stdint.h>
#include <custom_config.h>
#include "flash_scatter_config.h"

#if (0 == CONN_BUF_SIZE)
#define STACK_HEAP_INIT(heaps_table)                                            \
uint8_t env_heap_buf[ENV_HEAP_SIZE] __attribute__((aligned (32))) = {0};        \
uint8_t att_db_heap_buf[ATT_DB_HEAP_SIZE] __attribute__((aligned (32))) = {0};  \
uint8_t ke_msg_heap_buf[KE_MSG_HEAP_SIZE] __attribute__((aligned (32))) = {0};  \
uint8_t non_ret_heap_buf[NON_RET_HEAP_SIZE]__attribute__((aligned (32))) = {0}; \
stack_heaps_table_t heaps_table = { (uint32_t *)env_heap_buf,                   \
                                    (uint32_t *)att_db_heap_buf,                \
                                    (uint32_t *)ke_msg_heap_buf,                \
                                    (uint32_t *)non_ret_heap_buf,               \
                                    ENV_HEAP_SIZE,                              \
                                    ATT_DB_HEAP_SIZE,                           \
                                    KE_MSG_HEAP_SIZE,                           \
                                    NON_RET_HEAP_SIZE,                          \
                                    (uint8_t *)NULL,                            \
                                    0,                                          \
                                    (uint8_t *)NULL,                            \
                                    0,                                          \
                                    (uint8_t *)NULL,                            \
                                    0}
#else
#define STACK_HEAP_INIT(heaps_table)                                            \
uint8_t prf_buf[PRF_BUF_SIZE] __attribute__((aligned (32))) = {0};              \
uint8_t bond_buf[BOND_BUF_SIZE] __attribute__((aligned (32))) = {0};            \
uint8_t conn_buf[CONN_BUF_SIZE] __attribute__((aligned (32))) = {0};            \
uint8_t env_heap_buf[ENV_HEAP_SIZE] __attribute__((aligned (32))) = {0};        \
uint8_t att_db_heap_buf[ATT_DB_HEAP_SIZE] __attribute__((aligned (32))) = {0};  \
uint8_t ke_msg_heap_buf[KE_MSG_HEAP_SIZE] __attribute__((aligned (32))) = {0};  \
uint8_t non_ret_heap_buf[NON_RET_HEAP_SIZE]__attribute__((aligned (32))) = {0}; \
stack_heaps_table_t heaps_table = { (uint32_t *)env_heap_buf,                   \
                                    (uint32_t *)att_db_heap_buf,                \
                                    (uint32_t *)ke_msg_heap_buf,                \
                                    (uint32_t *)non_ret_heap_buf,               \
                                    ENV_HEAP_SIZE,                              \
                                    ATT_DB_HEAP_SIZE,                           \
                                    KE_MSG_HEAP_SIZE,                           \
                                    NON_RET_HEAP_SIZE,                          \
                                    (uint8_t *)prf_buf,                         \
                                    PRF_BUF_SIZE,                               \
                                    (uint8_t *)bond_buf,                        \
                                    BOND_BUF_SIZE,                              \
                                    (uint8_t *)conn_buf,                        \
                                    CONN_BUF_SIZE}
#endif // (0 == CONN_BUF_SIZE)
#endif // __SCATTER_COMMON_H__
