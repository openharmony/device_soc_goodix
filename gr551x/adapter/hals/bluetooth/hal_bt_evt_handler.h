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

#ifndef HAL_BT_EVT_HANDLER_
#define HAL_BT_EVT_HANDLER_

#include "los_queue.h"
#include <stdint.h>
#include <stdbool.h>

#define MSG_PACKET_SIZE 244

typedef enum
{
    BLE_MSG_START_ADV = 1,
    BLE_MSG_STOP_ADV,
    BLE_MSG_CONNECT_NEW_ONE_IND,
    BLE_MSG_CONN_PARAM_UPDATE_IND,
    BLE_MSG_DISCONNECT_ONE_IND,
    BLE_MSG_ENCRYPT_REQ,
    BLE_MSG_ENCRYPT_IND,
    BLE_MSG_MTU_EXCHANGE_IND,
    BLE_MSG_SERV_TREE_REG,
    BLE_MSG_GATT_WRITE_REQ,
    BLE_MSG_GATT_READ_REQ,
    BLE_MSG_CCCD_RECOVERY,
    BLE_MSG_NTF_IND_CPLT
} bleMsgType;

typedef struct
{
    bleMsgType msg_type;
    int      status;
    uint8_t  index;
    uint16_t handle;
    uint8_t  buf[MSG_PACKET_SIZE];
    uint32_t length;
    void     *func;
} ble_msg_t;

void BleEventTaskInit(void);
bool BleTaskMsgSend(ble_msg_t *tx_msg);



#endif
