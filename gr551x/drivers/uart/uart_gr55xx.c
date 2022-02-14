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

#include "los_sem.h"
#include "device_resource_if.h"
#include "hdf_base.h"
#include "hdf_log.h"
#include "osal_mem.h"
#include "osal_time.h"
#include "uart_core.h"
#include "uart_if.h"
#include "uart_gr55xx.h"

#define HDF_LOG_TAG uart_gr55xx

// #define HDF_LOGI    printf
// #define HDF_LOGE    printf
// #define GR55_UART_DEBUG

static uint32_t uart_rx_sem[APP_UART_ID_MAX];
static uint32_t uart_tx_mutex[APP_UART_ID_MAX];
static uint32_t uart_rx_mutex[APP_UART_ID_MAX];
static uint32_t g_rx_num[APP_UART_ID_MAX];

static void app_uart0_callback(app_uart_evt_t *p_evt);
static void app_uart1_callback(app_uart_evt_t *p_evt);
static void Gr55xxUartDeviceDump(struct UartDriverData *udd);

static const app_uart_evt_handler_t *evt_handler[APP_UART_ID_MAX] = {
    app_uart0_callback,
    app_uart1_callback
};

static void app_uart0_callback(app_uart_evt_t *p_evt)
{
    if (p_evt->type == APP_UART_EVT_RX_DATA) {
        g_rx_num[APP_UART_ID_0] = p_evt->data.size;
        LOS_SemPost(uart_rx_sem[APP_UART_ID_0]);
    } else if (p_evt->type == APP_UART_EVT_ERROR) {
        LOS_SemPost(uart_rx_sem[APP_UART_ID_0]);
    }
}

static void app_uart1_callback(app_uart_evt_t *p_evt)
{
    if (p_evt->type == APP_UART_EVT_RX_DATA) {
        g_rx_num[APP_UART_ID_1] = p_evt->data.size;
        LOS_SemPost(uart_rx_sem[APP_UART_ID_1]);
    } else if (p_evt->type == APP_UART_EVT_ERROR) {
        LOS_SemPost(uart_rx_sem[APP_UART_ID_1]);
    }
}

static int32_t Gr55xxUartConfig(struct UartDriverData *udd)
{
    uint32_t ret;
    app_uart_params_t *params = NULL;
    if (NULL == udd)
        return HDF_FAILURE;

    params = &udd->params;
    params->id = udd->id;
    params->init.baud_rate = udd->baudrate;

    switch (udd->attr.dataBits)
    {
        case UART_ATTR_DATABIT_5:
            params->init.data_bits  = UART_DATABITS_5;
            break;
        case UART_ATTR_DATABIT_6:
            params->init.data_bits  = UART_DATABITS_6;
            break;
        case UART_ATTR_DATABIT_7:
            params->init.data_bits  = UART_DATABITS_7;
            break;
        case UART_ATTR_DATABIT_8:
        default:
            params->init.data_bits  = UART_DATABITS_8;
            break;
    }

    switch (udd->attr.stopBits)
    {
        default:
        case UART_ATTR_STOPBIT_1:
            params->init.stop_bits = UART_STOPBITS_1;
            break;
        case UART_ATTR_STOPBIT_1P5:
            params->init.stop_bits = UART_STOPBITS_1_5;
            break;
        case UART_ATTR_STOPBIT_2:
            params->init.stop_bits = UART_STOPBITS_2;
            break;
    }

    switch (udd->attr.parity)
    {
        default:
        case UART_ATTR_PARITY_NONE:
            params->init.parity = UART_PARITY_NONE;
            break;
        case UART_ATTR_PARITY_ODD:
            params->init.parity = UART_PARITY_ODD;
            break;
        case UART_ATTR_PARITY_EVEN:
            params->init.parity = UART_PARITY_EVEN;
            break;
    }

    ret = app_uart_init(params, udd->eventCallback, &udd->txBuffer);
    if (ret != 0) {
        HDF_LOGE("%s , app uart init failed\r\n", __func__);
        return HDF_FAILURE;
    }

    Gr55xxUartDeviceDump(udd);

    return HDF_SUCCESS;
}

static int32_t Gr55xxRead(struct UartHost *host, uint8_t *data, uint32_t size)
{
    int32_t ret;
    uint32_t uwRet = 0;
    struct UartDriverData *udd = NULL;

    if (host == NULL || host->priv == NULL) {
        HDF_LOGE("%s: invalid parameter", __func__);
        return HDF_ERR_INVALID_PARAM;
    }
    udd = (struct UartDriverData *)host->priv;
    if (udd->state != UART_STATE_USEABLE) {
        return HDF_FAILURE;
    }

    LOS_MuxPend(uart_rx_mutex[udd->id], LOS_WAIT_FOREVER);

    g_rx_num[udd->id] = 0;
    LOS_SemPend(uart_rx_sem[udd->id], 0);
    ret = app_uart_receive_async(udd->id, data, size);
    if (ret != 0) {
        LOS_MuxPost(uart_rx_mutex[udd->id]);
        return HDF_FAILURE;
    }

    uwRet = LOS_SemPend(uart_rx_sem[udd->id], LOS_WAIT_FOREVER);
    if (uwRet != LOS_OK)  {
        LOS_MuxPost(uart_rx_mutex[udd->id]);
        return HDF_FAILURE;
    }

    LOS_MuxPost(uart_rx_mutex[udd->id]);

    return g_rx_num[udd->id];
}

static int32_t Gr55xxWrite(struct UartHost *host, uint8_t *data, uint32_t size)
{
    int32_t ret;
    struct UartDriverData *udd = NULL;

    if (host == NULL || host->priv == NULL) {
        HDF_LOGE("%s: invalid parameter", __func__);
        return HDF_ERR_INVALID_PARAM;
    }
    udd = (struct UartDriverData *)host->priv;
    if (udd->state != UART_STATE_USEABLE) {
        return HDF_FAILURE;
    }

    LOS_MuxPend(uart_tx_mutex[udd->id], LOS_WAIT_FOREVER);
    ret = app_uart_transmit_sync(udd->id, data, size, 1000);
    if (ret != 0) {
        LOS_MuxPost(uart_tx_mutex[udd->id]);
        HDF_LOGE("uart_%d send %d data failed", udd->id, size);
        return HDF_FAILURE;
    }
    LOS_MuxPost(uart_tx_mutex[udd->id]);

    HDF_LOGI("uart_%d send %d data success", udd->id, size);

    return HDF_SUCCESS;
}

static int32_t Gr55xxGetBaud(struct UartHost *host, uint32_t *baudRate)
{
    struct UartDriverData *udd = NULL;

    if (host == NULL || host->priv == NULL || baudRate == NULL) {
        HDF_LOGE("%s: invalid parameter", __func__);
        return HDF_ERR_INVALID_PARAM;
    }

    udd = (struct UartDriverData *)host->priv;
    if (udd->state != UART_STATE_USEABLE) {
        return HDF_FAILURE;
    }
    *baudRate = udd->baudrate;

    return HDF_SUCCESS;
}

static int32_t Gr55xxSetBaud(struct UartHost *host, uint32_t baudRate)
{
    struct UartDriverData *udd = NULL;

    if (host == NULL || host->priv == NULL) {
        HDF_LOGE("%s: invalid parameter", __func__);
        return HDF_ERR_INVALID_PARAM;
    }

    udd = (struct UartDriverData *)host->priv;
    if (udd->state != UART_STATE_USEABLE) {
        return HDF_FAILURE;
    }
    if ((baudRate > 0) && (baudRate <= CONFIG_MAX_BAUDRATE)) {
        udd->baudrate = baudRate;
        if (udd->config == NULL) {
            HDF_LOGE("%s: not support", __func__);
            return HDF_ERR_NOT_SUPPORT;
        }
        if (udd->config(udd) != HDF_SUCCESS) {
            HDF_LOGE("%s: config baudrate %d failed", __func__, baudRate);
            return HDF_FAILURE;
        }
    } else {
        HDF_LOGE("%s: invalid baudrate, which is:%d", __func__, baudRate);
        return HDF_FAILURE;
    }

    return HDF_SUCCESS;
}

static int32_t Gr55xxGetAttribute(struct UartHost *host, struct UartAttribute *attribute)
{
    struct UartDriverData *udd = NULL;

    if (host == NULL || host->priv == NULL || attribute == NULL) {
        HDF_LOGE("%s: invalid parameter", __func__);
        return HDF_ERR_INVALID_PARAM;
    }
    udd = (struct UartDriverData *)host->priv;
    if (udd->state != UART_STATE_USEABLE) {
        return HDF_FAILURE;
    }

    *attribute = udd->attr;

    return HDF_SUCCESS;
}

static int32_t Gr55xxSetAttribute(struct UartHost *host, struct UartAttribute *attribute)
{
    struct UartDriverData *udd = NULL;

    if (host == NULL || host->priv == NULL || attribute == NULL) {
        HDF_LOGE("%s: invalid parameter", __func__);
        return HDF_ERR_INVALID_PARAM;
    }
    udd = (struct UartDriverData *)host->priv;
    if (udd->state != UART_STATE_USEABLE) {
        return HDF_FAILURE;
    }

    udd->attr = *attribute;
    if (udd->config == NULL) {
        HDF_LOGE("%s: not support", __func__);
        return HDF_ERR_NOT_SUPPORT;
    }
    if (udd->config(udd) != HDF_SUCCESS) {
        HDF_LOGE("%s: config failed", __func__);
        return HDF_FAILURE;
    }

    return HDF_SUCCESS;
}

static int32_t Gr55xxSetTransMode(struct UartHost *host, enum UartTransMode mode)
{
    struct UartDriverData *udd = NULL;

    if (host == NULL || host->priv == NULL) {
        HDF_LOGE("%s: invalid parameter", __func__);
        return HDF_ERR_INVALID_PARAM;
    }

    udd = (struct UartDriverData *)host->priv;
    if (udd->state != UART_STATE_USEABLE) {
        return HDF_FAILURE;
    }
    if (mode == UART_MODE_RD_BLOCK) {
        udd->flags |= UART_FLG_RD_BLOCK;
    } else if (mode == UART_MODE_RD_NONBLOCK) {
        // udd->flags &= ~UART_FLG_RD_BLOCK;
        //only support block mode
        return HDF_FAILURE;
    }

    return HDF_SUCCESS;
}

static int32_t Gr55xxInit(struct UartHost *host)
{
    struct UartDriverData *udd = NULL;
    uint32_t uwRet = 0;
    uint8_t *ptx_buf = NULL;

    if (host == NULL || host->priv == NULL) {
        HDF_LOGE("%s: invalid parameter", __func__);
        return HDF_ERR_INVALID_PARAM;
    }

    udd = (struct UartDriverData *)host->priv;
    if (udd->id >= APP_UART_ID_MAX)
        return HDF_ERR_INVALID_PARAM;

    if (udd->state == UART_STATE_NOT_OPENED) {
        udd->state = UART_STATE_OPENING;

        uwRet = LOS_BinarySemCreate(0, &uart_rx_sem[udd->id]);
        if (uwRet != LOS_OK) {
            return HDF_FAILURE;
        }

        uwRet = LOS_MuxCreate(&uart_tx_mutex[udd->id]);
        if (uwRet != LOS_OK) {
            LOS_SemDelete(&uart_rx_sem[udd->id]);
            return HDF_FAILURE;
        }

        uwRet = LOS_MuxCreate(&uart_rx_mutex[udd->id]);
        if (uwRet != LOS_OK) {
            LOS_SemDelete(uart_rx_sem[udd->id]);
            LOS_SemDelete(uart_tx_mutex[udd->id]);
            return HDF_FAILURE;
        }

        ptx_buf = (uint8_t *)OsalMemCalloc(TX_BUF_SIZE);
        if (ptx_buf == NULL) {
            HDF_LOGE("%s: alloc tx buffer failed", __func__);

            LOS_SemDelete(uart_rx_sem[udd->id]);
            LOS_SemDelete(uart_tx_mutex[udd->id]);
            LOS_SemDelete(uart_rx_mutex[udd->id]);
            return HDF_ERR_MALLOC_FAIL;
        }
        udd->txBuffer.tx_buf = ptx_buf;
        udd->txBuffer.tx_buf_size = TX_BUF_SIZE;

        udd->eventCallback = evt_handler[udd->id];
        udd->config = Gr55xxUartConfig;
        if (udd->config(udd) != HDF_SUCCESS) {
            goto FREE_INITALED_PARA;
        }
    }

    udd->state = UART_STATE_USEABLE;
    udd->count++;
    return HDF_SUCCESS;

FREE_INITALED_PARA:
    LOS_SemDelete(uart_rx_sem[udd->id]);
    LOS_SemDelete(uart_tx_mutex[udd->id]);
    LOS_SemDelete(uart_rx_mutex[udd->id]);
    (void)OsalMemFree(udd->txBuffer.tx_buf);
    udd->txBuffer.tx_buf = NULL;
    return HDF_FAILURE;
}

static int32_t Gr55xxDeinit(struct UartHost *host)
{
    struct UartDriverData *udd = NULL;
    if (host == NULL || host->priv == NULL) {
        HDF_LOGE("%s: invalid parameter", __func__);
        return HDF_ERR_INVALID_PARAM;
    }

    udd = (struct UartDriverData *)host->priv;
    if ((--udd->count) != 0) {
        return HDF_SUCCESS;
    }

    LOS_SemDelete(uart_rx_sem[udd->id]);
    LOS_SemDelete(uart_tx_mutex[udd->id]);
    LOS_SemDelete(uart_rx_mutex[udd->id]);
    if (NULL != udd->txBuffer.tx_buf) {
        (void)OsalMemFree(udd->txBuffer.tx_buf);
        udd->txBuffer.tx_buf = NULL;
    }

    udd->state = UART_STATE_NOT_OPENED;
    return HDF_SUCCESS;
}

static int32_t Gr55xxPollEvent(struct UartHost *host, void *filep, void *table)
{
    struct UartDriverData *udd = NULL;

    if (host == NULL || host->priv == NULL) {
        HDF_LOGE("%s: host is NULL", __func__);
        return HDF_FAILURE;
    }
    udd = (struct UartDriverData *)host->priv;
    if (UART_STATE_USEABLE != udd->state) {
        return HDF_FAILURE;
    }

    return 0;
}

struct UartHostMethod g_uartHostMethod = {
    .Init = Gr55xxInit,
    .Deinit = Gr55xxDeinit,
    .Read = Gr55xxRead,
    .Write = Gr55xxWrite,
    .SetBaud = Gr55xxSetBaud,
    .GetBaud = Gr55xxGetBaud,
    .SetAttribute = Gr55xxSetAttribute,
    .GetAttribute = Gr55xxGetAttribute,
    .SetTransMode = Gr55xxSetTransMode,
    .pollEvent = Gr55xxPollEvent,
};

static int32_t UartGetConfigFromHcs(struct UartDriverData *udd, const struct DeviceResourceNode *node)
{
    uint32_t rcData;
    struct DeviceResourceIface *iface = DeviceResourceGetIfaceInstance(HDF_CONFIG_SOURCE);
    if (iface == NULL || iface->GetUint32 == NULL) {
        HDF_LOGE("%s: face is invalid", __func__);
        return HDF_FAILURE;
    }
    if (iface->GetUint32(node, "id", &rcData, 0) != HDF_SUCCESS) {
        HDF_LOGE("%s: read id fail", __func__);
        return HDF_FAILURE;
    }
    udd->id = rcData;

    if (iface->GetUint32(node, "baudrate", &rcData, 0) != HDF_SUCCESS) {
        HDF_LOGE("%s: read baudrate fail", __func__);
        return HDF_FAILURE;
    }
    udd->baudrate = rcData;

    if (iface->GetUint32(node, "pin_tx_type", &rcData, 0) != HDF_SUCCESS) {
        HDF_LOGE("%s: read pin_tx_type fail", __func__);
        return HDF_FAILURE;
    }
    udd->params.pin_cfg.tx.type = rcData;

    if (iface->GetUint32(node, "pin_tx_pin", &rcData, 0) != HDF_SUCCESS) {
        HDF_LOGE("%s: read pin_tx_pin fail", __func__);
        return HDF_FAILURE;
    }
    udd->params.pin_cfg.tx.pin = (1 << rcData);

    if (iface->GetUint32(node, "pin_tx_mux", &rcData, 0) != HDF_SUCCESS) {
        HDF_LOGE("%s: read pin_tx_pin fail", __func__);
        return HDF_FAILURE;
    }
    udd->params.pin_cfg.tx.mux = rcData;

    if (iface->GetUint32(node, "pin_tx_pull", &rcData, 0) != HDF_SUCCESS) {
        HDF_LOGE("%s: read pin_tx_pin fail", __func__);
        return HDF_FAILURE;
    }
    udd->params.pin_cfg.tx.pull = rcData;

    if (iface->GetUint32(node, "pin_rx_type", &rcData, 0) != HDF_SUCCESS) {
        HDF_LOGE("%s: read pin_rx_type fail", __func__);
        return HDF_FAILURE;
    }
    udd->params.pin_cfg.rx.type = rcData;

    if (iface->GetUint32(node, "pin_rx_pin", &rcData, 0) != HDF_SUCCESS) {
        HDF_LOGE("%s: read pin_rx_pin fail", __func__);
        return HDF_FAILURE;
    }
    udd->params.pin_cfg.rx.pin = (1 << rcData);

    if (iface->GetUint32(node, "pin_rx_mux", &rcData, 0) != HDF_SUCCESS) {
        HDF_LOGE("%s: read pin_rx_pin fail", __func__);
        return HDF_FAILURE;
    }
    udd->params.pin_cfg.rx.mux = rcData;

    if (iface->GetUint32(node, "pin_rx_pull", &rcData, 0) != HDF_SUCCESS) {
        HDF_LOGE("%s: read pin_rx_pin fail", __func__);
        return HDF_FAILURE;
    }
    udd->params.pin_cfg.rx.pull = rcData;

    if (iface->GetUint32(node, "use_mode_type", &rcData, 0) != HDF_SUCCESS) {
        HDF_LOGE("%s: read use_mode_type fail", __func__);
        return HDF_FAILURE;
    }
    udd->params.use_mode.type = rcData;

    if (iface->GetUint32(node, "use_mode_tx_dma_ch", &rcData, 0) != HDF_SUCCESS) {
        HDF_LOGE("%s: read use_mode_tx_dma_ch fail", __func__);
        return HDF_FAILURE;
    }
    udd->params.use_mode.tx_dma_channel = rcData;

    if (iface->GetUint32(node, "use_mode_rx_dma_ch", &rcData, 0) != HDF_SUCCESS) {
        HDF_LOGE("%s: read use_mode_rx_dma_ch fail", __func__);
        return HDF_FAILURE;
    }
    udd->params.use_mode.rx_dma_channel = rcData;

    if (iface->GetUint32(node, "rx_timeout_mode", &rcData, 0) != HDF_SUCCESS) {
        HDF_LOGE("%s: read rx_timeout_mode fail", __func__);
        return HDF_FAILURE;
    }
    udd->params.init.rx_timeout_mode = rcData;

    return HDF_SUCCESS;
}

static void Gr55xxUartDeviceDump(struct UartDriverData *udd)
{
#ifdef GR55_UART_DEBUG
    if (NULL == udd) {
        HDF_LOGI("%s > dump error\r\n", __func__);
    }
    HDF_LOGI("%s > udd.id: %d\r\n", __func__, udd->id);
    HDF_LOGI("%s > udd.baudrate: %d\r\n", __func__, udd->baudrate);
    HDF_LOGI("%s > udd.count: %d\r\n", __func__, udd->count);
    HDF_LOGI("%s > udd.state: %d\r\n", __func__, udd->state);
    HDF_LOGI("%s > params.id: %d\r\n", __func__, udd->params.id);
    HDF_LOGI("%s > params.init.baud_rate: %d\r\n", __func__, udd->params.init.baud_rate);
    HDF_LOGI("%s > params.init.data_bits: %d\r\n", __func__, udd->params.init.data_bits);
    HDF_LOGI("%s > params.init.hw_flow_ctrl: %d\r\n", __func__, udd->params.init.hw_flow_ctrl);
    HDF_LOGI("%s > params.init.parity: %d\r\n", __func__, udd->params.init.parity);
    HDF_LOGI("%s > params.init.rx_timeout_mode: %d\r\n", __func__, udd->params.init.rx_timeout_mode);
    HDF_LOGI("%s > params.init.stop_bits:  %d\r\n", __func__, udd->params.init.stop_bits);

    HDF_LOGI("%s > params.pin_cfg.tx.mux:  %d\r\n", __func__, udd->params.pin_cfg.tx.mux);
    HDF_LOGI("%s > params.pin_cfg.tx.pin:  %d\r\n", __func__, udd->params.pin_cfg.tx.pin);
    HDF_LOGI("%s > params.pin_cfg.tx.pull: %d\r\n", __func__, udd->params.pin_cfg.tx.pull);
    HDF_LOGI("%s > params.pin_cfg.tx.type: %d\r\n", __func__, udd->params.pin_cfg.tx.type);
    HDF_LOGI("%s > params.pin_cfg.rx.mux:  %d\r\n", __func__, udd->params.pin_cfg.rx.mux);
    HDF_LOGI("%s > params.pin_cfg.rx.pin:  %d\r\n", __func__, udd->params.pin_cfg.rx.pin);
    HDF_LOGI("%s > params.pin_cfg.rx.pull: %d\r\n", __func__, udd->params.pin_cfg.rx.pull);
    HDF_LOGI("%s > params.pin_cfg.rx.type: %d\r\n", __func__, udd->params.pin_cfg.rx.type);
    HDF_LOGI("%s > params.use_mode.type:   %d\r\n", __func__, udd->params.use_mode.type);
#else
    (void)udd;
#endif
}

static int32_t Gr55xxAttach(struct UartHost *host, struct HdfDeviceObject *device)
{
    int32_t ret;
    struct UartDriverData *udd = NULL;

    if (device->property == NULL) {
        HDF_LOGE("%s: property is null", __func__);
        return HDF_FAILURE;
    }
    udd = (struct UartDriverData *)OsalMemCalloc(sizeof(*udd));
    if (udd == NULL) {
        HDF_LOGE("%s: OsalMemCalloc udd error", __func__);
        return HDF_ERR_MALLOC_FAIL;
    }

    ret = UartGetConfigFromHcs(udd, device->property);
    if (ret != HDF_SUCCESS || udd->id >= APP_UART_ID_MAX) {
        (void)OsalMemFree(udd);
        return HDF_FAILURE;
    }

    udd->state = UART_STATE_NOT_OPENED;
    udd->config = NULL;
    udd->eventCallback = NULL;
    udd->count = 0;

    udd->params.id = udd->id;
    udd->params.init.baud_rate = udd->baudrate;
    udd->attr.dataBits = DEFAULT_DATABITS;
    udd->attr.stopBits = DEFAULT_STOPBITS;
    udd->attr.parity = DEFAULT_PARITY;

    host->priv = udd;
    host->num = udd->id;

    // UartAddDev(host);
    return HDF_SUCCESS;
}

static void Gr55xxDetach(struct UartHost *host)
{
    struct UartDriverData *udd = NULL;

    if (host->priv == NULL) {
        HDF_LOGE("%s: invalid parameter", __func__);
        return;
    }
    udd = (struct UartDriverData *)host->priv;
    if (udd->state != UART_STATE_NOT_OPENED) {
        HDF_LOGE("%s: uart driver data state invalid", __func__);
        return;
    }
    // UartRemoveDev(host);
    (void)OsalMemFree(udd);
    host->priv = NULL;
}

static int32_t HdfUartDeviceBind(struct HdfDeviceObject *device)
{
    HDF_LOGI("%s: enter\r\n", __func__);
    if (device == NULL) {
        return HDF_ERR_INVALID_OBJECT;
    }
    return (UartHostCreate(device) == NULL) ? HDF_FAILURE : HDF_SUCCESS;
}

int32_t HdfUartDeviceInit(struct HdfDeviceObject *device)
{
    int32_t ret;
    struct UartHost *host = NULL;

    HDF_LOGI("%s: enter\r\n", __func__);
    if (device == NULL) {
        HDF_LOGE("%s: device is null", __func__);
        return HDF_ERR_INVALID_OBJECT;
    }
    host = UartHostFromDevice(device);
    if (host == NULL) {
        HDF_LOGE("%s: host is null", __func__);
        return HDF_FAILURE;
    }
    ret = Gr55xxAttach(host, device);
    if (ret != HDF_SUCCESS) {
        HDF_LOGE("%s: attach error", __func__);
        return HDF_FAILURE;
    }
    host->method = &g_uartHostMethod;
    return ret;
}

void HdfUartDeviceRelease(struct HdfDeviceObject *device)
{
    struct UartHost *host = NULL;

    HDF_LOGI("%s: enter", __func__);
    if (device == NULL) {
        HDF_LOGE("%s: device is null", __func__);
        return;
    }
    host = UartHostFromDevice(device);
    if (host == NULL) {
        HDF_LOGE("%s: host is null", __func__);
        return;
    }
    if (host->priv != NULL) {
        Gr55xxDetach(host);
    }
    UartHostDestroy(host);
}

struct HdfDriverEntry g_hdfUartDevice = {
    .moduleVersion = 1,
    .moduleName = "HDF_PLATFORM_UART",
    .Bind = HdfUartDeviceBind,
    .Init = HdfUartDeviceInit,
    .Release = HdfUartDeviceRelease,
};

HDF_INIT(g_hdfUartDevice);
