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

#include "app_io.h"
#include "app_gpiote.h"
#include "device_resource_if.h"
#include "gpio_core.h"
#include "hdf_device_desc.h"
#include "hdf_log.h"
#include "plat_log.h"
#include "stdio.h"

#define HDF_LOG_TAG gpio_gr55xx
#define GR55_IO_NUM_GPIO        32
#define GR55_IO_NUM_AON         8
#define GR55_IO_NUM_MSIO        5
#define GR55_IO_NUM_MAX         (GR55_IO_NUM_GPIO + GR55_IO_NUM_AON + GR55_IO_NUM_MSIO)

#define GR55_IO_BASE_GPIO       0
#define GR55_IO_BASE_AON        GR55_IO_NUM_GPIO
#define GR55_IO_BASE_MSIO       (GR55_IO_NUM_GPIO + GR55_IO_NUM_AON)

#define GR55_IO_PULL_DEFAULT    APP_IO_PULLUP
#define GR55_IO_MODE_DEFAULT    APP_IO_MODE_OUT_PUT

// #define HDF_LOGI    printf
// #define HDF_LOGE    printf
// #define GR55_GPIO_DEBUG

struct Gr55xxGpioPortConfig {
    app_io_mode_t mode;
    app_io_pull_t pull;
    GpioIrqFunc   irqFunc;
    app_handle_mode_t handleMode;
    void         *arg;
};

struct Gr55xxGpioCntlr {
    struct GpioCntlr cntlr;
};

static struct Gr55xxGpioCntlr s_gr55xxGpioCntlr;

struct Gr55xxGpioPortConfig s_portCfg[GR55_IO_NUM_MAX];

static inline struct Gr55xxGpioCntlr *ToGr55xxGpioCntlr(struct GpioCntlr *cntlr)
{
    return (struct Gr55xxGpioCntlr *)cntlr;
}

static inline app_io_type_t Gr55xxPinMap(uint32_t id, uint32_t *pin)
{
    if (id < GR55_IO_BASE_AON)
    {
        *pin  = 1 << (id - GR55_IO_BASE_GPIO);
        return APP_IO_TYPE_NORMAL;
    }
    else if (id < GR55_IO_BASE_MSIO)
    {
        *pin  = 1 << (id - GR55_IO_BASE_AON);
        return APP_IO_TYPE_AON;
    }
    else
    {
        *pin  = 1 << (id - GR55_IO_BASE_MSIO);
        return APP_IO_TYPE_MSIO;
    }
}

static void Gr55xxPortConfigInit(void)
{
    uint16_t cnt;

    for (cnt = 0; cnt < GR55_IO_NUM_MAX; cnt++)
    {
        s_portCfg[cnt].irqFunc = NULL;
        s_portCfg[cnt].arg     = NULL;
        s_portCfg[cnt].pull    = GR55_IO_PULL_DEFAULT;
        s_portCfg[cnt].mode    = GR55_IO_MODE_DEFAULT;
        s_portCfg[cnt].handleMode = APP_IO_NONE_WAKEUP;
    }
}

static int32_t Gr55xxGpioSetDir(struct GpioCntlr *cntlr, uint16_t gpio, uint16_t dir)
{
    uint32_t pin = 0;
    app_io_init_t io_init;
    app_io_type_t io_type;

    if (gpio >= GR55_IO_NUM_MAX)
    {
        return HDF_ERR_INVALID_PARAM;
    }

    io_type = Gr55xxPinMap(gpio, &pin);
    io_init.pin  = pin;
    io_init.pull = s_portCfg[gpio].pull;
    io_init.mux  = APP_IO_MUX_7;
    io_init.mode = GR55_IO_MODE_DEFAULT;

    if (dir == GPIO_DIR_IN)
    {
        io_init.mode = APP_IO_MODE_INPUT;
    }
    else if (dir == GPIO_DIR_OUT)
    {
        io_init.mode = APP_IO_MODE_OUT_PUT;
    }

    s_portCfg[gpio].mode = io_init.mode;

    if (APP_DRV_SUCCESS == app_io_init(io_type, &io_init))
    {
        return HDF_SUCCESS;
    }
    else
    {
        return HDF_FAILURE;
    }
}

static int32_t Gr55xxGpioGetDir(struct GpioCntlr *cntlr, uint16_t gpio, uint16_t *dir)
{
    if (gpio >= GR55_IO_NUM_MAX)
    {
        return HDF_ERR_INVALID_PARAM;
    }

    *dir = s_portCfg[gpio].mode;

    return HDF_SUCCESS;
}

static int32_t Gr55xxGpioWrite(struct GpioCntlr *cntlr, uint16_t gpio, uint16_t val)
{
    uint32_t pin = 0;
    app_io_type_t io_type;

    if (gpio >= GR55_IO_NUM_MAX)
    {
        return HDF_ERR_INVALID_PARAM;
    }

    io_type = Gr55xxPinMap(gpio, &pin);

    if (val == GPIO_VAL_LOW)
    {
        app_io_write_pin(io_type, pin, APP_IO_PIN_RESET);
    }
    else if (val == GPIO_VAL_HIGH)
    {
        app_io_write_pin(io_type, pin, APP_IO_PIN_SET);
    }

    return HDF_SUCCESS;
}

static int32_t Gr55xxGpioRead(struct GpioCntlr *cntlr, uint16_t gpio, uint16_t *val)
{
    uint32_t pin = 0;
    app_io_type_t io_type;
    app_io_pin_state_t io_state;

    if (gpio >= GR55_IO_NUM_MAX)
    {
        return HDF_ERR_INVALID_PARAM;
    }

    io_type = Gr55xxPinMap(gpio, &pin);
    io_state = app_io_read_pin(io_type, pin);

    if (APP_IO_PIN_RESET == io_state)
    {
        *val = GPIO_VAL_LOW;
    }
    else
    {
        *val = GPIO_VAL_HIGH;
    }

    return HDF_SUCCESS;
}

static int get_pin_index(uint32_t pin)
{
    int index = 0;
    while ((pin & 1) != 1)
    {
        index++;
        pin = pin >> 1;
    }
    return index;
}

static void app_io_callback(app_gpiote_evt_t *p_evt)
{
    uint32_t gpio = 0;

    if (p_evt->type == APP_IO_TYPE_NORMAL)
    {
        gpio = get_pin_index(p_evt->pin);
    }
    else if (p_evt->type == APP_IO_TYPE_AON)
    {
        gpio = get_pin_index(p_evt->pin) + GR55_IO_BASE_AON;
    }
    else
    {
        HDF_LOGE("%s, type error", __func__);
    }

    if (s_portCfg[gpio].irqFunc != NULL)
    {
        HDF_LOGI("%s, Func@[%p], arg@[%p]\r\n", __func__, s_portCfg[gpio].irqFunc, s_portCfg[gpio].arg);
        s_portCfg[gpio].irqFunc(gpio, s_portCfg[gpio].arg);
    }
}

static void Gr55xxGpioteDebug(const char *pfunc, app_gpiote_param_t *pgpiote_param)
{
#ifdef GR55_GPIO_DEBUG
    HDF_LOGI("%s, type: [%d]\r\n", pfunc, pgpiote_param->type);
    HDF_LOGI("%s, pin : [%d]\r\n", pfunc, pgpiote_param->pin);
    HDF_LOGI("%s, mode: [%d]\r\n", pfunc, pgpiote_param->mode);
    HDF_LOGI("%s, handle_mode: [%d]\r\n", pfunc, pgpiote_param->handle_mode);
    HDF_LOGI("%s, io_evt_cb:   [%p]\r\n", pfunc, pgpiote_param->io_evt_cb);
#else
    (void)pgpiote_param;
#endif
}

static int32_t Gr55xxGpioEnableIrq(struct GpioCntlr *cntlr, uint16_t gpio)
{
    uint32_t pin = 0;
    app_io_type_t io_type;
    app_gpiote_param_t gpiote_param;

    if (gpio >= GR55_IO_NUM_MAX)
    {
        return HDF_ERR_INVALID_PARAM;
    }

    if (NULL == s_portCfg[gpio].irqFunc)
    {
        return HDF_FAILURE;
    }

    io_type = Gr55xxPinMap(gpio, &pin);
    gpiote_param.type = io_type;
    gpiote_param.pin  = pin;
    gpiote_param.pull = s_portCfg[gpio].pull;
    gpiote_param.mode = s_portCfg[gpio].mode;
    gpiote_param.handle_mode = s_portCfg[gpio].handleMode;
    gpiote_param.io_evt_cb = app_io_callback;

    Gr55xxGpioteDebug(__func__, &gpiote_param);

    app_gpiote_init(&gpiote_param, 1);

    HDF_LOGI("%s: io irq(%u@%p) enabled!", __func__, gpio, s_portCfg[gpio].irqFunc);
    return HDF_SUCCESS;
}

static int32_t Gr55xxGpioDisableIrq(struct GpioCntlr *cntlr, uint16_t gpio)
{
    uint32_t pin = 0;
    app_io_type_t io_type;
    app_gpiote_param_t gpiote_param;

    if (gpio >= GR55_IO_NUM_MAX)
    {
        return HDF_ERR_INVALID_PARAM;
    }

    if (NULL == s_portCfg[gpio].irqFunc)
    {
        return HDF_FAILURE;
    }

    io_type = Gr55xxPinMap(gpio, &pin);
    gpiote_param.type = io_type;
    gpiote_param.pin  = pin;
    gpiote_param.pull = s_portCfg[gpio].pull;
    gpiote_param.mode = s_portCfg[gpio].mode;
    gpiote_param.handle_mode = APP_IO_NONE_WAKEUP;
    gpiote_param.io_evt_cb = NULL;

    Gr55xxGpioteDebug(__func__, &gpiote_param);

    app_gpiote_init(&gpiote_param, 1);

    HDF_LOGI("%s: io irq(%u@%p) disabled!", __func__, gpio, s_portCfg[gpio].irqFunc);
    return HDF_SUCCESS;
}

static int32_t Gr55xxGpioSetIrq(struct GpioCntlr *cntlr, uint16_t gpio, uint16_t mode,
    GpioIrqFunc func, void *arg)
{
    uint32_t pin = 0;
    app_io_type_t io_type;
    app_gpiote_param_t gpiote_param;

    if ((gpio >= GR55_IO_NUM_MAX) || (NULL == func))
    {
        return HDF_ERR_INVALID_PARAM;
    }

    io_type = Gr55xxPinMap(gpio, &pin);
    if (APP_IO_TYPE_MSIO == io_type)
    {
        HDF_LOGE("%s, IO type not support", __func__);
        return HDF_ERR_INVALID_PARAM;
    }

    switch (mode)
    {
        case GPIO_IRQ_TRIGGER_RISING:
            s_portCfg[gpio].mode = APP_IO_MODE_IT_RISING;
            break;
        case GPIO_IRQ_TRIGGER_FALLING:
            s_portCfg[gpio].mode = APP_IO_MODE_IT_FALLING;
            break;
        case GPIO_IRQ_TRIGGER_HIGH:
            s_portCfg[gpio].mode = APP_IO_MODE_IT_HIGH;
            break;
        case GPIO_IRQ_TRIGGER_LOW:
            s_portCfg[gpio].mode = APP_IO_MODE_IT_LOW;
            break;
        default:
            HDF_LOGE("%s, IRQ mode not support", __func__);
            return HDF_ERR_INVALID_PARAM;
    }

    s_portCfg[gpio].irqFunc = func;
    s_portCfg[gpio].arg     = arg;

    gpiote_param.mode = s_portCfg[gpio].mode;
    gpiote_param.io_evt_cb = app_io_callback;

    Gr55xxGpioteDebug(__func__, &gpiote_param);
    //app_gpiote_init(&gpiote_param, 1);

    HDF_LOGI("%s: io irq(%u@%p) registered!", __func__, gpio, func);
    return HDF_SUCCESS;
}

static int32_t Gr55xxGpioUnsetIrq(struct GpioCntlr *cntlr, uint16_t gpio)
{
    uint32_t pin = 0;
    app_io_type_t io_type;
    app_gpiote_param_t gpiote_param;

    if (gpio >= GR55_IO_NUM_MAX)
    {
        return HDF_ERR_INVALID_PARAM;
    }

    if (NULL == s_portCfg[gpio].irqFunc)
    {
        return HDF_SUCCESS;
    }

    s_portCfg[gpio].irqFunc = NULL;
    s_portCfg[gpio].arg     = NULL;

    io_type = Gr55xxPinMap(gpio, &pin);
    gpiote_param.type = io_type;
    gpiote_param.pin  = pin;
    gpiote_param.mode = s_portCfg[gpio].mode;
    gpiote_param.pull = s_portCfg[gpio].pull;
    gpiote_param.handle_mode = APP_IO_NONE_WAKEUP;
    gpiote_param.io_evt_cb = NULL;

    Gr55xxGpioteDebug(__func__, &gpiote_param);
    app_gpiote_init(&gpiote_param, 1);

    return HDF_SUCCESS;
}

static struct GpioMethod g_method = {
    .request = NULL,
    .release = NULL,
    .write = Gr55xxGpioWrite,
    .read = Gr55xxGpioRead,
    .setDir = Gr55xxGpioSetDir,
    .getDir = Gr55xxGpioGetDir,
    .toIrq = NULL,
    .setIrq = Gr55xxGpioSetIrq,
    .unsetIrq = Gr55xxGpioUnsetIrq,
    .enableIrq = Gr55xxGpioEnableIrq,
    .disableIrq = Gr55xxGpioDisableIrq,
};

static int32_t Gr55xxGpioInitCntlrMem(struct Gr55xxGpioCntlr *gr55Gpio)
{
    (void)gr55Gpio;
    return HDF_SUCCESS;
}

static void Gr55xxGpioRleaseCntlrMem(struct Gr55xxGpioCntlr *gr55Gpio)
{
    (void)gr55Gpio;
}

static int32_t Gr55xxGpioReadDrs(struct Gr55xxGpioCntlr *gr55Gpio, const struct DeviceResourceNode *node)
{
    int32_t ret;
    uint16_t configNum = 0;
    uint16_t gpioIndex;
    uint16_t pullConfig;
    uint16_t handleModeConfig;
    struct DeviceResourceIface *drsOps = NULL;

    drsOps = DeviceResourceGetIfaceInstance(HDF_CONFIG_SOURCE);
    if (drsOps == NULL || drsOps->GetUint16 == NULL || drsOps->GetUint16ArrayElem == NULL) {
        HDF_LOGE("%s: invalid drs ops fail!", __func__);
        return HDF_FAILURE;
    }

    ret = drsOps->GetUint16(node, "configNum", &configNum, 0);
    if (ret != HDF_SUCCESS) {
        HDF_LOGE("%s: read configNum fail!", __func__);
        return ret;
    }

    if (configNum >= GR55_IO_NUM_MAX) {
        HDF_LOGE("%s: configNum(%d) invalid!", __func__, configNum);
        return HDF_ERR_INVALID_PARAM;
    }

    for (size_t i = 0; i < configNum; i++)
    {
        ret = drsOps->GetUint16ArrayElem(node, "gpioIndex", i, &gpioIndex, 0);
        if (ret != HDF_SUCCESS) {
            HDF_LOGE("%s: read gpioIndex fail!", __func__);
            return ret;
        }

        ret = drsOps->GetUint16ArrayElem(node, "pull", i, &pullConfig, 0);
        if (ret != HDF_SUCCESS) {
            HDF_LOGE("%s: read regStep fail!", __func__);
            return ret;
        }

        ret = drsOps->GetUint16ArrayElem(node, "handleMode", i, &handleModeConfig, 0);
        if (ret != HDF_SUCCESS) {
            HDF_LOGE("%s: read handleModeConfig fail!", __func__);
            return ret;
        }

        if (gpioIndex >= GR55_IO_NUM_MAX) {
            HDF_LOGE("%s: gpioIndex(%d) invalid!", __func__, gpioIndex);
            return HDF_ERR_INVALID_PARAM;
        }

        if (pullConfig >= APP_IO_PULL_MAX) {
            HDF_LOGE("%s: pullConfig(%d) invalid!", __func__, pullConfig);
            return HDF_ERR_INVALID_PARAM;
        }

        if (handleModeConfig > APP_IO_ENABLE_WAKEUP) {
            HDF_LOGE("%s: handleModeConfig(%d) invalid!", __func__, handleModeConfig);
            return HDF_ERR_INVALID_PARAM;
        }

        s_portCfg[gpioIndex].pull = pullConfig;
        s_portCfg[gpioIndex].handleMode = handleModeConfig;
    }

    return HDF_SUCCESS;
}

static void Gr55xxGpioDebugCntlr(const struct Gr55xxGpioCntlr *gr55Gpio)
{
    (void)gr55Gpio;
}

static int32_t GR55xxGpioBind(struct HdfDeviceObject *device)
{
    (void)device;

    HDF_LOGI("%s: Enter", __func__);

    return HDF_SUCCESS;
}

static int32_t GR55xxGpioInit(struct HdfDeviceObject *device)
{
    int32_t ret;
    struct Gr55xxGpioCntlr *gr55Gpio = &s_gr55xxGpioCntlr;

    HDF_LOGI("%s: Enter", __func__);
    if (device == NULL || device->property == NULL) {
        HDF_LOGE("%s: device or property null!", __func__);
        return HDF_ERR_INVALID_OBJECT;
    }

    Gr55xxPortConfigInit();

    ret = Gr55xxGpioReadDrs(gr55Gpio, device->property);
    if (ret != HDF_SUCCESS) {
        HDF_LOGE("%s: read drs fail:%d", __func__, ret);
        return ret;
    }

    ret = Gr55xxGpioInitCntlrMem(gr55Gpio);
    if (ret != HDF_SUCCESS) {
        HDF_LOGE("%s: err init cntlr mem:%d", __func__, ret);
        return ret;
    }

    gr55Gpio->cntlr.count = GR55_IO_NUM_MAX;
    gr55Gpio->cntlr.priv = (void *)device->property;
    gr55Gpio->cntlr.ops = &g_method;
    gr55Gpio->cntlr.device = device;
    ret = GpioCntlrAdd(&gr55Gpio->cntlr);
    if (ret != HDF_SUCCESS) {
        HDF_LOGE("%s: err add controller:%d", __func__, ret);
        return ret;
    }

    Gr55xxGpioDebugCntlr(gr55Gpio);

    HDF_LOGI("%s: dev service:%s init success!", __func__, HdfDeviceGetServiceName(device));
    return HDF_SUCCESS;
}

static void GR55xxGpioRelease(struct HdfDeviceObject *device)
{
    struct GpioCntlr *cntlr = NULL;
    struct Gr55xxGpioCntlr *gr55Gpio = NULL;

    HDF_LOGI("%s, Enter\r\n", __func__);

    if (device == NULL) {
        HDF_LOGE("%s: device is null!", __func__);
        return;
    }

    cntlr = GpioCntlrFromDevice(device);
    if (cntlr == NULL) {
        HDF_LOGE("%s: no service binded!", __func__);
        return;
    }

    GpioCntlrRemove(cntlr);

    app_gpiote_deinit();

    gr55Gpio = ToGr55xxGpioCntlr(cntlr);
    Gr55xxGpioRleaseCntlrMem(gr55Gpio);
}

struct HdfDriverEntry g_gpioDriverEntry = {
    .moduleVersion = 1,
    .moduleName = "HDF_PLATFORM_GPIO",
    .Bind = GR55xxGpioBind,
    .Init = GR55xxGpioInit,
    .Release = GR55xxGpioRelease,
};

HDF_INIT(g_gpioDriverEntry);
