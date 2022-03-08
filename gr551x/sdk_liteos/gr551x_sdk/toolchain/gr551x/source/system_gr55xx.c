/**************************************************************************//**
 * @file     system_gr55xx.c
 * @brief    CMSIS Device System Source File for
 *           Device GR55xx
 * @version  V1.00
 * @date     12. June 2018
 ******************************************************************************/
/*
 * Copyright (c) 2018 GOODIX. All rights reserved.
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Licensed under the Apache License, Version 2.0 (the License); you may
 * not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an AS IS BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <stdint.h>
#include "gr55xx.h"
#include "gr55xx_sys.h"
#include "gr55xx_hal.h"
#include "platform_sdk.h"
#include "custom_config.h"
#include "gr55xx_rom_symbol.h"

/*----------------------------------------------------------------------------
  WEAK Functions
 *----------------------------------------------------------------------------*/
__WEAK void sdk_init(void)
{
    /* Prevent unused argument(s) compilation warning */
    return;
}
__WEAK void rom_init(void)
{
    /* Prevent unused argument(s) compilation warning */
    return;
}

/*----------------------------------------------------------------------------
  Define clocks
 *----------------------------------------------------------------------------*/
#define SOFTWARE_REG_WAKEUP_FLAG_POS   (8)
static inline void REG_PL_WR(uint32_t addr, uint32_t value)
{
    (*((volatile uint32_t *)(addr))) = (value);
}
static inline uint32_t REG_PL_RD(uint32_t addr)
{
    return (*((volatile uint32_t *)(addr)));
}

#define SCB_CPACR_BASE_NUM           3UL
static inline uint32_t READ_VERSION_ADDR(void)
{
    return REG_PL_RD(0x45004);
}
#define CALIB_LP_CYCLE_COUNT           20

#define REGION_TABLE_LIMIT           0x0007e9c0
#define REGION_TABLE_BASE            0x0007e990
#define SCATTERLOAD_COPY             0x00062d05
#define SCATTERLOAD_ZEROINIT         0x00062d21
#define DFU_DATA_START_ADDR         (0x800000 + 0x4000)

typedef struct {
    uint32_t rom_addr;
    uint32_t ram_addr;
    uint32_t len;
    uint32_t fun;
} sactter_copy_info_t;

volatile uint32_t g_app_msp_addr;   /* record app msp address */

static const uint32_t systemClock[CLK_TYPE_NUM] = {
    CLK_64M, /* CPLL_S64M_CLK */
    CLK_48M, /* CPLL_F48M_CLK */
    CLK_16M, /* XO_S16M_CLK */
    CLK_24M, /* CPLL_T24M_CLK */
    CLK_16M, /* CPLL_S16M_CLK */
    CLK_32M, /* CPLL_T32M_CLK */
};

// xqspi clock table by sys_clk_type
const uint32_t mcu_clk_2_qspi_clk[CLK_TYPE_NUM] = {
    [CPLL_S64M_CLK] = QSPI_64M_CLK,
    [CPLL_F48M_CLK] = QSPI_48M_CLK,
    [CPLL_T32M_CLK] = QSPI_32M_CLK,
    [CPLL_T24M_CLK] = QSPI_24M_CLK,
    [CPLL_S16M_CLK] = QSPI_16M_CLK,
    [XO_S16M_CLK] = QSPI_16M_CLK,
};

/*----------------------------------------------------------------------------
  System Core Clock Variable
 *----------------------------------------------------------------------------*/
uint32_t SystemCoreClock = CLK_64M;  /* System Core Clock Frequency as 64Mhz     */

// lint -e{2,10,48,63}
// The previous line of comment is to inhibit PC-Lint errors for next code block.
void SystemCoreSetClock(mcu_clock_type_t clock_type)
{
    if (clock_type >= CLK_TYPE_NUM)
        return;        // input parameter is out of range

    if ((AON->PWR_RET01 & AON_PWR_REG01_SYS_CLK_SEL) != clock_type) {
        uint32_t temp = AON->PWR_RET01 & (~(AON_PWR_REG01_SYS_CLK_SEL | AON_PWR_REG01_XF_SCK_CLK_SEL));
        // When a 16M or 64M clock is switched to another clock, it needs to be switched to 32M first.
        AON->PWR_RET01 = (temp | (CPLL_T32M_CLK << AON_PWR_REG01_SYS_CLK_SEL_Pos) |
                          (QSPI_32M_CLK << AON_PWR_REG01_XF_SCK_CLK_SEL_Pos));

        __asm ("nop;nop;nop;nop;");
        temp = AON->PWR_RET01 & (~(AON_PWR_REG01_SYS_CLK_SEL | AON_PWR_REG01_XF_SCK_CLK_SEL));
        AON->PWR_RET01 = (temp | (clock_type << AON_PWR_REG01_SYS_CLK_SEL_Pos) |
                          (mcu_clk_2_qspi_clk[clock_type] << AON_PWR_REG01_XF_SCK_CLK_SEL_Pos));
    }

    SystemCoreClock = systemClock[clock_type];

    // update sleep parameters by system clock.
    pwr_mgmt_update_wkup_param();

    return;
}

void SystemCoreGetClock(mcu_clock_type_t *clock_type)
{
    *clock_type = (mcu_clock_type_t)(AON->PWR_RET01 & AON_PWR_REG01_SYS_CLK_SEL);
}

void SystemCoreUpdateClock(void)
{
    SystemCoreClock  = systemClock[AON->PWR_RET01 & AON_PWR_REG01_SYS_CLK_SEL];
}

static inline uint32_t get_wakeup_flag(void)
{
    return (AON->SOFTWARE_2 & (1 << SOFTWARE_REG_WAKEUP_FLAG_POS));
}

void set_msp(void)
{
#ifndef DRIVER_TEST
#ifdef APP_CODE_RUN_ADDR
    __DMB();
    __set_MSP(REG_PL_RD(APP_CODE_RUN_ADDR));
    __DSB();
#endif
#endif
}

static void sdk_init_patch(void)
{
    sactter_copy_info_t sactter_copy_info = {0};

    for (int i = 0; i < (REGION_TABLE_LIMIT - REGION_TABLE_BASE) / (sizeof(sactter_copy_info_t)); i++) {
        memcpy_s((void *)&sactter_copy_info, sizeof (sactter_copy_info), \
                 (void *)(REGION_TABLE_BASE + i * sizeof(sactter_copy_info_t)), sizeof(sactter_copy_info_t));

        if ((sactter_copy_info.fun + 1) == SCATTERLOAD_COPY) {
            if (sactter_copy_info.ram_addr == DFU_DATA_START_ADDR) {
                continue;
            }
            memcpy_s((void *)(sactter_copy_info.ram_addr), sactter_copy_info.len, \
                     (void *)(sactter_copy_info.rom_addr), sactter_copy_info.len);
        } else {
            if ((sactter_copy_info.fun + 1) == SCATTERLOAD_ZEROINIT) {
                memset_s((void *)(sactter_copy_info.ram_addr), sactter_copy_info.len, 0, sactter_copy_info.len);
            }
        }
    }
}

void SystemInit(void)
{
#if (__FPU_USED == 1)
    SCB->CPACR |= ((SCB_CPACR_BASE_NUM << ITEM_10*ITEM_2) |                /* set CP10 Full Access */
                   (SCB_CPACR_BASE_NUM << ITEM_11*ITEM_2)  );              /* set CP11 Full Access */
#endif

#ifdef UNALIGNED_SUPPORT_DISABLE
    SCB->CCR |= SCB_CCR_UNALIGN_TRP_Msk;
#endif

    if (COLD_BOOT == get_wakeup_flag()) {
#if defined(GR5515_D)
        sdk_init_patch();
#endif
    }

    return;
}

void system_platform_init(void)
{
#if (!defined(ROM_RUN_IN_FLASH)) && defined(GR5515_E)
    sdk_init();
#else
#if defined(GR5515_E)
    rom_init();
#endif
#endif

    platform_init();

    /* record app msp */
    g_app_msp_addr = REG_PL_RD(APP_CODE_RUN_ADDR);

    return;
}

void main_init(void)
{
    uint32_t boot_flag = get_wakeup_flag();
    if (COLD_BOOT == boot_flag) {
        __main();
    } else {
        pwr_mgmt_warm_boot();
        while (1) {}
    }
    // Never execute here
}

