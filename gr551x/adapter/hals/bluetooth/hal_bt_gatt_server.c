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

#include <stdint.h>
#include <stdbool.h>

#include "ohos_bt_gatt_server.h"

#include "ble.h"
#include "custom_config.h"
#include "app_log.h"

#include "hal_bt_evt_handler.h"

#define REG_SERV_NUM_MAX  (CFG_MAX_PRFS - 2)

typedef struct
{
    uint8_t  offset;
    uint16_t cccd_val;
} cccd_info_t;

typedef struct
{
    uint8_t              attNum;
    attm_desc_128_t     *attElementTab;
    BleGattOperateFunc  *attOperateFunctab;
    cccd_info_t         *cccdInfoTab;
    uint8_t              cccdCnt;
    uint16_t            *servHandlePtr;
    BleAttribType       *attrTypeTab;
    UuidType             uuidType;
    unsigned char        uuid[OHOS_BLE_UUID_MAX_LEN];

} servDb_info_t;


typedef struct
{
    servDb_info_t servDb[REG_SERV_NUM_MAX];
    uint8_t       servRegIdx;
    uint8_t       servNum;
} serv_env_t;

typedef struct {
    uint8_t servIdx;
    uint8_t attIdx;
} handle_loc_t;

static serv_env_t    s_servEnvInfo;

static ble_err_t ServDbLoad(void);
static void ServGattsReadCb(uint8_t connIdx, const gatts_read_req_cb_t *pReadReq);
static void ServGattsWriteCb(uint8_t connIdx, const gatts_write_req_cb_t *pWriteReq);
static void ServGattsPrepWriteCb(uint8_t connIdx, const gatts_prep_write_req_cb_t *pPrepWriteReq);
static void ServGattsNtfIndCb(uint8_t connIdx, uint8_t status, const ble_gatts_ntf_ind_t *pNtfInd);

static ble_prf_manager_cbs_t s_servMgrCbs = {
    ServDbLoad,
    NULL,
    NULL,
};

static gatts_prf_cbs_t s_servGattsCbs = {
    ServGattsReadCb,
    ServGattsWriteCb,
    ServGattsPrepWriteCb,
    ServGattsNtfIndCb,
    NULL,
};

static prf_server_info_t s_servPrfInfo = {
    .max_connection_nb = CFG_MAX_CONNECTIONS,
    .manager_cbs       = &s_servMgrCbs,
    .gatts_prf_cbs     = &s_servGattsCbs
};

extern BtGattServerCallbacks  *g_gatt_server_callbacks;

static uint16_t get_attr_handle(int serverId, uint8_t char_id)
{
    BleAttribType *attr    = s_servEnvInfo.servDb[serverId].attrTypeTab;
    uint16_t      handle   = *s_servEnvInfo.servDb[serverId].servHandlePtr;
    uint8_t       char_idx = 0;

    for (uint8_t i = 0; i < s_servEnvInfo.servDb[serverId].attNum; i++)
    {
        if (*attr == OHOS_BLE_ATTRIB_TYPE_SERVICE)
        {
            attr++;
            continue;
        }

        handle ++;

        if(*attr == OHOS_BLE_ATTRIB_TYPE_CHAR)
        {
            char_idx++;
        }


        if (char_idx == char_id)
        {
            handle ++;
            break;
        }

        attr++;
    }

    return handle;
}


int BleGattsDisconnect(int serverId, BdAddr bdAddr, int connId)
{
    APP_LOG_DEBUG(">>>>>>>>>>>>>>>>>>>>>>>>>>%s---conn_idx:%d--- Entry!!! ", __FUNCTION__, connId);
    if (ble_gap_disconnect(connId))
    {
        return OHOS_BT_STATUS_PARM_INVALID;
    }

    return OHOS_BT_STATUS_SUCCESS; 
}

int BleGattsSendIndication(int serverId, GattsSendIndParam *param)
{
    gatts_noti_ind_t noti_ind;
    uint16_t err_code;

    uint8_t char_id = param->attrHandle - *s_servEnvInfo.servDb[serverId].servHandlePtr;
    uint16_t handle = get_attr_handle(serverId, char_id);

    APP_LOG_DEBUG(">>>>>>>>>>>>>>>>>>>>>>>>>>%s---handle:%d Entry!!! ", __FUNCTION__, handle);

    noti_ind.type = param->confirm ? BLE_GATT_INDICATION : BLE_GATT_NOTIFICATION;
    noti_ind.length = param->valueLen;
    noti_ind.value = param->value;
    noti_ind.handle = handle;  

    err_code = ble_gatts_noti_ind(param->connectId, &noti_ind);
    if (err_code)
    {
        APP_LOG_DEBUG(">>>>>>>>>>>>>>>>>>>>>>>>>>%s---error:%d !!! ", __FUNCTION__, err_code);
        return OHOS_BT_STATUS_PARM_INVALID; 
    }

    return OHOS_BT_STATUS_SUCCESS;
}

int BleGattsSetEncryption(BdAddr bdAddr, BleSecAct secAct)
{
    APP_LOG_DEBUG(">>>>>>>>>>>>>>>>>>>>>>>>>>%s--- Entry!!! ", __FUNCTION__);
    return OHOS_BT_STATUS_SUCCESS;
}

int BleGattsRegisterCallbacks(BtGattServerCallbacks *func)
{
    APP_LOG_DEBUG(">>>>>>>>>>>>>>>>>>>>>>>>>>%s--- Entry!!! ", __FUNCTION__);
    g_gatt_server_callbacks =  func;
    return OHOS_BT_STATUS_SUCCESS;
}

static int ServDeclAttStore(BleGattAttr *attrInfo, attm_desc_128_t *table, unsigned int att_idx)
{
    if (NULL == attrInfo || NULL == table)
    {
        return OHOS_BT_STATUS_PARM_INVALID;
    }

    uint8_t uuid[16] = BLE_ATT_16_TO_128_ARRAY(BLE_ATT_DECL_PRIMARY_SERVICE);
    memcpy(table[att_idx].uuid, uuid, 16);

    table[att_idx].perm = READ_PERM_UNSEC;
    table[att_idx].ext_perm = 0;
    table[att_idx].max_size = 0;

    return OHOS_BT_STATUS_SUCCESS;
}

static void CharAttPermSet(uint16_t *attPerm, unsigned int properties, unsigned int permission)
{
    uint8_t rd_access = 0;
    uint8_t wr_access = 0;

    if(permission & OHOS_GATT_PERMISSION_READ)
    {
        rd_access = NOAUTH;
    }
    if(permission & OHOS_GATT_PERMISSION_READ_ENCRYPTED)
    {
        rd_access = UNAUTH;
    }
    if(permission & OHOS_GATT_PERMISSION_READ_ENCRYPTED_MITM)
    {
        rd_access = AUTH;
    }

    if(permission & OHOS_GATT_PERMISSION_WRITE)
    {
        wr_access = NOAUTH;
    }
    if(permission & OHOS_GATT_PERMISSION_WRITE_ENCRYPTED)
    {
        wr_access = UNAUTH;
    }
    if(permission & OHOS_GATT_PERMISSION_WRITE_ENCRYPTED_MITM)
    {
        wr_access = AUTH;
    }
    if(permission & OHOS_GATT_PERMISSION_WRITE_SIGNED)
    {
        wr_access = SEC_CON;
    }
    if(permission & OHOS_GATT_PERMISSION_WRITE_SIGNED_MITM)
    {
        wr_access = SEC_CON;
    }

    if (properties & OHOS_GATT_CHARACTER_PROPERTY_BIT_BROADCAST)
    {
        *attPerm |= BROADCAST_ENABLE;
    }
    if (properties & OHOS_GATT_CHARACTER_PROPERTY_BIT_READ)
    {
        *attPerm |= READ_PERM(rd_access);
    }
    if (properties & OHOS_GATT_CHARACTER_PROPERTY_BIT_WRITE_NO_RSP)
    {
        *attPerm |= WRITE_CMD_PERM(wr_access);
    }
    if (properties & OHOS_GATT_CHARACTER_PROPERTY_BIT_WRITE)
    {
        *attPerm |= WRITE_REQ_PERM(wr_access);
    }
    if (properties & OHOS_GATT_CHARACTER_PROPERTY_BIT_NOTIFY)
    {
        *attPerm |= NOTIFY_PERM_UNSEC;
    }
    if (properties & OHOS_GATT_CHARACTER_PROPERTY_BIT_INDICATE)
    {
        *attPerm |= INDICATE_PERM_UNSEC;
    }
    if (properties & OHOS_GATT_CHARACTER_PROPERTY_BIT_SIGNED_WRITE)
    {
        *attPerm |= WRITE_SIGNED_PERM_UNSEC;
    }
    if (properties & OHOS_GATT_CHARACTER_PROPERTY_BIT_EXTENDED_PROPERTY)
    {
        *attPerm |= EXT_PROP_ENABLE;
    }
}

static int CharDeclAttStore(BleGattAttr *attrInfo, attm_desc_128_t *table, unsigned int att_idx)
{
    if (NULL == attrInfo || NULL == table)
    {
        return OHOS_BT_STATUS_PARM_INVALID;
    }

    uint8_t uuid[16] = BLE_ATT_16_TO_128_ARRAY(BLE_ATT_DECL_CHARACTERISTIC);
    memcpy(table[att_idx].uuid, uuid, 16);

    table[att_idx].perm     = READ_PERM_UNSEC;
    table[att_idx].ext_perm = 0;
    table[att_idx].max_size = 0;

    return OHOS_BT_STATUS_SUCCESS;
}

static int CharValueAttStore(BleGattAttr *attrInfo, attm_desc_128_t *table, unsigned int att_idx)
{
    if (NULL == attrInfo || NULL == table)
    {
        return OHOS_BT_STATUS_PARM_INVALID;
    }

    memcpy(table[att_idx].uuid, attrInfo->uuid, 16);
    CharAttPermSet(&table[att_idx].perm, attrInfo->properties, attrInfo->permission);
    table[att_idx].ext_perm = ATT_VAL_LOC_USER;
    if (attrInfo->uuidType == OHOS_UUID_TYPE_128_BIT)
    {
        table[att_idx].ext_perm |= ATT_UUID_TYPE_SET(UUID_TYPE_128);
    }
    table[att_idx].max_size = 512;

    return OHOS_BT_STATUS_SUCCESS;
}

static int CharCccdAttStore(BleGattAttr *attrInfo, attm_desc_128_t *table, unsigned int att_idx)
{
    if (NULL == attrInfo || NULL == table)
    {
        return OHOS_BT_STATUS_PARM_INVALID;
    }

    uint8_t uuid[16] = BLE_ATT_16_TO_128_ARRAY(BLE_ATT_DESC_CLIENT_CHAR_CFG);
    memcpy(table[att_idx].uuid, uuid, 16);
    table[att_idx].perm = WRITE_REQ_PERM_UNSEC | WRITE_CMD_PERM_UNSEC | READ_PERM_UNSEC;
    table[att_idx].ext_perm = 0;
    table[att_idx].max_size = 2;

    return OHOS_BT_STATUS_SUCCESS;
}

static int CharCudAttStore(BleGattAttr *attrInfo, attm_desc_128_t *table, unsigned int att_idx)
{
    if (NULL == attrInfo || NULL == table)
    {
        return OHOS_BT_STATUS_PARM_INVALID;
    }

    uint8_t uuid[16] = BLE_ATT_16_TO_128_ARRAY(BLE_ATT_DESC_CHAR_USER_DESCRIPTION);
    memcpy(table[att_idx].uuid, uuid, 16);
    CharAttPermSet(&table[att_idx].perm, attrInfo->properties, attrInfo->permission);
    table[att_idx].ext_perm = ATT_VAL_LOC_USER;
    table[att_idx].max_size = attrInfo->valLen;

    return OHOS_BT_STATUS_SUCCESS;
}

static uint8_t get_attr_count(BleGattService *srvcInfo, uint8_t *cccd_cnt)
{
    uint8_t attr_num = srvcInfo->attrNum;
    BleGattAttr *attr = srvcInfo->attrList;
    uint8_t tlt_cnt = attr_num;
    *cccd_cnt = 0;

    while(attr_num && attr)
    {
        if(attr->attrType == OHOS_BLE_ATTRIB_TYPE_CHAR)
        {
            tlt_cnt++;//for OHOS_BLE_ATTRIB_TYPE_CHAR_VALUE
            if((attr->properties & OHOS_GATT_CHARACTER_PROPERTY_BIT_NOTIFY) ||(attr->properties & OHOS_GATT_CHARACTER_PROPERTY_BIT_INDICATE))
            {
                tlt_cnt++;
                (*cccd_cnt)++;
            }
        }

        attr_num--;
        attr++;
    }

    return tlt_cnt;
}


int BleGattsStartServiceEx(int *srvcHandle, BleGattService *srvcInfo)
{
    APP_LOG_DEBUG(">>>>>>>>>>>>>>>>>>>>>>>>>>%s--- Entry!!! ", __FUNCTION__);
    uint8_t             att_cnt;
    uint8_t             cccd_cnt; 
    attm_desc_128_t    *attElementTab;
    BleGattOperateFunc *attOperateFunctab;
    cccd_info_t        *cccdInfoTab;
    BleAttribType      *attrTypeTab;
    uint8_t             att_idx = 0;
    uint8_t             cccd_idx = 0;

    if (NULL == srvcInfo || s_servEnvInfo.servNum >= REG_SERV_NUM_MAX)
    {
        return OHOS_BT_STATUS_NOMEM;
    }

    att_cnt = get_attr_count(srvcInfo, &cccd_cnt);

    attElementTab     = sys_malloc(sizeof(attm_desc_128_t) * att_cnt);
    attOperateFunctab = sys_malloc(sizeof(BleGattOperateFunc) * att_cnt);
    attrTypeTab       = sys_malloc(sizeof(BleAttribType) * att_cnt);
    if (NULL == attElementTab || NULL == attOperateFunctab || NULL == attrTypeTab)
    {
        return OHOS_BT_STATUS_NOMEM;
    }

    if (cccd_cnt)
    {
        cccdInfoTab = sys_malloc(sizeof(cccd_info_t) * cccd_cnt);
        if (NULL == cccdInfoTab)
        {
            return OHOS_BT_STATUS_NOMEM;
        }
        memset(cccdInfoTab, 0, sizeof(cccd_info_t) * cccd_cnt);
    }

    memset(attElementTab, 0, sizeof(attm_desc_128_t) * att_cnt);
    memset(attOperateFunctab, 0, sizeof(BleGattOperateFunc) * att_cnt);

    s_servEnvInfo.servDb[s_servEnvInfo.servNum].attNum            = att_cnt;
    s_servEnvInfo.servDb[s_servEnvInfo.servNum].cccdCnt           = cccd_cnt;
    s_servEnvInfo.servDb[s_servEnvInfo.servNum].attElementTab     = attElementTab;
    s_servEnvInfo.servDb[s_servEnvInfo.servNum].attOperateFunctab = attOperateFunctab;
    s_servEnvInfo.servDb[s_servEnvInfo.servNum].cccdInfoTab       = cccdInfoTab;
    s_servEnvInfo.servDb[s_servEnvInfo.servNum].attrTypeTab       = attrTypeTab;

    s_servEnvInfo.servDb[s_servEnvInfo.servNum].servHandlePtr = srvcHandle;

    for (uint8_t i = 0; i < srvcInfo->attrNum; i++)
    {
        switch(srvcInfo->attrList[i].attrType)
        {
            case OHOS_BLE_ATTRIB_TYPE_SERVICE:
                memcpy(s_servEnvInfo.servDb[s_servEnvInfo.servNum].uuid, srvcInfo->attrList[i].uuid, 
                OHOS_BLE_UUID_MAX_LEN);
                s_servEnvInfo.servDb[s_servEnvInfo.servNum].uuidType = srvcInfo->attrList[i].uuidType;
                ServDeclAttStore(&srvcInfo->attrList[i], attElementTab, att_idx);
                attrTypeTab[att_idx] = OHOS_BLE_ATTRIB_TYPE_SERVICE;
                att_idx++;
                break;
            case OHOS_BLE_ATTRIB_TYPE_CHAR:
                CharDeclAttStore(&srvcInfo->attrList[i], attElementTab, att_idx);
                attrTypeTab[att_idx] = OHOS_BLE_ATTRIB_TYPE_CHAR;
                att_idx++;
                CharValueAttStore(&srvcInfo->attrList[i], attElementTab, att_idx);
                attrTypeTab[att_idx] = OHOS_BLE_ATTRIB_TYPE_CHAR_VALUE;
                memcpy(&s_servEnvInfo.servDb[s_servEnvInfo.servNum].attOperateFunctab[att_idx], &srvcInfo->attrList[i].func, sizeof(BleGattOperateFunc));
                APP_LOG_DEBUG(">>>>>>>>>>>>>>>>>>>>>>>>>>service id %d, att id %d, read:%p, write:%p, indicate:%p", 
                    s_servEnvInfo.servNum, att_idx,
                    s_servEnvInfo.servDb[s_servEnvInfo.servNum].attOperateFunctab[att_idx].read,
                    s_servEnvInfo.servDb[s_servEnvInfo.servNum].attOperateFunctab[att_idx].write,
                    s_servEnvInfo.servDb[s_servEnvInfo.servNum].attOperateFunctab[att_idx].indicate);
                att_idx++;
                if((srvcInfo->attrList[i].properties & OHOS_GATT_CHARACTER_PROPERTY_BIT_NOTIFY)
                ||(srvcInfo->attrList[i].properties & OHOS_GATT_CHARACTER_PROPERTY_BIT_INDICATE))
                {
                    cccdInfoTab[cccd_idx].cccd_val = 0;
                    cccdInfoTab[cccd_idx].offset   = att_idx;
                    CharCccdAttStore(&srvcInfo->attrList[i], attElementTab, att_idx);
                    attrTypeTab[att_idx] = OHOS_BLE_ATTRIB_TYPE_CHAR_CLIENT_CONFIG;
                    att_idx++;
                    cccd_idx++;	
                }
                break;
            case OHOS_BLE_ATTRIB_TYPE_CHAR_USER_DESCR:
                CharCudAttStore(&srvcInfo->attrList[i], attElementTab, att_idx);
                attrTypeTab[att_idx] = OHOS_BLE_ATTRIB_TYPE_CHAR_USER_DESCR;
                att_idx++;
                break;
        }
    }

    uint16_t ret = ble_server_prf_add(&s_servPrfInfo);
    if (g_gatt_server_callbacks && g_gatt_server_callbacks->serviceStartCb)
    {
        g_gatt_server_callbacks->serviceStartCb(ret ? 1 : 0, s_servEnvInfo.servNum, 0);
    }
    if (ret)
    {
        return OHOS_BT_STATUS_FAIL;
    }
    s_servEnvInfo.servNum++;

    return OHOS_BT_STATUS_SUCCESS;
}


int BleGattsStopServiceEx(int srvcHandle)
{
    return OHOS_BT_STATUS_UNSUPPORTED;
}

static ble_err_t ServDbLoad(void)
{
    sdk_err_t         error_code;
    gatts_create_db_t gatts_db;

    memset(&gatts_db, 0, sizeof(gatts_create_db_t));

    *(s_servEnvInfo.servDb[s_servEnvInfo.servRegIdx].servHandlePtr) = 0x0000;

    gatts_db.shdl                  = s_servEnvInfo.servDb[s_servEnvInfo.servRegIdx].servHandlePtr;
    gatts_db.uuid                  = s_servEnvInfo.servDb[s_servEnvInfo.servRegIdx].uuid;
    gatts_db.attr_tab_cfg          = NULL;
    gatts_db.max_nb_attr           = s_servEnvInfo.servDb[s_servEnvInfo.servRegIdx].attNum;
    if (s_servEnvInfo.servDb[s_servEnvInfo.servRegIdx].uuidType == OHOS_UUID_TYPE_128_BIT)
    {
        gatts_db.srvc_perm         = SRVC_UUID_TYPE_SET(UUID_TYPE_128);
    }
    else
    {
        gatts_db.srvc_perm         = SRVC_UUID_TYPE_SET(UUID_TYPE_16);
    }
    gatts_db.attr_tab_type         = SERVICE_TABLE_TYPE_128;
    gatts_db.attr_tab.attr_tab_128 = s_servEnvInfo.servDb[s_servEnvInfo.servRegIdx].attElementTab;

    error_code = ble_gatts_srvc_db_create(&gatts_db);

    sys_free(s_servEnvInfo.servDb[s_servEnvInfo.servRegIdx].attElementTab);

    s_servEnvInfo.servRegIdx++;
    return error_code;
}


static bool ServGattsHandlIsValid(uint16_t handle, handle_loc_t *handle_loc)
{
    for (uint8_t i = 0; i < s_servEnvInfo.servNum; i++) 
    {
        if (*s_servEnvInfo.servDb[i].servHandlePtr <= handle && \
            *s_servEnvInfo.servDb[i].servHandlePtr + s_servEnvInfo.servDb[i].attNum >= handle)
        {
            handle_loc->servIdx = i;
            handle_loc->attIdx  = handle - *s_servEnvInfo.servDb[i].servHandlePtr;
            return true;
        }
    }

    return false;
}

static int GattsOperateCccd(uint8_t servId, uint8_t attIdx)
{
    for (uint8_t i = 0; i <  s_servEnvInfo.servDb[servId].cccdCnt; i++)
    {
        if (s_servEnvInfo.servDb[servId].cccdInfoTab[i].offset == attIdx)
        {
             return i;
        }
    }

    return -1;
}

static void ServGattsReadCb(uint8_t connIdx, const gatts_read_req_cb_t *pReadReq)
{
    APP_LOG_DEBUG(">>>>>>>>>>>>>>>>>>>>>>>>>>%s--- Entry!!! ", __FUNCTION__);
    handle_loc_t handleLoc;
    gatts_read_cfm_t cfm;

    cfm.handle = pReadReq->handle;

    if (ServGattsHandlIsValid(pReadReq->handle, &handleLoc))
    {
        int ret =  GattsOperateCccd(handleLoc.servIdx, handleLoc.attIdx);
        if (-1 != ret)
        {
            cfm.length = 2;
            cfm.value  = (uint8_t *)&s_servEnvInfo.servDb[handleLoc.servIdx].cccdInfoTab[ret].cccd_val;
            cfm.status = BLE_SUCCESS;
            ble_gatts_read_cfm(connIdx, &cfm);
            return;
        }
        if (s_servEnvInfo.servDb[handleLoc.servIdx].attOperateFunctab && \
            s_servEnvInfo.servDb[handleLoc.servIdx].attOperateFunctab[handleLoc.attIdx].read)
        {
            ble_msg_t tx_msg = 
            {
                .msg_type  = BLE_MSG_GATT_READ_REQ,
                .index     = connIdx,
                .handle    = pReadReq->handle,
                .status    = OHOS_BT_STATUS_SUCCESS,
                .func      = s_servEnvInfo.servDb[handleLoc.servIdx].attOperateFunctab[handleLoc.attIdx].read,
            };
            BleTaskMsgSend(&tx_msg);
        }
    }
    else
    {
        cfm.status = BLE_ATT_ERR_INVALID_HANDLE;
        ble_gatts_read_cfm(connIdx, &cfm);
    }
}

static void ServGattsWriteCb(uint8_t connIdx, const gatts_write_req_cb_t *pWriteReq)
{
    APP_LOG_DEBUG(">>>>>>>>>>>>>>>>>>>>>>>>>>%s--- Entry!!! ", __FUNCTION__);
    gatts_write_cfm_t cfm;
    handle_loc_t handleLoc;

    cfm.handle = pWriteReq->handle;
    cfm.status = BLE_SUCCESS;

    if (ServGattsHandlIsValid(pWriteReq->handle, &handleLoc))
    {
        ble_gatts_write_cfm(connIdx, &cfm);
        int ret =  GattsOperateCccd(handleLoc.servIdx, handleLoc.attIdx);
        if (-1 != ret)
        {
            s_servEnvInfo.servDb[handleLoc.servIdx].cccdInfoTab[ret].cccd_val = pWriteReq->value[0]| pWriteReq->value[1] << 8;
            return;
        }
        if (s_servEnvInfo.servDb[handleLoc.servIdx].attOperateFunctab && \
            s_servEnvInfo.servDb[handleLoc.servIdx].attOperateFunctab[handleLoc.attIdx].write)
        {
            ble_msg_t tx_msg = 
            {
                .msg_type = BLE_MSG_GATT_WRITE_REQ,
                .index     = connIdx,
                .handle    = pWriteReq->handle,
                .status    = OHOS_BT_STATUS_SUCCESS,
                .length    = pWriteReq->length,
                .func      = s_servEnvInfo.servDb[handleLoc.servIdx].attOperateFunctab[handleLoc.attIdx].write,
            };
            memcpy(tx_msg.buf, pWriteReq->value, pWriteReq->length);
            BleTaskMsgSend(&tx_msg);
        }
    }
    else
    {
        cfm.status = BLE_ATT_ERR_INVALID_HANDLE;
        ble_gatts_write_cfm(connIdx, &cfm);
    }
}

static void ServGattsPrepWriteCb(uint8_t connIdx, const gatts_prep_write_req_cb_t *pPrepWriteReq)
{
    APP_LOG_DEBUG(">>>>>>>>>>>>>>>>>>>>>>>>>>%s--- Entry!!! ", __FUNCTION__);
    gatts_prep_write_cfm_t cfm;
    handle_loc_t handleLoc;

    cfm.handle = pPrepWriteReq->handle;
    cfm.status = BLE_SUCCESS; 

    if (!ServGattsHandlIsValid(pPrepWriteReq->handle, &handleLoc))
    {
        cfm.status = BLE_ATT_ERR_INVALID_HANDLE;
    }

    ble_gatts_prepare_write_cfm(connIdx, &cfm);
}

static void ServGattsNtfIndCb(uint8_t connIdx, uint8_t status, const ble_gatts_ntf_ind_t *pNtfInd)
{
    APP_LOG_DEBUG(">>>>>>>>>>>>>>>>>>>>>>>>>>%s---status:%d Entry!!! ", __FUNCTION__, status);
    handle_loc_t handleLoc;
    if (ServGattsHandlIsValid(pNtfInd->handle, &handleLoc) && !status)
    {
        if (s_servEnvInfo.servDb[handleLoc.servIdx].attOperateFunctab && \
            s_servEnvInfo.servDb[handleLoc.servIdx].attOperateFunctab[handleLoc.attIdx].indicate)
        {
            ble_msg_t tx_msg = 
            {
                .msg_type = BLE_MSG_NTF_IND_CPLT,
                .index     = connIdx,
                .func      = s_servEnvInfo.servDb[handleLoc.servIdx].attOperateFunctab[handleLoc.attIdx].indicate,
            };
            BleTaskMsgSend(&tx_msg);
         }
         else
         {
             APP_LOG_DEBUG(">>>>>>>>>>>>>>>>>>>>>>>>>>No register indicate callback ");
         }
    }
}



