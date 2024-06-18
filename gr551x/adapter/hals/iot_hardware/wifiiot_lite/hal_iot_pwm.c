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

#include "iot_errno.h"
#include "iot_pwm.h"
#include "app_pwm.h"

// APP_PWM_ID_0
#define USER_PWM0_CHANNEL_A_PIN      APP_IO_PIN_0
#define USER_PWM0_CHANNEL_A_PIN_TYPE APP_IO_TYPE_MSIO
#define USER_PWM0_CHANNEL_A_PIN_MUX  APP_IO_MUX_0
#define USER_PWM0_CHANNEL_A_PIN_PULL APP_IO_NOPULL

#define USER_PWM0_CHANNEL_B_PIN      APP_IO_PIN_1
#define USER_PWM0_CHANNEL_B_PIN_TYPE APP_IO_TYPE_MSIO
#define USER_PWM0_CHANNEL_B_PIN_MUX  APP_IO_MUX_0
#define USER_PWM0_CHANNEL_B_PIN_PULL APP_IO_NOPULL

#define USER_PWM0_CHANNEL_C_PIN      APP_IO_PIN_2
#define USER_PWM0_CHANNEL_C_PIN_TYPE APP_IO_TYPE_MSIO
#define USER_PWM0_CHANNEL_C_PIN_MUX  APP_IO_MUX_0
#define USER_PWM0_CHANNEL_C_PIN_PULL APP_IO_NOPULL

#define PWM0_IO_CONFIG                                                                                                                       \
{                                                                                                                                            \
    {USER_PWM0_CHANNEL_A_PIN_TYPE, USER_PWM0_CHANNEL_A_PIN_MUX, USER_PWM0_CHANNEL_A_PIN, USER_PWM0_CHANNEL_A_PIN_PULL, APP_PWM_PIN_ENABLE},  \
    {USER_PWM0_CHANNEL_B_PIN_TYPE, USER_PWM0_CHANNEL_B_PIN_MUX, USER_PWM0_CHANNEL_B_PIN, USER_PWM0_CHANNEL_B_PIN_PULL, APP_PWM_PIN_ENABLE},  \
    {USER_PWM0_CHANNEL_C_PIN_TYPE, USER_PWM0_CHANNEL_C_PIN_MUX, USER_PWM0_CHANNEL_C_PIN, USER_PWM0_CHANNEL_C_PIN_PULL, APP_PWM_PIN_ENABLE},  \
}

#define PWM0_ACTIVE_CHANNEL APP_PWM_ACTIVE_CHANNEL_ALL

#define PWM0_PWM_CHANNEL_A_CONFIG      \
    {                                  \
        50, PWM_DRIVEPOLARITY_POSITIVE \
    }

#define PWM0_PWM_CHANNEL_B_CONFIG      \
    {                                  \
        50, PWM_DRIVEPOLARITY_POSITIVE \
    }

#define PWM0_PWM_CHANNEL_C_CONFIG      \
    {                                  \
        50, PWM_DRIVEPOLARITY_POSITIVE \
    }

#define PWM0_PWM_CONFIG                                                                                                                     \
    {                                                                                                                                       \
        PWM_MODE_FLICKER, PWM_ALIGNED_EDGE, 10, 500, 200, PWM0_PWM_CHANNEL_A_CONFIG, PWM0_PWM_CHANNEL_B_CONFIG, PWM0_PWM_CHANNEL_C_CONFIG,  \
    }

#define PWM0_PARAM_CONFIG                                                  \
    {                                                                      \
        APP_PWM_ID_0, PWM0_IO_CONFIG, PWM0_ACTIVE_CHANNEL, PWM0_PWM_CONFIG \
    }

// APP_PWM_ID_1
#define USER_PWM1_CHANNEL_A_PIN      APP_IO_PIN_3
#define USER_PWM1_CHANNEL_A_PIN_TYPE APP_IO_TYPE_MSIO
#define USER_PWM1_CHANNEL_A_PIN_MUX  APP_IO_MUX_0
#define USER_PWM1_CHANNEL_A_PIN_PULL APP_IO_NOPULL

#define USER_PWM1_CHANNEL_B_PIN      APP_IO_PIN_4
#define USER_PWM1_CHANNEL_B_PIN_TYPE APP_IO_TYPE_MSIO
#define USER_PWM1_CHANNEL_B_PIN_MUX  APP_IO_MUX_0
#define USER_PWM1_CHANNEL_B_PIN_PULL APP_IO_NOPULL

#define PWM1_IO_CONFIG                                                                                                                       \
{                                                                                                                                            \
    {USER_PWM1_CHANNEL_A_PIN_TYPE, USER_PWM1_CHANNEL_A_PIN_MUX, USER_PWM1_CHANNEL_A_PIN, USER_PWM1_CHANNEL_A_PIN_PULL, APP_PWM_PIN_ENABLE},  \
    {USER_PWM1_CHANNEL_B_PIN_TYPE, USER_PWM1_CHANNEL_B_PIN_MUX, USER_PWM1_CHANNEL_B_PIN, USER_PWM1_CHANNEL_B_PIN_PULL, APP_PWM_PIN_ENABLE},  \
    {0},                                                                                                                                     \
}

#define PWM1_ACTIVE_CHANNEL (APP_PWM_ACTIVE_CHANNEL_A | APP_PWM_ACTIVE_CHANNEL_B)

#define PWM1_PWM_CHANNEL_A_CONFIG      \
    {                                  \
        50, PWM_DRIVEPOLARITY_POSITIVE \
    }

#define PWM1_PWM_CHANNEL_B_CONFIG      \
    {                                  \
        50, PWM_DRIVEPOLARITY_POSITIVE \
    }

#define PWM1_PWM_CONFIG                                                                                               \
    {                                                                                                                 \
        PWM_MODE_FLICKER, PWM_ALIGNED_EDGE, 10, 500, 200, PWM1_PWM_CHANNEL_A_CONFIG, PWM1_PWM_CHANNEL_B_CONFIG, {0},  \
    }

#define PWM1_PARAM_CONFIG                                                  \
    {                                                                      \
        APP_PWM_ID_1, PWM1_IO_CONFIG, PWM1_ACTIVE_CHANNEL, PWM1_PWM_CONFIG \
    }

static app_pwm_params_t s_pwm_params[APP_PWM_ID_MAX] = {
    PWM0_PARAM_CONFIG,
    PWM1_PARAM_CONFIG,
};

unsigned int IoTPwmInit(unsigned int port)
{
    uint16_t ret = APP_DRV_SUCCESS;

    if (port >= APP_PWM_ID_MAX) {
        return IOT_FAILURE;
    }

    ret = app_pwm_init(&s_pwm_params[port]);
    if (ret != APP_DRV_SUCCESS) {
        return IOT_FAILURE;
    }
    return IOT_SUCCESS;
}

unsigned int IoTPwmDeinit(unsigned int port)
{
    int ret = 0;

    if (port >= APP_PWM_ID_MAX) {
        return IOT_FAILURE;
    }

    app_pwm_stop(port);
    ret = app_pwm_deinit(port);
    if (ret != 0) {
        return IOT_FAILURE;
    }
    return IOT_SUCCESS;
}

unsigned int IoTPwmStart(unsigned int port, unsigned short duty, unsigned int freq)
{
    app_pwm_channel_init_t channel_cfg = {0};

    if (port >= APP_PWM_ID_MAX) {
        return IOT_FAILURE;
    }

    app_pwm_update_freq(port, freq);
    channel_cfg.duty = duty;
    channel_cfg.drive_polarity = PWM_DRIVEPOLARITY_POSITIVE;
    app_pwm_config_channel(port, APP_PWM_ACTIVE_CHANNEL_ALL, &channel_cfg);
    app_pwm_start(port);

    return IOT_SUCCESS;
}

unsigned int IoTPwmStop(unsigned int port)
{
    if (port >= APP_PWM_ID_MAX) {
        return IOT_FAILURE;
    }

    app_pwm_stop(port);
    return IOT_SUCCESS;
}
