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
#include "ble_cfg_net_api.h"
#include "hilink_bt_api.h"
#include "hilink_bt_function.h"
#include "hilink_bt_netcfg_api.h"

#include "app_log.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif /* __cpluscplus */
#endif /* __cpluscplus */

#define CN_MINISECONDS_IN_SECOND    1000
#define CN_MAINBOOT_TASK_STACKSIZE  0X4000
#define CN_MAINBOOT_TASK_PRIOR      2
#define CN_MAINBOOT_TASK_NAME       "MainBoot"

/*
 * 设备基本信息
 * 与hilink sdk相同定义，双模组模式只需一份，已提供给第三方厂家，暂不按编程规范整改
 */
#define DEVICE_SN "12345678"
#define PRODUCT_ID "2EKT"
#define DEVICE_MODEL "D12"
#define DEVICE_TYPE "000"
#define MANUAFACTURER "832"
#define DEVICE_MAC "AABBCCDDEEFF"
/*
 * 请确保设备类型英文名和厂商英文名长度之和不超过17字节
 * 如果需要发送蓝牙广播，设备类型英文名和厂商英文名长度分别不能超过4字节
 * 与hilink sdk相同定义，双模组模式只需一份，已提供给第三方厂家，暂不按编程规范整改
 */
#define DEVICE_TYPE_NAME "YH"
#define MANUAFACTURER_NAME "L"

/* 蓝牙sdk单独使用的定义 */
#define SUB_PRODUCT_ID "00"
#define ADV_TX_POWER 0xF8



/* 与hilink sdk相同定义，双模组模式只需一份，已提供给第三方厂家，暂不按编程规范整改 */
typedef struct {
    const char* sn;      /* 设备唯一标识，比如sn号，长度范围（0,40] */
    const char* prodId;  /* 设备HiLink认证号，长度范围（0,5] */
    const char* model;   /* 设备型号，长度范围（0,32] */
    const char* dev_t;   /* 设备类型，长度范围（0,4] */
    const char* manu;    /* 设备制造商，长度范围（0,4] */
    const char* mac;     /* 设备MAC地址，固定32字节 */
    const char* hiv;     /* 设备Hilink协议版本，长度范围（0,32] */
    const char* fwv;     /* 设备固件版本，长度范围[0,64] */
    const char* hwv;     /* 设备硬件版本，长度范围[0,64] */
    const char* swv;     /* 设备软件版本，长度范围[0,64] */
    const int prot_t;    /* 设备协议类型，取值范围[1,3] */
} dev_info_t;

/*
 * 设备类型英文名和厂商英文名长度之和不能超过17字节
 * 如果需要发送蓝牙广播，设备类型英文名和厂商英文名长度分别不能超过4字节
 * 与hilink sdk相同定义，双模组模式只需一份，已提供给第三方厂家，暂不按编程规范整改
 */
typedef struct {
    const char* devTypeName; /* 设备类型英文名称 */
    const char* manuName;    /* 厂商英文名称 */
} DevNameEn;


#define MIN_LEN(a, b) (((a) < (b)) ? (a) : (b))

/*
 * 设备名称定义, 请确保设备类型英文名和厂商英文名长度之和不超过17字节
 * 如果需要发送蓝牙广播，设备类型英文名和厂商英文名长度分别不能超过4字节
 * 与hilink sdk相同定义，双模组模式只需一份，已提供给第三方厂家，暂不按编程规范整改
 */
DevNameEn g_devNameEn = {
    DEVICE_TYPE_NAME,
    MANUAFACTURER_NAME
};

/*
 * 设备信息定义
 * 与hilink sdk相同定义，双模组模式只需一份，已提供给第三方厂家，暂不按编程规范整改
 */
dev_info_t dev_info = {
    DEVICE_SN,
    PRODUCT_ID,
    DEVICE_MODEL,
    DEVICE_TYPE,
    MANUAFACTURER,
    DEVICE_MAC,
    "1.0.0",
    "1.0.0",
    "1.1.0",
    "1.1.1",
    1
};


static int PutServiceOta(const void *svc, const unsigned char *in, unsigned int inLen,
    unsigned char **out, unsigned int *outLen);
static int GetServiceOta(const void *svc, const unsigned char *in, unsigned int inLen,
    unsigned char **out, unsigned int *outLen);

static int ble_rcv_custom_data(unsigned char *buff, unsigned int len)
{
    APP_LOG_INFO(">>>>>>>>>>>>>>>>>>>>>>>>>>custom data: %s", buff);
    return 0;
}

static BLE_ConfPara isblepair =
{
    .isBlePair = 1,
};

static BLE_InitPara ble_init_param = 
{
    .confPara = &isblepair,
    .advInfo  = NULL,
    .gattList = NULL,
};

static BLE_CfgNetCb ble_cfg_net_cb = 
{
    .rcvCustomDataCb = ble_rcv_custom_data,
};
    

static HILINK_BT_DevInfo g_btDevInfo;


/* 服务信息 */
static HILINK_BT_SvcInfo g_svcInfo[] =
{
    { 64, "ota", PutServiceOta, GetServiceOta, 0, NULL },
};

/* Profile */
static HILINK_BT_Profile g_profile =
{
    .svcNum  = sizeof(g_svcInfo) / sizeof(g_svcInfo[0]),
    .svcInfo = g_svcInfo,
};

/* 蓝牙发送数据接口打桩函数,由厂家实现 */
static int HILINK_BT_FeedBackBtData(const unsigned char *buf, unsigned int len)
{
    (void)buf;
    (void)len;
    return 0;
}

static int PutServiceOta(const void *svc, const unsigned char *in, unsigned int inLen,
    unsigned char **out, unsigned int *outLen)
{
    (void)inLen;
    (void)out;
    (void)outLen;
    APP_LOG_INFO(">>>>>>>>>>>>>>>>>>>>>>>>>>service:%s, payload:%s", (char *)svc, (char *)in);
    return 0;
}

static int GetServiceOta(const void *svc, const unsigned char *in, unsigned int inLen,
    unsigned char **out, unsigned int *outLen)
{
    (void)inLen;
    (void)out;
    (void)outLen;
    APP_LOG_INFO(">>>>>>>>>>>>>>>>>>>>>>>>>>service:%s, payload:%s", (char *)svc, (char *)in);
    return 0;
}

static void hilink_bt_event_handler(HILINK_BT_SdkStatus event, const void *param)
{
    if (HILINK_BT_SDK_STATUS_SVC_RUNNING == event)
    {
        BLE_CfgNetAdvCtrl(0xfffffffe);
    }
}

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
        // BleAdapterPreInit();
        // HILINK_BT_SetSdkEventCallback(hilink_bt_event_handler);
        // BLE_CfgNetInit(&ble_init_param, &ble_cfg_net_cb);
        HardwareRandomInit();
        DeviceManagerStart();
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

/* 获取蓝牙SDK设备相关信息 */
HILINK_BT_DevInfo *HILINK_BT_GetDevInfo(void)
{
    g_btDevInfo.manuName = (char *)g_devNameEn.manuName;
    g_btDevInfo.devName = (char *)g_devNameEn.devTypeName;
    g_btDevInfo.productId = (char *)dev_info.prodId;
    g_btDevInfo.mac = (char *)dev_info.mac;
    g_btDevInfo.model = (char *)dev_info.model;
    g_btDevInfo.devType = (char *)dev_info.dev_t;
    g_btDevInfo.hiv = (char *)dev_info.hiv;
    g_btDevInfo.protType = dev_info.prot_t;
    return &g_btDevInfo;
}

unsigned char A_C[48] =
{
    0x49, 0x3F, 0x45, 0x4a, 0x3a, 0x72, 0x38, 0x7b, 0x36, 0x32, 0x50, 0x3c, 0x49, 0x39, 0x63, 0x38,
    0x72, 0xcb, 0x6d, 0xc5, 0xae, 0xe5, 0x4a, 0x82, 0xd3, 0xe5, 0x6d, 0xf5, 0x36, 0x82, 0x62, 0xeb,
    0x89, 0x30, 0x6c, 0x88, 0x32, 0x56, 0x23, 0xfd, 0xb8, 0x67, 0x90, 0xa7, 0x7b, 0x61, 0x1e, 0xae
};

unsigned char *HILINK_GetAutoAc(void)
{
    return A_C;
}

int HiLinkGetPinCode(void)
{
    return -1;
}

int getDeviceVersion(char **firmwareVer, char **softwareVer, char **hardwareVer)
{
    *firmwareVer = "1.0.0";//(char *)dev_info.fwv;
    *softwareVer = "1.1.0";//(char *)dev_info.swv;
    *hardwareVer = "1.1.1";//(char *)dev_info.hwv;
    return 0;
}


/*
 * 功能: 获取设备sn号
 * 参数[in]: len sn的最大长度, 39字节
 * 参数[out]: sn 设备sn
 * 注意: sn指针的字符串长度为0时将使用设备mac地址作为sn
 * 与hilink sdk相同定义，双模组模式只需一份，已提供给第三方厂家，暂不按编程规范整改
 */
void HILINK_GetDeviceSn(unsigned int len, char *sn)
{
    if (sn == NULL) {
        return;
    }
    const char *ptr = DEVICE_SN;
    int tmp = MIN_LEN(len ,sizeof(DEVICE_SN));
    for (int i = 0; i < tmp; i++) {
        sn[i] = ptr[i];
    }
    return;
}

/*
 * 获取设备的子型号，长度固定两个字节
 * subProdId为保存子型号的缓冲区，len为缓冲区的长度
 * 如果产品定义有子型号，则填入两字节子型号，并以'\0'结束, 返回0
 * 没有定义子型号，则返回-1
 * 该接口需设备开发者实现
 * 与hilink sdk相同定义，双模组模式只需一份，已提供给第三方厂家，暂不按编程规范整改
 */
int HILINK_GetSubProdId(char *subProdId, int len)
{
    if (subProdId == NULL) {
        return -1;
    }
    const char *ptr = SUB_PRODUCT_ID;
    int tmp = MIN_LEN((unsigned int)len ,sizeof(SUB_PRODUCT_ID));
    for (int i = 0; i < tmp; i++) {
        subProdId[i] = ptr[i];
    }
    return 0;
}

/*
 * 获取设备表面的最强点信号发射功率强度，最强点位置的确定以及功率测试方
 * 法，参照hilink认证蓝牙靠近发现功率设置及测试方法指导文档，power为出参
 * ，单位dbm，返回设备表面的最强信号强度值，如果厂商不想使用蓝牙靠近发现功
 * 能，接口直接返-1，如果需要使用蓝牙靠近发现，则接口返回0，如需及时生效，需
 * 调用HILINK_BT_StartAdvertise()方法启动广播
 */
int HILINK_BT_GetDevSurfacePower(char *power)
{
    if (power == NULL) {
        return -1;
    }
    *power = ADV_TX_POWER;
    return 0;
}

#ifdef __cplusplus
#if __cplusplus
}
#endif /* __cpluscplus */
#endif /* __cpluscplus */


