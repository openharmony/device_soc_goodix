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

#include "main.h"
#include "los_arch_interrupt.h"
#include "stdint.h"
#include "gr55xx_sys.h"
#include "custom_config.h"
#include "uart.h"
#include "app_rng.h"
#include "app_log.h"
#include "los_task.h"
#include "cmsis_os2.h"

#define CN_MINISECONDS_IN_SECOND    1000
#define CN_MAINBOOT_TASK_STACKSIZE  0X1000
#define CN_MAINBOOT_TASK_PRIOR      2
#define CN_MAINBOOT_TASK_NAME       "MainBoot"

#define RNG_PARAM   {APP_RNG_TYPE_POLLING, {RNG_SEED_FR0_S0, RNG_LFSR_MODE_59BIT, RNG_OUTPUT_LFSR, RNG_POST_PRO_NOT}}

static const uint8_t  s_bd_addr[SYS_BD_ADDR_LEN] = {0x08, 0x08, 0x08, 0xea, 0xea, 0xea};
static uint16_t g_random_seed[8] = {0x1234, 0x5678, 0x90AB, 0xCDEF, 0x1468, 0x2345, 0x5329, 0x2411};

/* Initialize system peripherals. */
void SystemPeripheralsInit(void)
{
    uint8_t   addr[6];
    uint16_t  lenght = 6;

    if (NVDS_TAG_NOT_EXISTED == nvds_get(0xC001, &lenght, (uint8_t*)addr)) {
        SYS_SET_BD_ADDR(s_bd_addr);
    }

    bsp_log_init();
    APP_LOG_INFO("GR551x system start!!!");
}

/* Initialize Hardware RNG peripherals. */
void HardwareRandomInit(void)
{
    app_rng_params_t params_t = RNG_PARAM;
    app_rng_init(&params_t, NULL);
}

int HardwareRandomGet(uint32_t *p_random)
{
    int ret = 0;

    ret = app_rng_gen_sync(g_random_seed, p_random);
    if (ret != 0) {
        return -1;
    }

    return 0;
}

void OSVectorInit(void)
{
    uint32_t *p_vector = (uint32_t *)SCB->VTOR;
    p_vector[SVCall_IRQn + OS_SYS_VECTOR_CNT]  = SVC_Handler;

    OsSetVector(WDT_IRQn, (HWI_PROC_FUNC)WDT_IRQHandler);
    OsSetVector(BLE_SDK_IRQn, (HWI_PROC_FUNC)BLE_SDK_Handler);
    OsSetVector(BLE_IRQn, (HWI_PROC_FUNC)BLE_IRQHandler);
    OsSetVector(DMA_IRQn, (HWI_PROC_FUNC)DMA_IRQHandler);
    OsSetVector(SPI_M_IRQn, (HWI_PROC_FUNC)SPI_M_IRQHandler);
    OsSetVector(SPI_S_IRQn, (HWI_PROC_FUNC)SPI_S_IRQHandler);
    OsSetVector(EXT0_IRQn, (HWI_PROC_FUNC)EXT0_IRQHandler);
    OsSetVector(EXT1_IRQn, (HWI_PROC_FUNC)EXT1_IRQHandler);
    OsSetVector(TIMER0_IRQn, (HWI_PROC_FUNC)TIMER0_IRQHandler);
    OsSetVector(TIMER1_IRQn, (HWI_PROC_FUNC)TIMER1_IRQHandler);
    OsSetVector(DUAL_TIMER_IRQn, (HWI_PROC_FUNC)DUAL_TIMER_IRQHandler);
    OsSetVector(QSPI0_IRQn, (HWI_PROC_FUNC)QSPI0_IRQHandler);
    OsSetVector(UART0_IRQn, (HWI_PROC_FUNC)UART0_IRQHandler);
    OsSetVector(UART1_IRQn, (HWI_PROC_FUNC)UART1_IRQHandler);
    OsSetVector(I2C0_IRQn, (HWI_PROC_FUNC)I2C0_IRQHandler);
    OsSetVector(I2C1_IRQn, (HWI_PROC_FUNC)I2C1_IRQHandler);
    OsSetVector(AES_IRQn, (HWI_PROC_FUNC)AES_IRQHandler);
    OsSetVector(HMAC_IRQn, (HWI_PROC_FUNC)HMAC_IRQHandler);
    OsSetVector(EXT2_IRQn, (HWI_PROC_FUNC)EXT2_IRQHandler);
    OsSetVector(RNG_IRQn, (HWI_PROC_FUNC)RNG_IRQHandler);
    OsSetVector(PMU_IRQn, (HWI_PROC_FUNC)PMU_IRQHandler);
    OsSetVector(PKC_IRQn, (HWI_PROC_FUNC)PKC_IRQHandler);
    OsSetVector(XQSPI_IRQn, (HWI_PROC_FUNC)XQSPI_IRQHandler);
    OsSetVector(QSPI1_IRQn, (HWI_PROC_FUNC)QSPI1_IRQHandler);
    OsSetVector(PWR_CMD_IRQn, (HWI_PROC_FUNC)PWR_CMD_IRQHandler);
    OsSetVector(BLESLP_IRQn, (HWI_PROC_FUNC)BLESLP_IRQHandler);
    OsSetVector(SLPTIMER_IRQn, (HWI_PROC_FUNC)SLPTIMER_IRQHandler);
    OsSetVector(COMP_EXT_IRQn, (HWI_PROC_FUNC)COMP_IRQHandler);
    OsSetVector(AON_WDT_IRQn, (HWI_PROC_FUNC)AON_WDT_IRQHandler);
    OsSetVector(I2S_M_IRQn, (HWI_PROC_FUNC)I2S_M_IRQHandler);
    OsSetVector(I2S_S_IRQn, (HWI_PROC_FUNC)I2S_S_IRQHandler);
    OsSetVector(ISO7816_IRQn, (HWI_PROC_FUNC)ISO7816_IRQHandler);
    OsSetVector(PRESENT_IRQn, (HWI_PROC_FUNC)PRESENT_IRQHandler);
    OsSetVector(CALENDAR_IRQn, (HWI_PROC_FUNC)CALENDAR_IRQHandler);

    NVIC_SetPriorityGrouping(0x3);
}

static void MainBoot(void)
{
    UINT32 uwRet;
    UINT32 taskID;
    TSK_INIT_PARAM_S stTask = {0};

    stTask.pfnTaskEntry = (TSK_ENTRY_FUNC)OHOS_SystemInit;
    stTask.uwStackSize = CN_MAINBOOT_TASK_STACKSIZE;
    stTask.pcName = CN_MAINBOOT_TASK_NAME;
    stTask.usTaskPrio = CN_MAINBOOT_TASK_PRIOR;
    uwRet = LOS_TaskCreate(&taskID, &stTask);
    if (uwRet != LOS_OK) {
        APP_LOG_ERROR("MainBoot task create failed!!!");
    }
}

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
    UINT32 ret;

    ret = LOS_KernelInit();
    if (ret == LOS_OK) {
        OSVectorInit();
#if (LOSCFG_USE_SHELL == 1)
        LosShellInit();
        OsShellInit();
#endif
        SystemPeripheralsInit();
        HardwareRandomInit();
        DeviceManagerStart();
        MainBoot();
        LOS_Start();
    }
    return 0;
}

