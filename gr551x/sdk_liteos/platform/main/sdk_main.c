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

#include "los_task.h"
#include "los_interrupt.h"
#include "los_compiler.h"
#include "los_arch_interrupt.h"
#include "cmsis_os2.h"
#include "gr55xx.h"
#include "main.h"
#include "log_serial.h"
#include "core_cm4.h"

#include "app_log.h"
#include "app_timer.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif /* __cpluscplus */
#endif /* __cpluscplus */

#define CN_MINISECONDS_IN_SECOND    1000
#define CN_MAINBOOT_TASK_STACKSIZE  0X4000
#define CN_MAINBOOT_TASK_PRIOR      2
#define CN_MAINBOOT_TASK_NAME       "MainBoot"
#define DEVICE_TYPE_NAME "YH"
#define MANUAFACTURER_NAME "L"

extern void iot_device_main(void);
extern int32_t TestCaseGpioIrqEdge(void);

static void MainBoot(void)
{
    UINT32 uwRet;
    UINT32 taskID;
    TSK_INIT_PARAM_S stTask = {0};

    stTask.pfnTaskEntry = (TSK_ENTRY_FUNC)OHOS_SystemInit;
    stTask.uwStackSize = CN_MAINBOOT_TASK_STACKSIZE;
    stTask.pcName = CN_MAINBOOT_TASK_NAME;
    stTask.usTaskPrio = CN_MAINBOOT_TASK_PRIOR; /* Os task priority is 6 */
    uwRet = LOS_TaskCreate(&taskID, &stTask);
    if (uwRet != LOS_OK) {
        APP_LOG_ERROR(">>>>>>>>>>>>>>>>>>>>>>>>>>MainBoot task create failed!!!");
    }
}

static void BleAdapterPreInit(void)
{
    //The device name must be set here
    ble_gap_device_name_set(BLE_GAP_WRITE_PERM_NOAUTH, DEVICE_TYPE_NAME,  strlen(DEVICE_TYPE_NAME));
    BleEventTaskInit();
}

/***
 * @brief This is the ohos entry, and you could call this in your main funciton after the
 *        necessary hardware has been initialized.
 */
void OHOS_Boot(void)
{
    UINT32 ret;

    ret = LOS_KernelInit();
    if (ret == LOS_OK) {
        OSVectorInit();
        SystemPeripheralsInit();
        FileSystemInit();
        BleAdapterPreInit();
        // iot_device_main();
        HardwareRandomInit();
        DeviceManagerStart();
        // TestCaseGpioIrqEdge();
        MainBoot();
        LOS_Start();
    }
    return;  // and should never come here
}

VOID HalDelay(UINT32 ticks)
{
    delay_ms(ticks);
}

/**
 * @brief Convert miniseconds to system ticks
 * @param ms Indicates the mimiseconds to convert
 * @return Returns the corresponding ticks of specified time
 */
uint32_t Time2Tick(uint32_t ms)
{
    uint64_t ret;
    ret = ((uint64_t)ms * osKernelGetTickFreq()) / CN_MINISECONDS_IN_SECOND;
    return (uint32_t)ret;
}

int HalGetDevUdid( char* udid, int udidLen)
{
    uint8_t uid[16];

    if (sys_device_uid_get(uid))
    {
        return -1;
    }

    udid[0] = uid[3];
    udid[1] = uid[4];
    udid[2] = uid[5];
    udid[3] = uid[6];
    udid[4] = uid[10];
    udid[5] = uid[13];
    udid[6] = uid[14];
    udid[7] = uid[15];
    return 0;
}

#ifdef __cplusplus
#if __cplusplus
}
#endif /* __cpluscplus */
#endif /* __cpluscplus */


