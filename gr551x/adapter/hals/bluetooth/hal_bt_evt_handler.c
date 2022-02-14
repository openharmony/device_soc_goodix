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

#include "hal_bt_evt_handler.h"
#include "ohos_bt_gatt_server.h"
#include "ohos_bt_gatt.h"

#include "los_task.h"

#include "ble.h"
#include "app_log.h"


#define CN_BLE_EVENT_TASK_STACKSIZE  0X4000
#define CN_BLE_EVENT_TASK_PRIOR      16
#define CN_BLE_EVENT_TASK_NAME       "BLE_EVENT"

#define BLE_MSG_QUEUE_LEN   16
#define BLE_MSG_INFO_SIZE   sizeof(ble_msg_t)

static uint32_t g_bleMsgHandle;

static void BleEventTask(void);
static void AppGattsReadReqHandler(ble_msg_t *msg);
static void AppGattsWriteReqHandler(ble_msg_t *msg);
static void AppAdvStartHandler(ble_msg_t *msg);
static void AppAdvStopHandler(ble_msg_t *msg);
static void AppGattsConnHandler(ble_msg_t *msg);
static void AppGattsDisConnHandler(ble_msg_t *msg);
static void AppGattsMTUExchangeHandler(ble_msg_t *msg);
static void AppSecEncryptReqHandler(ble_msg_t *msg);
static void AppGattsNtyIndHandler(ble_msg_t *msg);


void BleEventTaskInit(void)
{
    UINT32 uwRet;
    UINT32 taskID;
    TSK_INIT_PARAM_S stTask;

    if (LOS_OK != LOS_QueueCreate("ble_msg_q", BLE_MSG_QUEUE_LEN, &g_bleMsgHandle, 0, BLE_MSG_INFO_SIZE)) {
        APP_LOG_ERROR(">>>>>>>>>>>>>>>>>>>>>>>>>>LOS_QueueCreate fail %d",uwRet);
        return;
    }

    stTask.pfnTaskEntry = (TSK_ENTRY_FUNC)BleEventTask;
    stTask.uwStackSize = CN_BLE_EVENT_TASK_STACKSIZE;
    stTask.pcName = CN_BLE_EVENT_TASK_NAME;
    stTask.usTaskPrio = CN_BLE_EVENT_TASK_PRIOR; /* Os task priority is 6 */
    uwRet = LOS_TaskCreate(&taskID, &stTask);
    if (uwRet != LOS_OK) {
        APP_LOG_ERROR(">>>>>>>>>>>>>>>>>>>>>>>>>>BleEventTask create failed");
    }
}

bool BleTaskMsgSend(ble_msg_t *tx_msg)
{
    UINT32 uwRet;
    uwRet = LOS_QueueWriteCopy(g_bleMsgHandle, tx_msg, sizeof(ble_msg_t), 0);
    if (uwRet != LOS_OK) {
        APP_LOG_ERROR(">>>>>>>>>>>>>>>>>>>>>>>>>>LOS_QueueWriteCopy fail %d",uwRet);
        return false;
    }
    return true;
}


static void BleEventTask(void)
{
    uint32_t msg_size = BLE_MSG_INFO_SIZE;
    uint32_t uwRet;
    ble_msg_t rx_msg;

    while (1)
    {
        uwRet = LOS_QueueReadCopy(g_bleMsgHandle, &rx_msg, &msg_size, LOS_WAIT_FOREVER);
        if (uwRet != LOS_OK) 
        {
            continue;
        }

        switch (rx_msg.msg_type)
        {
            case BLE_MSG_START_ADV:
                AppAdvStartHandler(&rx_msg);
                break;
            case BLE_MSG_STOP_ADV:
                AppAdvStopHandler(&rx_msg);
                break;
            case BLE_MSG_GATT_READ_REQ:
                AppGattsReadReqHandler(&rx_msg);
                break;
            case BLE_MSG_GATT_WRITE_REQ:
                AppGattsWriteReqHandler(&rx_msg);
                break;
            case BLE_MSG_CONNECT_NEW_ONE_IND:
                AppGattsConnHandler(&rx_msg);
                break;
            case BLE_MSG_DISCONNECT_ONE_IND:
                AppGattsDisConnHandler(&rx_msg);
                break;
            case BLE_MSG_MTU_EXCHANGE_IND:
                AppGattsMTUExchangeHandler(&rx_msg);
                break;
            case BLE_MSG_ENCRYPT_REQ:
                AppSecEncryptReqHandler(&rx_msg);
                break;
            case BLE_MSG_NTF_IND_CPLT:
                AppGattsNtyIndHandler(&rx_msg);
                break;
            default:
                break;
        }
    }
}

static void AppAdvStartHandler(ble_msg_t *msg)
{
    APP_LOG_DEBUG(">>>>>>>>>>>>>>>>>>>>>>>>>>%s---Entry!!! ", __FUNCTION__);
    if (msg && msg->func)
    {
        ((AdvEnableCallback)(msg->func))(msg->index, msg->status);
    }
}

static void AppAdvStopHandler(ble_msg_t *msg)
{
    APP_LOG_DEBUG(">>>>>>>>>>>>>>>>>>>>>>>>>>%s---Entry!!! ", __FUNCTION__);
    if (msg && msg->func)
    {
        ((AdvDisableCallback)(msg->func))(msg->index, msg->status);
    }
}

static void AppGattsReadReqHandler(ble_msg_t *msg)
{
    APP_LOG_DEBUG(">>>>>>>>>>>>>>>>>>>>>>>>>>%s---Entry!!! ", __FUNCTION__);
    if (msg && msg->func)
    {
        gatts_read_cfm_t cfm;
        unsigned char buff[244];
        unsigned int  len = 244;
        ((BleGattServiceRead)(msg->func))(buff, &len);
        cfm.handle = msg->handle;
        cfm.status = BLE_SUCCESS;
        cfm.value  = buff;
        cfm.length = len;
        ble_gatts_read_cfm(msg->index, &cfm);
    }
}


static void AppGattsWriteReqHandler(ble_msg_t *msg)
{
    APP_LOG_DEBUG(">>>>>>>>>>>>>>>>>>>>>>>>>>%s---Entry!!! ", __FUNCTION__);
    if (msg && msg->func)
    {
        ((BleGattServiceWrite)(msg->func))(msg->buf, msg->length);
    }
}

static void AppGattsConnHandler(ble_msg_t *msg)
{
    APP_LOG_DEBUG(">>>>>>>>>>>>>>>>>>>>>>>>>>%s---Entry!!! ", __FUNCTION__);
    if (msg && msg->func)
    {
        BdAddr dst_addr;
        memcpy(dst_addr.addr, msg->buf, 6);
        ((ConnectServerCallback)(msg->func))(msg->index, 0, &dst_addr);
    }
}

static void AppGattsDisConnHandler(ble_msg_t *msg)
{
    APP_LOG_DEBUG(">>>>>>>>>>>>>>>>>>>>>>>>>>%s---Entry!!! ", __FUNCTION__);
    if (msg && msg->func)
    {
        BdAddr dst_addr;
        memcpy(dst_addr.addr, msg->buf, 6);
        ((DisconnectServerCallback)(msg->func))(msg->index, 0, &dst_addr);
    }
}

static void AppGattsMTUExchangeHandler(ble_msg_t *msg)
{
    APP_LOG_DEBUG(">>>>>>>>>>>>>>>>>>>>>>>>>>%s---Entry!!! ", __FUNCTION__);
    if (msg && msg->func)
    {
        ((MtuChangeCallback)(msg->func))(msg->index, msg->buf[0] | msg->buf[1] << 8);
    }
}

static void AppSecEncryptReqHandler(ble_msg_t *msg)
{
    APP_LOG_DEBUG(">>>>>>>>>>>>>>>>>>>>>>>>>>%s---Entry!!! ", __FUNCTION__);
    if (msg && msg->func)
    {
        BdAddr dst_addr;
        memcpy(dst_addr.addr, msg->buf, 6);
        ((SecurityRespondCallback)(msg->func))(&dst_addr);
    }
}

static void AppGattsNtyIndHandler(ble_msg_t *msg)
{
    APP_LOG_DEBUG(">>>>>>>>>>>>>>>>>>>>>>>>>>%s---Entry!!! ", __FUNCTION__);
    if (msg && msg->func)
    {
        ((BleGattServiceIndicate)(msg->func))(NULL, 0);
    }
}


