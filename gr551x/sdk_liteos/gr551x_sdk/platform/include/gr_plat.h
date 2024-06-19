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

#ifndef GR_PLAT_H
#define GR_PLAT_H

#include <stdint.h>

#define MAX_COMMENTS_CNT        12
#define MAX_RESERVED_1_CNT      6
typedef struct __attribute((packed))
{
    uint32_t    app_pattern;
    uint32_t    app_info_version;
    uint32_t    chip_ver;
    uint32_t    load_addr;
    uint32_t    run_addr;
    uint32_t    app_info_sum;
    uint8_t     check_img;
    uint8_t     boot_delay;
    uint8_t     sec_cfg;
    uint8_t     reserved0;
    uint8_t     comments[MAX_COMMENTS_CNT];
    uint32_t    reserved1[MAX_RESERVED_1_CNT];
} APP_INFO_t;

#define APP_INFO_ADDR           (APP_CODE_RUN_ADDR+0x200)
#define APP_INFO_PATTERN_VALUE  0x47525858
#define APP_INFO_VERSION        0x1
#define CHECK_SUM               (APP_INFO_PATTERN_VALUE+APP_INFO_VERSION+CHIP_VER+APP_CODE_LOAD_ADDR+APP_CODE_RUN_ADDR)

extern void __main(void);

extern void sdk_init(void);

#endif
