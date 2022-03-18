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

#include <stdbool.h>
#include <stdint.h>

#include "ohos_bt_gatt.h"
#include "ohos_bt_gatt_server.h"

#include "ble.h"
#include "scatter_common.h"
#include "gr55xx_sys.h"
#include "app_log.h"

#include "hal_bt_evt_handler.h"


typedef struct
{
    uint8_t   conn_idx;
    BdAddr    peer_addr;
} conn_info_t;

STACK_HEAP_INIT(heaps_table);

static void ble_init_cmp_callback(void);
static void app_sec_rcv_enc_req_cb(uint8_t conn_idx, sec_enc_req_t *p_enc_req);
static void app_gap_adv_start_cb(uint8_t inst_idx, uint8_t status);
static void app_gap_connect_cb(uint8_t conn_idx, uint8_t status, const gap_conn_cmp_t *p_conn_param);
static void app_gap_disconnect_cb(uint8_t conn_idx, uint8_t status, uint8_t reason);
static void app_gap_connection_update_req_cb(uint8_t conn_idx, const gap_conn_param_t *p_conn_param_update_req);
static void app_gap_adv_stop_cb(uint8_t inst_idx, uint8_t status, gap_stopped_reason_t reason);
static void app_gatt_mtu_exchange_cb(uint8_t conn_idx, uint8_t status, uint16_t mtu);

static const gap_cb_fun_t s_app_gap_callbacks =
{
    .app_gap_adv_start_cb               = app_gap_adv_start_cb,
    .app_gap_adv_stop_cb                = app_gap_adv_stop_cb,
    .app_gap_connect_cb                 = app_gap_connect_cb,
    .app_gap_disconnect_cb              = app_gap_disconnect_cb,
    .app_gap_connection_update_req_cb   = app_gap_connection_update_req_cb,
};

static const gatt_common_cb_fun_t s_app_gatt_common_callbacks = 
{
    .app_gatt_mtu_exchange_cb = app_gatt_mtu_exchange_cb,
};

static const sec_cb_fun_t  s_app_sec_callbacks =
{
    .app_sec_enc_req_cb = app_sec_rcv_enc_req_cb,
};

static app_callback_t s_app_ble_callback =
{
    .app_ble_init_cmp_callback = ble_init_cmp_callback,
    .app_gap_callbacks         = &s_app_gap_callbacks,
    .app_gatt_common_callback  = &s_app_gatt_common_callbacks,
    .app_gattc_callback        = NULL,
    .app_sec_callback          = &s_app_sec_callbacks,
};

static uint8_t               s_is_btstack_init;
static BtGattCallbacks      *s_gatt_callbacks;
BtGattServerCallbacks       *g_gatt_server_callbacks;
static sec_enc_req_type_t    s_sec_enc_req_type;
static conn_info_t           s_conn_info[10];

static sec_param_t  s_sec_param = 
{
    .level      = SEC_MODE1_LEVEL2,
    .io_cap     = IO_NO_INPUT_NO_OUTPUT,
    .auth       = AUTH_BOND,
    .oob        = false,
    .key_size   = 16,
    .ikey_dist  = KDIST_ALL,
    .rkey_dist  = KDIST_ALL,
};



uint8_t get_conn_index(BdAddr bdAddr)
{
    uint8_t  conn_idx = 0xff;

   for (uint8_t i = 0; i < 10; i++)
   {
       if (0 == memcmp(s_conn_info[i].peer_addr.addr, bdAddr.addr, 6))
        {
           conn_idx = i;
           break;
        }
   }

    return conn_idx;
}

static void ble_init_cmp_callback(void)
{
    APP_LOG_INFO(">>>>>>>>>>>>>>>>>>>>>>>>>>BLE Stack Startup!!! ");
    uint8_t   addr[6];
    uint16_t  lenght = 6;

    if (!nvds_get(0xC001, &lenght, (uint8_t*)addr))
    {
        gap_bdaddr_t ble_addr;

        ble_addr.addr_type = 0;
        memcpy(ble_addr.gap_addr.addr, (uint8_t*)addr, 6);
	ble_gap_addr_set(&ble_addr);
    }
    ble_gap_pair_enable(true);
    ble_sec_params_set(&s_sec_param);
    ble_gap_privacy_params_set(900, true);
    ble_gap_l2cap_params_set(247, 247, 1);
    ble_gap_data_length_set(251, 2120);
}

int InitBtStack(void)
{
    APP_LOG_DEBUG(">>>>>>>>>>>>>>>>>>>>>>>>>>%s--- Entry!!! ", __FUNCTION__);
    ble_stack_init(&s_app_ble_callback, &heaps_table);
    s_is_btstack_init = 1;
    return OHOS_BT_STATUS_SUCCESS;
}

int EnableBtStack(void)
{
    APP_LOG_DEBUG(">>>>>>>>>>>>>>>>>>>>>>>>>>%s--- Entry!!! ", __FUNCTION__);
    if  (s_is_btstack_init)
    {
        return OHOS_BT_STATUS_SUCCESS;
    }
    
    return OHOS_BT_STATUS_FAIL;
}

int DisableBtStack(void)
{
    APP_LOG_DEBUG(">>>>>>>>>>>>>>>>>>>>>>>>>>%s--- Entry!!! ", __FUNCTION__);
    return OHOS_BT_STATUS_UNSUPPORTED;
}

int SetDeviceName(const char *name, unsigned int len)
{
    APP_LOG_DEBUG(">>>>>>>>>>>>>>>>>>>>>>>>>>%s--- Entry!!! ", __FUNCTION__);
    return OHOS_BT_STATUS_SUCCESS;
}

int BleSetAdvData(int advId, const BleConfigAdvData *data)
{
    APP_LOG_DEBUG(">>>>>>>>>>>>>>>>>>>>>>>>>>%s--- Entry!!! ", __FUNCTION__);
    uint16_t  adv_len  = data->advLength;
    uint8_t  *adv_data = (uint8_t *)data->advData;

    if (data->advData[0] == 0x02 && data->advData[1] == 0x01)
    {
        adv_len  -= 3;
        adv_data += 3;
    }

    if (ble_gap_adv_data_set(advId,  BLE_GAP_ADV_DATA_TYPE_DATA, adv_data, adv_len))
    {
        return OHOS_BT_STATUS_PARM_INVALID;
    }

    if (data->scanRspData && data->scanRspLength)
    {
        if (ble_gap_adv_data_set(advId,  BLE_GAP_ADV_DATA_TYPE_SCAN_RSP, (uint8_t *)data->scanRspData, (uint16_t)data->scanRspLength))
        {
            return OHOS_BT_STATUS_PARM_INVALID;
        }
    }

    return OHOS_BT_STATUS_SUCCESS;
}


int BleStartAdv(int advId, const BleAdvParams *param)
{
    APP_LOG_DEBUG(">>>>>>>>>>>>>>>>>>>>>>>>>>%s--- Entry!!! ", __FUNCTION__);
    gap_adv_param_t         gap_adv_param = {0};
    gap_adv_time_param_t    gap_adv_time_param = {0};

    switch (param->advType)
    {
        case OHOS_BLE_ADV_IND:
            gap_adv_param.adv_mode =  GAP_ADV_TYPE_ADV_IND;
            break;
        case OHOS_BLE_ADV_DIRECT_IND_HIGH:
            gap_adv_param.adv_mode =  GAP_ADV_TYPE_ADV_HIGH_DIRECT_IND;
            break;
        case OHOS_BLE_ADV_SCAN_IND:
            gap_adv_param.adv_mode =  GAP_ADV_TYPE_ADV_SCAN_IND;
            break;
        case OHOS_BLE_ADV_NONCONN_IND:
            gap_adv_param.adv_mode =  GAP_ADV_TYPE_ADV_NONCONN_IND;
            break;
        case OHOS_BLE_ADV_DIRECT_IND_LOW:
            gap_adv_param.adv_mode =  GAP_ADV_TYPE_ADV_LOW_DIRECT_IND;
            break;
        default:
            return OHOS_BT_STATUS_PARM_INVALID;
    }

    gap_adv_param.disc_mode            = GAP_DISC_MODE_GEN_DISCOVERABLE;  
    gap_adv_param.filter_pol           = param->advFilterPolicy;  
    gap_adv_param. peer_addr.addr_type = param->peerAddrType;   
    gap_adv_param.adv_intv_min         = param->minInterval;   
    gap_adv_param.adv_intv_max         = param->maxInterval; 
    gap_adv_param.chnl_map             = param->channelMap;  
    gap_adv_param.scan_req_ind_en      = 0; 
    gap_adv_param.max_tx_pwr           = param->txPower; 
    memcpy(gap_adv_param.peer_addr.gap_addr.addr, param->peerAddr.addr, OHOS_BD_ADDR_LEN);

    gap_adv_time_param.duration           = param->duration;
    gap_adv_time_param.max_adv_evt = 0;

    if (ble_gap_adv_param_set(advId, BLE_GAP_OWN_ADDR_STATIC, &gap_adv_param))
    {
         return OHOS_BT_STATUS_PARM_INVALID;
    }

    if (ble_gap_adv_start(advId, &gap_adv_time_param))
    {
        return OHOS_BT_STATUS_PARM_INVALID;
    }

    return OHOS_BT_STATUS_SUCCESS;
}

int BleStopAdv(int advId)
{
    APP_LOG_DEBUG(">>>>>>>>>>>>>>>>>>>>>>>>>>%s--- Entry!!! ", __FUNCTION__);
    if (ble_gap_adv_stop(advId))
    {
        return OHOS_BT_STATUS_PARM_INVALID;
    }

    return OHOS_BT_STATUS_SUCCESS;
}


int BleSetSecurityIoCap(BleIoCapMode mode)
{
    APP_LOG_DEBUG(">>>>>>>>>>>>>>>>>>>>>>>>>>%s--- Entry!!! ", __FUNCTION__);
    s_sec_param.io_cap = (sec_io_cap_t)mode;
    ble_sec_params_set(&s_sec_param);
    return OHOS_BT_STATUS_SUCCESS;
}

int BleSetSecurityAuthReq(BleAuthReqMode mode)
{
    APP_LOG_DEBUG(">>>>>>>>>>>>>>>>>>>>>>>>>>%s--- Entry!!! ", __FUNCTION__);
    switch (mode)
    {
        /** No bonding */
        case OHOS_BLE_AUTH_NO_BOND :
            s_sec_param.auth = AUTH_NONE;
            s_sec_param.level = SEC_MODE1_LEVEL1;
            break;
        /** Bonding */
        case OHOS_BLE_AUTH_BOND:
            s_sec_param.auth = AUTH_BOND;
            s_sec_param.level = SEC_MODE1_LEVEL2;
            break;
        /** MITM only */
        case OHOS_BLE_AUTH_REQ_MITM:
             s_sec_param.auth = AUTH_MITM;
             s_sec_param.level = SEC_MODE1_LEVEL3;
            break;
        /** Secure connection only */
        case OHOS_BLE_AUTH_REQ_SC_ONLY:
             s_sec_param.auth = AUTH_SEC_CON;
             s_sec_param.level = SEC_MODE1_LEVEL4;
            break;
        /** Secure connection and bonding */
        case OHOS_BLE_AUTH_REQ_SC_BOND:
             s_sec_param.auth = AUTH_BOND | AUTH_SEC_CON;
             s_sec_param.level = SEC_MODE1_LEVEL4;
            break;
        /** Secure connection and MITM */
        case OHOS_BLE_AUTH_REQ_SC_MITM:
            s_sec_param.auth = AUTH_MITM | AUTH_SEC_CON;
            s_sec_param.level = SEC_MODE1_LEVEL4;
            break;
        /** Secure connection, MITM, and bonding */
        case OHOS_BLE_AUTH_REQ_SC_MITM_BOND:
            s_sec_param.auth = AUTH_BOND | AUTH_MITM | AUTH_SEC_CON;
            s_sec_param.level = SEC_MODE1_LEVEL4;
            break;
        default:
            return OHOS_BT_STATUS_PARM_INVALID;

    }

    ble_sec_params_set(&s_sec_param);
    return OHOS_BT_STATUS_SUCCESS;   
}


int BleGattSecurityRsp(BdAddr bdAddr, bool accept)
{
    APP_LOG_DEBUG(">>>>>>>>>>>>>>>>>>>>>>>>>>%s--- Entry!!! ", __FUNCTION__);
    sec_cfm_enc_t cfm_enc;
    uint8_t  conn_idx;

    memset((uint8_t *)&cfm_enc, 0, sizeof(sec_cfm_enc_t));

    cfm_enc.req_type = s_sec_enc_req_type;
    cfm_enc.accept   = accept;

    conn_idx = get_conn_index(bdAddr);

    if (0xff == conn_idx)
    {
        return OHOS_BT_STATUS_PARM_INVALID;
    }

    ble_sec_enc_cfm(conn_idx, &cfm_enc);

    return OHOS_BT_STATUS_SUCCESS;
}

int GetDeviceMacAddress(unsigned char* result)
{
    APP_LOG_DEBUG(">>>>>>>>>>>>>>>>>>>>>>>>>>%s--- Entry!!! ", __FUNCTION__);
    uint8_t   addr[6];
    uint16_t  lenght = 6;
    if (result == NULL) {
        return OHOS_BT_STATUS_PARM_INVALID;
    }

    if (nvds_get(0xC001, &lenght, (uint8_t*)addr))
    {
        return OHOS_BT_STATUS_PARM_INVALID;
    }

    result[0] = addr[5];
    result[1] = addr[4];
    result[2] = addr[3];
    result[3] = addr[2];
    result[4] = addr[1];
    result[5] = addr[0];
    return OHOS_BT_STATUS_SUCCESS;
}

int ReadBtMacAddr(unsigned char *mac, unsigned int len)
{
    APP_LOG_DEBUG(">>>>>>>>>>>>>>>>>>>>>>>>>>%s--- Entry!!! ", __FUNCTION__);
    uint8_t   addr[6];
    uint16_t  lenght = 6;
    if (mac == NULL || len < 6)
    {
        return OHOS_BT_STATUS_PARM_INVALID;
    }

    if (nvds_get(0xC001, &lenght, (uint8_t*)addr))
    {
        return OHOS_BT_STATUS_PARM_INVALID;
    }

    mac[0] = addr[5];
    mac[1] = addr[4];
    mac[2] = addr[3];
    mac[3] = addr[2];
    mac[4] = addr[1];
    mac[5] = addr[0];
    return OHOS_BT_STATUS_SUCCESS;
}

int BleGattRegisterCallbacks(BtGattCallbacks *func)
{
    APP_LOG_DEBUG(">>>>>>>>>>>>>>>>>>>>>>>>>>%s--- Entry!!! ", __FUNCTION__);
    s_gatt_callbacks = func;
    return OHOS_BT_STATUS_SUCCESS; 
}

int BleStartAdvEx(int *advId, const StartAdvRawData rawData, BleAdvParams advParam)
{
    APP_LOG_DEBUG(">>>>>>>>>>>>>>>>>>>>>>>>>>%s--- Entry!!! ", __FUNCTION__);

    uint16_t  adv_len  = rawData.advDataLen;
    uint8_t  *adv_data = rawData.advData;

    *advId = 0;

    if (rawData.advData[0] == 0x02 && rawData.advData[1] == 0x01)
    {
        adv_len  -= 3;
        adv_data += 3;
    }

    if (ble_gap_adv_data_set((uint8_t)(*advId),  BLE_GAP_ADV_DATA_TYPE_DATA, (uint8_t *)adv_data,  (uint16_t)adv_len))
    {
        return OHOS_BT_STATUS_PARM_INVALID;
    }

    if (rawData.rspData && rawData.rspDataLen)
    {
        if (ble_gap_adv_data_set((uint8_t)(*advId),  BLE_GAP_ADV_DATA_TYPE_SCAN_RSP, (uint8_t *)rawData.rspData, (uint16_t)rawData.rspDataLen))
        {
            return OHOS_BT_STATUS_PARM_INVALID;
        }
    }

     gap_adv_param_t         gap_adv_param = {0};
     gap_adv_time_param_t    gap_adv_time_param = {0};

    switch (advParam.advType)
    {
        case OHOS_BLE_ADV_IND:
            gap_adv_param.adv_mode =  GAP_ADV_TYPE_ADV_IND;
            break;
        case OHOS_BLE_ADV_DIRECT_IND_HIGH:
            gap_adv_param.adv_mode =  GAP_ADV_TYPE_ADV_HIGH_DIRECT_IND;
            break;
        case OHOS_BLE_ADV_SCAN_IND:
            gap_adv_param.adv_mode =  GAP_ADV_TYPE_ADV_SCAN_IND;
            break;
        case OHOS_BLE_ADV_NONCONN_IND:
            gap_adv_param.adv_mode =  GAP_ADV_TYPE_ADV_NONCONN_IND;
            break;
        case OHOS_BLE_ADV_DIRECT_IND_LOW:
            gap_adv_param.adv_mode =  GAP_ADV_TYPE_ADV_LOW_DIRECT_IND;
            break;
        default:
            return OHOS_BT_STATUS_PARM_INVALID;
    }

    gap_adv_param.disc_mode            = GAP_DISC_MODE_GEN_DISCOVERABLE;  
    gap_adv_param.filter_pol           = advParam.advFilterPolicy;   
    gap_adv_param. peer_addr.addr_type = advParam.peerAddrType;   
    gap_adv_param.adv_intv_min         = advParam.minInterval;   
    gap_adv_param.adv_intv_max         = advParam.maxInterval; 
    gap_adv_param.chnl_map             = advParam.channelMap;  
    gap_adv_param.scan_req_ind_en      = 0; 
    gap_adv_param.max_tx_pwr           = advParam.txPower; 
    memcpy(gap_adv_param.peer_addr.gap_addr.addr, advParam.peerAddr.addr, OHOS_BD_ADDR_LEN);

    gap_adv_time_param.duration    = advParam.duration;
    gap_adv_time_param.max_adv_evt = 0;

    if (ble_gap_adv_param_set((uint8_t)(*advId), BLE_GAP_OWN_ADDR_STATIC, &gap_adv_param))
    {
         return OHOS_BT_STATUS_PARM_INVALID;
    }

    if (ble_gap_adv_start((uint8_t)(*advId), &gap_adv_time_param))
    {
        return OHOS_BT_STATUS_PARM_INVALID;
    }

    return OHOS_BT_STATUS_SUCCESS;
}

static void app_gap_adv_start_cb(uint8_t inst_idx, uint8_t status)
{
    APP_LOG_INFO(">>>>>>>>>>>>>>>>>>>>>>>>>>%s---status:%d--- Entry!!! ", __FUNCTION__, status);
    if (s_gatt_callbacks && s_gatt_callbacks->advEnableCb)
    {
        ble_msg_t tx_msg = 
        {
            .msg_type = BLE_MSG_START_ADV,
            .index  = inst_idx,
            .status    = status ? OHOS_BT_STATUS_FAIL : OHOS_BT_STATUS_SUCCESS,
            .func      = s_gatt_callbacks->advEnableCb,
        };
        BleTaskMsgSend(&tx_msg);
    }
}


static void app_gap_adv_stop_cb(uint8_t inst_idx, uint8_t status, gap_stopped_reason_t reason)
{
    APP_LOG_INFO(">>>>>>>>>>>>>>>>>>>>>>>>>>%s--- status:%d--- Entry!!! ", __FUNCTION__, status);
    if (s_gatt_callbacks && s_gatt_callbacks->advDisableCb && GAP_STOPPED_REASON_ON_USER == reason)
    {
        ble_msg_t tx_msg = 
        {
            .msg_type = BLE_MSG_STOP_ADV,
            .index    = inst_idx,
            .status    = status ? OHOS_BT_STATUS_FAIL : OHOS_BT_STATUS_SUCCESS,
            .func      = s_gatt_callbacks->advDisableCb,
        };
        BleTaskMsgSend(&tx_msg);
    }
}

static void app_gap_connect_cb(uint8_t conn_idx, uint8_t status, const gap_conn_cmp_t *p_conn_param)
{
    APP_LOG_INFO(">>>>>>>>>>>>>>>>>>>>>>>>>>%s---conn_idx:%d--- Entry!!! ", __FUNCTION__, conn_idx);
    if (!status)
    {
        ble_gap_phy_update(conn_idx, BLE_GAP_PHY_LE_2MBPS, BLE_GAP_PHY_LE_2MBPS, 0);
        memcpy(s_conn_info[conn_idx].peer_addr.addr, p_conn_param->peer_addr.addr, 6);
        if (g_gatt_server_callbacks && g_gatt_server_callbacks->connectServerCb)
        {
            ble_msg_t tx_msg = 
            {
                .msg_type  = BLE_MSG_CONNECT_NEW_ONE_IND,
                .index     = conn_idx,
                .status    = OHOS_BT_STATUS_SUCCESS,
                .func      = g_gatt_server_callbacks->connectServerCb,
                .length    = 6,
            };
            memcpy(tx_msg.buf, p_conn_param->peer_addr.addr, 6);
            BleTaskMsgSend(&tx_msg);
        }
    }
}

static void app_gap_disconnect_cb(uint8_t conn_idx, uint8_t status, uint8_t reason)
{
    APP_LOG_INFO(">>>>>>>>>>>>>>>>>>>>>>>>>>%s---conn_idx:%d--- Entry!!! ", __FUNCTION__, conn_idx);
    if (!status)
    {
        if (g_gatt_server_callbacks && g_gatt_server_callbacks->disconnectServerCb)
        {
            BdAddr dst_addr;
            memcpy(dst_addr.addr, s_conn_info[conn_idx].peer_addr.addr, 6);
            ble_msg_t tx_msg = 
            {
                .msg_type  = BLE_MSG_DISCONNECT_ONE_IND,
                .index     = conn_idx,
                .status    = OHOS_BT_STATUS_SUCCESS,
                .func      = g_gatt_server_callbacks->disconnectServerCb,
                .length    = 6,
            };
            memcpy(tx_msg.buf, dst_addr.addr, 6);
            BleTaskMsgSend(&tx_msg);
        }
    }
}

static void app_gap_connection_update_req_cb(uint8_t conn_idx, const gap_conn_param_t *p_conn_param_update_req)
{
    APP_LOG_INFO(">>>>>>>>>>>>>>>>>>>>>>>>>>%s---conn_idx:%d--- Entry!!! ", __FUNCTION__, conn_idx);
    ble_gap_conn_param_update_reply(conn_idx, true);
    gap_conn_update_param_t gap_conn_param;
    gap_conn_param.interval_min  = 12;
    gap_conn_param.interval_max  = 12;
    gap_conn_param.slave_latency = 0;
    gap_conn_param.sup_timeout   = 400;
    ble_gap_conn_param_update(conn_idx, &gap_conn_param);
}

static void app_gatt_mtu_exchange_cb(uint8_t conn_idx, uint8_t status, uint16_t mtu)
{
    APP_LOG_INFO(">>>>>>>>>>>>>>>>>>>>>>>>>>MTU exchanged to :%d!!! ", mtu);
    if (g_gatt_server_callbacks && g_gatt_server_callbacks->mtuChangeCb && !status)
    {
        ble_msg_t tx_msg = 
        {
            .msg_type = BLE_MSG_MTU_EXCHANGE_IND,
            .index    = conn_idx,
            .status    = OHOS_BT_STATUS_SUCCESS,
            .func      = g_gatt_server_callbacks->mtuChangeCb,
            .length    = 2,
        };
        memcpy(tx_msg.buf, (uint8_t *)&mtu, 2);
        BleTaskMsgSend(&tx_msg);
    }
}

static void app_sec_rcv_enc_req_cb(uint8_t conn_idx, sec_enc_req_t *p_enc_req)
{
    APP_LOG_INFO(">>>>>>>>>>>>>>>>>>>>>>>>>>%s---conn_idx:%d--- Entry!!! ", __FUNCTION__, conn_idx);

    s_sec_enc_req_type = p_enc_req->req_type;

    if (s_gatt_callbacks && s_gatt_callbacks->securityRespondCb)
    {
        BdAddr dst_addr;
        memcpy(dst_addr.addr,s_conn_info[conn_idx].peer_addr.addr, OHOS_BD_ADDR_LEN);

        ble_msg_t tx_msg = 
        {
            .msg_type = BLE_MSG_ENCRYPT_REQ,
            .index    = conn_idx,
            .status   = OHOS_BT_STATUS_SUCCESS,
            .func     =  s_gatt_callbacks->securityRespondCb,
            .length   = 6,
        };
        memcpy(tx_msg.buf, dst_addr.addr, 6);
        BleTaskMsgSend(&tx_msg);
    }
}


















