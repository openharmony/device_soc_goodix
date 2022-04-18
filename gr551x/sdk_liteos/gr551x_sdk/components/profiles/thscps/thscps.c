/**
 *****************************************************************************************
 *
 * @file thscps.c
 *
 * @brief Throughput Control Point Service Implementation.
 *
 *****************************************************************************************
 * @attention
  #####Copyright (c) 2019 GOODIX
  All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are met:
  * Redistributions of source code must retain the above copyright
    notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright
    notice, this list of conditions and the following disclaimer in the
    documentation and/or other materials provided with the distribution.
  * Neither the name of GOODIX nor the names of its contributors may be used
    to endorse or promote products derived from this software without
    specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
  ARE DISCLAIMED. IN NO EVENT SHALL COPYRIGHT HOLDERS AND CONTRIBUTORS BE
  LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
  CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
  SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
  INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
  CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
  ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
  POSSIBILITY OF SUCH DAMAGE.
 *****************************************************************************************
 */

/*
 * INCLUDE FILES
 *****************************************************************************************
 */
#include <string.h>
#include "ble_prf_utils.h"
#include "utility.h"
#include "app_log.h"
#include "thscps.h"

#define INDEX_0 0
#define INDEX_1 1
#define INDEX_2 2
#define INDEX_4 4
#define INDEX_6 6
#define INDEX_7 7
#define LEN_3 3
/*
* DEFINES
*****************************************************************************************
*/
#define THSCPS_CTRL_PT_CHARACTERISTIC_UUID        {0x1B, 0xD7, 0x90, 0xEC, 0xE8, 0xB9, 0x75, 0x80, \
                                                   0x0A, 0x46, 0x44, 0xD3, 0x02, 0x07, 0xED, 0xA6}
#define THSCPS_TEST_SETTING_CHARACTERISTIC_UUID   {0x1B, 0xD7, 0x90, 0xEC, 0xE8, 0xB9, 0x75, 0x80, \
                                                   0x0A, 0x46, 0x44, 0xD3, 0x03, 0x07, 0xED, 0xA6}
#define THSCPS_TEST_INFO_CHARACTERISTIC_UUID      {0x1B, 0xD7, 0x90, 0xEC, 0xE8, 0xB9, 0x75, 0x80, \
                                                   0x0A, 0x46, 0x44, 0xD3, 0x04, 0x07, 0xED, 0xA6}
#define THSCPS_CONN_INFO_CHARACTERISTIC_UUID      {0x1B, 0xD7, 0x90, 0xEC, 0xE8, 0xB9, 0x75, 0x80, \
                                                   0x0A, 0x46, 0x44, 0xD3, 0x05, 0x07, 0xED, 0xA6}

/**@brief Macros for conversion of 128bit to 16bit UUID. */
#define ATT_128_PRIMARY_SERVICE     BLE_ATT_16_TO_128_ARRAY(BLE_ATT_DECL_PRIMARY_SERVICE)
#define ATT_128_CHARACTERISTIC      BLE_ATT_16_TO_128_ARRAY(BLE_ATT_DECL_CHARACTERISTIC)
#define ATT_128_CLIENT_CHAR_CFG     BLE_ATT_16_TO_128_ARRAY(BLE_ATT_DESC_CLIENT_CHAR_CFG)

/*
 * ENUMERATIONS
 *****************************************************************************************
 */
/**@brief THS Control Point Service Attributes Indexes. */
enum {
    // THS Control Point Service
    THSCPS_IDX_SVC,

    // THS Control Point
    THSCPS_IDX_THS_CTRL_PT_CHAR,
    THSCPS_IDX_THS_CTRL_PT_VAL,
    THSCPS_IDX_THS_CTRL_PT_CFG,

    // THS throughput Test Setting
    THSCPS_IDX_TEST_SETTING_CHAR,
    THSCPS_IDX_TEST_SETTING_VAL,
    THSCPS_IDX_TEST_SETTING_CFG,

    // THS throughput Test Information
    THSCPS_IDX_TEST_INFO_CHAR,
    THSCPS_IDX_TEST_INFO_VAL,
    THSCPS_IDX_TEST_INFO_CFG,

    // THS throughput Conn Information
    THSCPS_IDX_CONN_INFO_CHAR,
    THSCPS_IDX_CONN_INFO_VAL,
    THSCPS_IDX_CONN_INFO_CFG,

    THSCPS_IDX_NB
};

/*
 * STRUCTURES
 *****************************************************************************************
 */
/**@brief THS Control Point Service environment variable. */
struct thscps_env_t {
    thscps_evt_handler_t   evt_handler;         /**< THS Control Point Service event handler. */
    uint16_t               start_hdl;           /**< THS Control Point Service start handle. */
    thscps_test_role_t     curr_role;           /**< Current test role. */
    thscps_test_state_t    test_state;          /**< Test is on going or not. */
    uint16_t
    ctrl_pt_ind_cfg[THSCPS_CONNECTION_MAX];      /**< The configuration of Control Point Response \
                                                 which is configured by the peer devices. */
    uint16_t
    test_setting_ntf_cfg[THSCPS_CONNECTION_MAX]; /**< The configuration of Test Setting \
                                                 which is configured by the peer devices. */
    uint16_t
    conn_info_ntf_cfg[THSCPS_CONNECTION_MAX];    /**< The configuration of Connection Information \
                                                 which is configured by the peer devices. */
    uint16_t
    test_info_ntf_cfg[THSCPS_CONNECTION_MAX];    /**< The configuration of Test Information \
                                                 which is configured by the peer devices. */
};

/*
 * LOCAL FUNCTION DECLARATION
 *****************************************************************************************
 */
static sdk_err_t   thscps_init(void);
static void        thscps_read_att_cb(uint8_t  conn_idx, const gatts_read_req_cb_t  *p_param);
static void        thscps_write_att_cb(uint8_t conn_idx, const gatts_write_req_cb_t *p_param);
static void        thscps_cccd_set_cb(uint8_t conn_idx, uint16_t handle, uint16_t cccd_value);
static void        thscps_ctrl_pt_handler(uint8_t conn_idx, const gatts_write_req_cb_t *p_param);
static sdk_err_t   thscps_ctrl_pt_fail_rsp_send(uint8_t conn_idx, thscps_ctrl_pt_id_t cmd_id,
                                                thscps_status_rsp_t status);

/*
 * LOCAL VARIABLE DEFINITIONS
 *****************************************************************************************
 */
static struct thscps_env_t s_thscps_env;
static uint16_t            s_thscps_char_mask = 0x1fff;
/**@brief Full THSCPS Database Description - Used to add attributes into the database. */
static const attm_desc_128_t thscps_attr_tab[THSCPS_IDX_NB] = {
    // THS Control Point Service
    [THSCPS_IDX_SVC] = {ATT_128_PRIMARY_SERVICE, READ_PERM_UNSEC, 0, 0},

    // THS Control Point Characteristic - Declaration
    [THSCPS_IDX_THS_CTRL_PT_CHAR] = {ATT_128_CHARACTERISTIC, READ_PERM_UNSEC, 0, 0},
    // THS Control Point Characteristic - Value
    [THSCPS_IDX_THS_CTRL_PT_VAL]  = {
        THSCPS_CTRL_PT_CHARACTERISTIC_UUID,
        WRITE_REQ_PERM_UNSEC | INDICATE_PERM_UNSEC,
        (ATT_VAL_LOC_USER | ATT_UUID_TYPE_SET(UUID_TYPE_128)),
        THSCPS_CTRL_PT_VAL_LEN
    },
    // THS Control Point Characteristic - Client Characteristic Configuration Descriptor
    [THSCPS_IDX_THS_CTRL_PT_CFG]  = {ATT_128_CLIENT_CHAR_CFG, READ_PERM_UNSEC | WRITE_REQ_PERM_UNSEC, 0, 0},

    // THS Test Setting Characteristic - Declaration
    [THSCPS_IDX_TEST_SETTING_CHAR] = {ATT_128_CHARACTERISTIC, READ_PERM_UNSEC, 0, 0},
    // THS Test Setting Characteristic - Value
    [THSCPS_IDX_TEST_SETTING_VAL]  = {
        THSCPS_TEST_SETTING_CHARACTERISTIC_UUID,
        WRITE_REQ_PERM_UNSEC | NOTIFY_PERM_UNSEC,
        (ATT_VAL_LOC_USER | ATT_UUID_TYPE_SET(UUID_TYPE_128)),
        THSCPS_TEST_SETTING_VAL_LEN
    },
    // THS Test Setting Characteristic - Client Characteristic Configuration Descriptor
    [THSCPS_IDX_TEST_SETTING_CFG]  = {ATT_128_CLIENT_CHAR_CFG, READ_PERM_UNSEC | WRITE_REQ_PERM_UNSEC, 0, 0},

    // THS Test Information Characteristic - Declaration
    [THSCPS_IDX_TEST_INFO_CHAR] = {ATT_128_CHARACTERISTIC, READ_PERM_UNSEC, 0, 0},
    // THS Test Information Characteristic - Value
    [THSCPS_IDX_TEST_INFO_VAL]  = {
        THSCPS_TEST_INFO_CHARACTERISTIC_UUID,
        NOTIFY_PERM_UNSEC,
        (ATT_VAL_LOC_USER | ATT_UUID_TYPE_SET(UUID_TYPE_128)),
        THSCPS_TEST_INFO_VAL_LEN
    },
    // THS Test Information Characteristic - Client Characteristic Configuration Descriptor
    [THSCPS_IDX_TEST_INFO_CFG]  = {ATT_128_CLIENT_CHAR_CFG, READ_PERM_UNSEC | WRITE_REQ_PERM_UNSEC, 0, 0},

    // THS Test Connection Characteristic - Declaration
    [THSCPS_IDX_CONN_INFO_CHAR] = {ATT_128_CHARACTERISTIC, READ_PERM_UNSEC, 0, 0},
    // THS Test Connection Characteristic - Value
    [THSCPS_IDX_CONN_INFO_VAL]  = {
        THSCPS_CONN_INFO_CHARACTERISTIC_UUID,
        NOTIFY_PERM_UNSEC,
        (ATT_VAL_LOC_USER | ATT_UUID_TYPE_SET(UUID_TYPE_128)),
        THSCPS_CONN_INFO_VAL_LEN
    },
    // THS Test Connection Characteristic - Client Characteristic Configuration Descriptor
    [THSCPS_IDX_CONN_INFO_CFG]  = {ATT_128_CLIENT_CHAR_CFG, READ_PERM_UNSEC | WRITE_REQ_PERM_UNSEC, 0, 0}
};

/**@brief THSCPS Service interface required by profile manager. */
static ble_prf_manager_cbs_t thscps_mgr_cbs = {
    (prf_init_func_t)thscps_init,
    NULL,
    NULL
};

/**@brief THSCPS GATT Server Callbacks. */
static gatts_prf_cbs_t thscps_gatts_cbs = {
    thscps_read_att_cb,
    thscps_write_att_cb,
    NULL,
    NULL,
    thscps_cccd_set_cb
};

/**@brief THSCPS Service Information. */
static const prf_server_info_t thscps_prf_info = {
    .max_connection_nb = THSCPS_CONNECTION_MAX,
    .manager_cbs       = &thscps_mgr_cbs,
    .gatts_prf_cbs     = &thscps_gatts_cbs
};

/*
 * LOCAL FUNCTION DEFINITIONS
 *****************************************************************************************
 */
/**
 *****************************************************************************************
 * @brief Initialize THS Control Point service and create database in ATT.
 *
 * @return Error code to know if profile initialization succeed or not.
 *****************************************************************************************
 */
static sdk_err_t thscps_init(void)
{
    const uint8_t     thscps_svc_uuid[] = {THSCPS_SERVICE_UUID};
    uint16_t          start_hdl         = PRF_INVALID_HANDLE;
    sdk_err_t         error_code;
    gatts_create_db_t gatts_db;

    error_code = memset_s(&gatts_db, sizeof(gatts_db), 0, sizeof(gatts_db));
    if (error_code < 0) {
        return error_code;
    }

    gatts_db.shdl                  = &start_hdl;
    gatts_db.uuid                  = thscps_svc_uuid;
    gatts_db.attr_tab_cfg          = NULL;
    gatts_db.max_nb_attr           = THSCPS_IDX_NB;
    gatts_db.srvc_perm             = SRVC_UUID_TYPE_SET(UUID_TYPE_128);
    gatts_db.attr_tab_type         = SERVICE_TABLE_TYPE_128;
    gatts_db.attr_tab.attr_tab_128 = thscps_attr_tab;

    error_code = ble_gatts_srvc_db_create(&gatts_db);
    if (SDK_SUCCESS == error_code) {
        s_thscps_env.start_hdl = *gatts_db.shdl;
    }

    return error_code;
}

/**
 *****************************************************************************************
 * @brief Handles reception of the attribute info request message.
 *
 * @param[in] conn_idx: Connection index.
 * @param[in] p_param:  Pointer to the parameters of the read request.
 *****************************************************************************************
 */
static void  thscps_read_att_cb(uint8_t conn_idx, const gatts_read_req_cb_t *p_param)
{
    gatts_read_cfm_t cfm;
    uint8_t          handle    = p_param->handle;
    uint8_t          tab_index = 0;

    tab_index  = prf_find_idx_by_handle(handle, s_thscps_env.start_hdl, THSCPS_IDX_NB, (uint8_t *)&s_thscps_char_mask);
    cfm.handle = handle;
    cfm.status = BLE_SUCCESS;

    switch (tab_index) {
        case THSCPS_IDX_THS_CTRL_PT_CFG:
            cfm.length = sizeof(uint16_t);
            cfm.value  = (uint8_t *)&s_thscps_env.ctrl_pt_ind_cfg[conn_idx];
            break;

        case THSCPS_IDX_TEST_SETTING_CFG:
            cfm.length = sizeof(uint16_t);
            cfm.value  = (uint8_t *)&s_thscps_env.test_setting_ntf_cfg[conn_idx];
            break;

        case THSCPS_IDX_TEST_INFO_CFG:
            cfm.length = sizeof(uint16_t);
            cfm.value  = (uint8_t *)&s_thscps_env.test_info_ntf_cfg[conn_idx];
            break;

        case THSCPS_IDX_CONN_INFO_CFG:
            cfm.length = sizeof(uint16_t);
            cfm.value  = (uint8_t *)&s_thscps_env.conn_info_ntf_cfg[conn_idx];
            break;

        default:
            cfm.length = 0;
            cfm.status = BLE_ATT_ERR_INVALID_HANDLE;
            break;
    }

    ble_gatts_read_cfm(conn_idx, &cfm);
}

/**
 *****************************************************************************************
 * @brief Handles reception of the write request.
 *
 * @param[in] conn_idx: Connection index.
 * @param[in] p_param:  Pointer to the parameters of the write request.
 *****************************************************************************************
 */
static void thscps_write_att_cb(uint8_t conn_idx, const gatts_write_req_cb_t *p_param)
{
    uint8_t              handle           = p_param->handle;
    uint8_t              tab_index        = 0;
    uint16_t             cccd_value       = 0;
    bool                 is_ctrl_pt_wr    = false;
    thscps_rsp_val_t     rsp_val;
    thscps_evt_t         event;
    gatts_write_cfm_t    cfm;

    tab_index      = prf_find_idx_by_handle(handle, s_thscps_env.start_hdl,
                                            THSCPS_IDX_NB, (uint8_t *)&s_thscps_char_mask);
    cfm.handle     = handle;
    cfm.status     = BLE_SUCCESS;
    event.conn_idx = conn_idx;
    event.evt_type = THSCPS_EVT_INVALID;

    switch (tab_index) {
        case THSCPS_IDX_THS_CTRL_PT_VAL:
            is_ctrl_pt_wr = true;
            break;

        case THSCPS_IDX_THS_CTRL_PT_CFG:
            cccd_value = le16toh(&p_param->value[0]);
            event.evt_type = ((PRF_CLI_START_IND == cccd_value) ? \
                              THSCPS_EVT_CTRL_PT_IND_ENABLE : \
                              THSCPS_EVT_CTRL_PT_IND_DISABLE);
            s_thscps_env.ctrl_pt_ind_cfg[conn_idx] = cccd_value;
            break;

        case THSCPS_IDX_TEST_SETTING_VAL:
            event.evt_type = THSCPS_EVT_SETTING_SET;
            event.param.setting_info.length = p_param->length;
            event.param.setting_info.p_data = p_param->value;
            break;

        case THSCPS_IDX_TEST_SETTING_CFG:
            cccd_value = le16toh(&p_param->value[0]);
            event.evt_type = ((PRF_CLI_START_NTF == cccd_value) ? \
                              THSCPS_EVT_TSET_SET_NTF_ENABLE : \
                              THSCPS_EVT_TSET_SET_NTF_DISABLE);
            s_thscps_env.test_setting_ntf_cfg[conn_idx] = cccd_value;
            break;

        case THSCPS_IDX_TEST_INFO_CFG:
            cccd_value = le16toh(&p_param->value[0]);
            event.evt_type = ((PRF_CLI_START_NTF == cccd_value) ? \
                              THSCPS_EVT_TSET_INFO_NTF_ENABLE : \
                              THSCPS_EVT_TSET_INFO_NTF_DISABLE);
            s_thscps_env.test_info_ntf_cfg[conn_idx] = cccd_value;
            break;

        case THSCPS_IDX_CONN_INFO_CFG:
            cccd_value = le16toh(&p_param->value[0]);
            event.evt_type = ((PRF_CLI_START_NTF == cccd_value) ? \
                              THSCPS_EVT_CONN_INFO_NTF_ENABLE : \
                              THSCPS_EVT_CONN_INFO_NTF_DISABLE);
            s_thscps_env.conn_info_ntf_cfg[conn_idx] = cccd_value;
            break;

        default:
            cfm.status = BLE_ATT_ERR_INVALID_HANDLE;
            break;
    }

    ble_gatts_write_cfm(conn_idx, &cfm);

    if (is_ctrl_pt_wr) {
        thscps_ctrl_pt_handler(conn_idx, p_param);
    } else if (THSCPS_EVT_SETTING_SET == event.evt_type &&
               THSCPS_TEST_STARTED == s_thscps_env.test_state &&
               p_param->value[0] != THSCPS_SETTINGS_TYPE_TOGGLE) {
        rsp_val.cmd_id       = p_param->value[0];
        rsp_val.conn_idx     = p_param->value[1];
        rsp_val.status       = THSCPS_RSP_ID_STATUS_ERR;

        thscps_test_setting_rsp_send(conn_idx, &rsp_val);
    } else if (THSCPS_EVT_INVALID != event.evt_type && s_thscps_env.evt_handler) {
        s_thscps_env.evt_handler(&event);
    }
}

/**
 *****************************************************************************************
 * @brief Handles reception of the cccd recover request.
 *
 * @param[in]: conn_idx:   Connection index
 * @param[in]: handle:     The handle of cccd attribute.
 * @param[in]: cccd_value: The value of cccd attribute.
 *****************************************************************************************
 */
static void thscps_cccd_set_cb(uint8_t conn_idx, uint16_t handle, uint16_t cccd_value)
{
    uint8_t       tab_index = 0;
    thscps_evt_t  event;

    event.conn_idx = conn_idx;
    event.evt_type = THSCPS_EVT_INVALID;

    if (!prf_is_cccd_value_valid(cccd_value)) {
        return;
    }

    tab_index  = prf_find_idx_by_handle(handle, s_thscps_env.start_hdl, THSCPS_IDX_NB, (uint8_t *)&s_thscps_char_mask);

    switch (tab_index) {
        case THSCPS_IDX_THS_CTRL_PT_CFG:
            event.evt_type = ((PRF_CLI_START_IND == cccd_value) ? \
                              THSCPS_EVT_CTRL_PT_IND_ENABLE : \
                              THSCPS_EVT_CTRL_PT_IND_DISABLE);
            s_thscps_env.ctrl_pt_ind_cfg[conn_idx] = cccd_value;
            break;

        case THSCPS_IDX_TEST_SETTING_CFG:
            event.evt_type = ((PRF_CLI_START_NTF == cccd_value) ? \
                              THSCPS_EVT_TSET_SET_NTF_ENABLE : \
                              THSCPS_EVT_TSET_SET_NTF_DISABLE);
            s_thscps_env.test_setting_ntf_cfg[conn_idx] = cccd_value;
            break;

        case THSCPS_IDX_TEST_INFO_CFG:
            event.evt_type = ((PRF_CLI_START_NTF == cccd_value) ? \
                              THSCPS_EVT_TSET_INFO_NTF_ENABLE : \
                              THSCPS_EVT_TSET_INFO_NTF_DISABLE);
            s_thscps_env.test_info_ntf_cfg[conn_idx] = cccd_value;
            break;

        case THSCPS_IDX_CONN_INFO_CFG:
            event.evt_type = ((PRF_CLI_START_NTF == cccd_value) ? \
                              THSCPS_EVT_CONN_INFO_NTF_ENABLE : \
                              THSCPS_EVT_CONN_INFO_NTF_DISABLE);
            s_thscps_env.conn_info_ntf_cfg[conn_idx] = cccd_value;
            break;

        default:
            break;
    }

    if (s_thscps_env.evt_handler) {
        s_thscps_env.evt_handler(&event);
    }
}

/**
 *****************************************************************************************
 * @brief Handle THS Control Point.
 *
 * @param[in] conn_idx Connection index.
 * @param[in] p_param: Pointer to the parameters of the write request.
 *****************************************************************************************
 */
static void thscps_ctrl_pt_handler(uint8_t conn_idx, const gatts_write_req_cb_t *p_param)
{
    thscps_evt_t        event;
    thscps_ctrl_pt_id_t ctrl_pt_id;

    ctrl_pt_id = (thscps_ctrl_pt_id_t)p_param->value[0];

    if (THSCPS_TEST_STARTED == s_thscps_env.test_state) {
        thscps_ctrl_pt_fail_rsp_send(conn_idx, ctrl_pt_id, THSCPS_RSP_ID_STATUS_ERR);
        return;
    }

    event.conn_idx = conn_idx;
    event.evt_type = THSCPS_EVT_INVALID;

    switch (ctrl_pt_id) {
        case THSCPS_CTRL_PT_TEST_ROLE:
            if (p_param->value[1] != THSCPS_TEST_ROLE_SLAVE && p_param->value[1] != THSCPS_TEST_ROLE_MASTER) {
                thscps_ctrl_pt_fail_rsp_send(conn_idx, ctrl_pt_id, THSCPS_RSP_ID_PARAM_ERR);
            } else {
                s_thscps_env.curr_role = (thscps_test_role_t)p_param->value[1];
                event.evt_type       = THSCPS_EVT_TEST_ROLE_SET;
                event.param.test_role = (thscps_test_role_t)p_param->value[1];
            }
            break;

        case THSCPS_CTRL_PT_ADV_PARAM:
            if (THSCPS_TEST_ROLE_SLAVE == s_thscps_env.curr_role) {
                event.evt_type                 = THSCPS_EVT_ADV_PRAM_SET;
                event.param.adv_param.phy      = (thscps_adv_phy_t)(p_param->value[1]);
                event.param.adv_param.interval = le16toh(&p_param->value[INDEX_2]);
                event.param.adv_param.duration = le16toh(&p_param->value[INDEX_4]);
                event.param.adv_param.tx_power = p_param->value[INDEX_6] == 1 ? \
                                                 0 - p_param->value[INDEX_7] : p_param->value[INDEX_7];
            } else {
                thscps_ctrl_pt_fail_rsp_send(conn_idx, ctrl_pt_id, THSCPS_RSP_ID_TEST_ROLE_ERR);
            }
            break;

        case THSCPS_CTRL_PT_ADV_ACTION:
            if (THSCPS_TEST_ROLE_SLAVE == s_thscps_env.curr_role) {
                event.evt_type         = THSCPS_EVT_ADV_ACTION;
                event.param.action_set = p_param->value[1];
            } else {
                thscps_ctrl_pt_fail_rsp_send(conn_idx, ctrl_pt_id, THSCPS_RSP_ID_TEST_ROLE_ERR);
            }
            break;

        case THSCPS_CTRL_PT_SCAN_ACTION:
            if (THSCPS_TEST_ROLE_MASTER == s_thscps_env.curr_role) {
                event.evt_type         = THSCPS_EVT_SCAN_ACTION;
                event.param.action_set = p_param->value[1];
            } else {
                thscps_ctrl_pt_fail_rsp_send(conn_idx, ctrl_pt_id, THSCPS_RSP_ID_TEST_ROLE_ERR);
            }
            break;

        default:
            thscps_ctrl_pt_fail_rsp_send(conn_idx, ctrl_pt_id, THSCPS_RSP_ID_UNSUPPORT);
            break;
    }

    if (THSCPS_EVT_INVALID != event.evt_type && s_thscps_env.evt_handler) {
        s_thscps_env.evt_handler(&event);
    }
}

/**
 *****************************************************************************************
 * @brief Encode THS Control Point Response value.
 *
 * @param[in]  p_rsp_val:      Pointer to Control Point Response value.
 * @param[out] p_encoded_buff: Pointer to buffer encoded.
 *
 * @return Length of encoded
 *****************************************************************************************
 */
static uint16_t thscps_ctrl_pt_rsp_encode(thscps_rsp_val_t *p_rsp_val, uint8_t *p_encoded_buff)
{
    uint16_t length = 0;

    p_encoded_buff[length++] = THSCPS_CTRL_PT_RSP_CODE;
    p_encoded_buff[length++] = p_rsp_val->cmd_id;
    p_encoded_buff[length++] = p_rsp_val->status;

    return length;
}

/**
 *****************************************************************************************
 * @brief Encode THS Settings Response value.
 *
 * @param[in]  p_rsp_val:      Pointer to Settings Response value.
 * @param[out] p_encoded_buff: Pointer to buffer encoded.
 *
 * @return Length of encoded
 *****************************************************************************************
 */
static uint16_t thscps_settings_rsp_encode(thscps_rsp_val_t *p_rsp_val, uint8_t *p_encoded_buff)
{
    uint16_t length = 0;

    p_encoded_buff[length++] = THSCPS_CTRL_PT_RSP_CODE;
    p_encoded_buff[length++] = p_rsp_val->cmd_id;
    p_encoded_buff[length++] = p_rsp_val->conn_idx;
    p_encoded_buff[length++] = p_rsp_val->status;

    return length;
}

/**
 *****************************************************************************************
 * @brief Encode THS test information.
 *
 * @param[in]  p_ctrl_val_pt:  Pointer to test information.
 * @param[out] p_encoded_buff: Pointer to buffer encoded.
 *
 * @return Length of encoded
 *****************************************************************************************
 */
static uint16_t thscps_test_info_encode(thscps_test_info_t *p_test_info, uint8_t *p_encoded_buff)
{
    uint16_t length = 0;

    // RSSI
    if (p_test_info->rssi > 0) {
        p_encoded_buff[length++] = 0;
        p_encoded_buff[length++] = p_test_info->rssi;
    } else {
        p_encoded_buff[length++] = 1;
        p_encoded_buff[length++] = 0 - p_test_info->rssi;
    }

    // Right rate
    p_encoded_buff[length++] = p_test_info->right_rate;

    // Throughput instant value
    p_encoded_buff[length++] = LO_U16(p_test_info->instant_val);
    p_encoded_buff[length++] = HI_U16(p_test_info->instant_val);

    // Throughput average value
    p_encoded_buff[length++] = LO_U16(p_test_info->average_val);
    p_encoded_buff[length++] = HI_U16(p_test_info->average_val);

    // Total recieved packets value in one second
    p_encoded_buff[length++] = LO_U16(p_test_info->packets_val);
    p_encoded_buff[length++] = HI_U16(p_test_info->packets_val);

    return length;
}

/**
 *****************************************************************************************
 * @brief Encode THS conn information.
 *
 * @param[in]  p_ctrl_val_pt:  Pointer to test information.
 * @param[out] p_encoded_buff: Pointer to buffer encoded.
 *
 * @return Length of encoded
 *****************************************************************************************
 */
static uint16_t thscps_conn_info_encode(thscps_test_conn_info_t *p_conn_info, uint8_t *p_encoded_buff)
{
    uint16_t length = 0;

    // CI
    p_encoded_buff[length++] = LO_U16(p_conn_info->ci);
    p_encoded_buff[length++] = HI_U16(p_conn_info->ci);

    // PDU
    p_encoded_buff[length++] = LO_U16(p_conn_info->pdu);
    p_encoded_buff[length++] = HI_U16(p_conn_info->pdu);

    // MTU
    p_encoded_buff[length++] = LO_U16(p_conn_info->mtu);
    p_encoded_buff[length++] = HI_U16(p_conn_info->mtu);

    // PHY
    p_encoded_buff[length++] = p_conn_info->tx_phy;
    p_encoded_buff[length++] = p_conn_info->rx_phy;

    // TX POWER
    if (p_conn_info->tx_power > 0) {
        p_encoded_buff[length++] = 0;
        p_encoded_buff[length++] = p_conn_info->tx_power;
    } else {
        p_encoded_buff[length++] = 1;
        p_encoded_buff[length++] = 0 - p_conn_info->tx_power;
    }

    // THS Mode
    p_encoded_buff[length++] = p_conn_info->ths_mode;

    return length;
}

/**
 *****************************************************************************************
 * @brief Send Control Point Fail Response if its indicaiton has been enabled.
 *
 * @param[in] conn_idx:  Connnection index.
* @param[in] cmd_id:     Control Point ID.
 * @param[in] status:    Status response ID.
 *
 * @return Result of indicate value
 *****************************************************************************************
 */
static sdk_err_t thscps_ctrl_pt_fail_rsp_send(uint8_t conn_idx, thscps_ctrl_pt_id_t cmd_id, thscps_status_rsp_t status)
{
    uint8_t          encoded_ctrl_pt_rsp[3];
    gatts_noti_ind_t ctrl_pt_rsp_ind;

    encoded_ctrl_pt_rsp[INDEX_0] = THSCPS_CTRL_PT_RSP_CODE;
    encoded_ctrl_pt_rsp[INDEX_1] = cmd_id;
    encoded_ctrl_pt_rsp[INDEX_2] = status;

    if (PRF_CLI_START_IND == s_thscps_env.ctrl_pt_ind_cfg[conn_idx]) {
        ctrl_pt_rsp_ind.type    = BLE_GATT_INDICATION;
        ctrl_pt_rsp_ind.handle  = prf_find_handle_by_idx(THSCPS_IDX_THS_CTRL_PT_VAL,
                                                         s_thscps_env.start_hdl,
                                                         (uint8_t *)&s_thscps_char_mask);
        ctrl_pt_rsp_ind.length  = LEN_3;
        ctrl_pt_rsp_ind.value   = encoded_ctrl_pt_rsp;

        return ble_gatts_noti_ind(conn_idx, &ctrl_pt_rsp_ind);
    }

    return SDK_ERR_IND_DISABLED;
}

/*
 * GLOBAL FUNCTION DEFINITIONS
 *****************************************************************************************
 */
sdk_err_t thscps_ctrl_pt_rsp_send(uint8_t conn_idx, thscps_rsp_val_t *p_rsp_val)
{
    uint8_t          encoded_ctrl_pt_rsp[THSCPS_CTRL_PT_VAL_LEN];
    gatts_noti_ind_t ctrl_pt_rsp_ind;
    uint16_t         encoded_length;

    encoded_length =thscps_ctrl_pt_rsp_encode(p_rsp_val, encoded_ctrl_pt_rsp);

    if (PRF_CLI_START_IND == s_thscps_env.ctrl_pt_ind_cfg[conn_idx]) {
        ctrl_pt_rsp_ind.type    = BLE_GATT_INDICATION;
        ctrl_pt_rsp_ind.handle  = prf_find_handle_by_idx(THSCPS_IDX_THS_CTRL_PT_VAL,
                                                         s_thscps_env.start_hdl,
                                                         (uint8_t *)&s_thscps_char_mask);
        ctrl_pt_rsp_ind.length  = encoded_length;
        ctrl_pt_rsp_ind.value   = encoded_ctrl_pt_rsp;

        return ble_gatts_noti_ind(conn_idx, &ctrl_pt_rsp_ind);
    }

    return SDK_ERR_IND_DISABLED;
}

sdk_err_t thscps_test_setting_rsp_send(uint8_t conn_idx, thscps_rsp_val_t *p_rsp_val)
{
    uint8_t          encoded_rsp[THSCPS_CTRL_PT_VAL_LEN];
    gatts_noti_ind_t test_setting_rsp_ntf;
    uint16_t         encoded_length;

    encoded_length =thscps_settings_rsp_encode(p_rsp_val, encoded_rsp);

    if (PRF_CLI_START_NTF == s_thscps_env.test_setting_ntf_cfg[conn_idx]) {
        test_setting_rsp_ntf.type    = BLE_GATT_NOTIFICATION;
        test_setting_rsp_ntf.handle  = prf_find_handle_by_idx(THSCPS_IDX_TEST_SETTING_VAL,
                                                              s_thscps_env.start_hdl,
                                                              (uint8_t *)&s_thscps_char_mask);
        test_setting_rsp_ntf.length  = encoded_length;
        test_setting_rsp_ntf.value   = encoded_rsp;

        return ble_gatts_noti_ind(conn_idx, &test_setting_rsp_ntf);
    }

    return SDK_ERR_NTF_DISABLED;
}

sdk_err_t thscps_test_info_send(uint8_t conn_idx, thscps_test_info_t *p_test_info)
{
    uint8_t          encoded_test_info[THSCPS_TEST_INFO_VAL_LEN];
    gatts_noti_ind_t test_info_ntf;
    uint16_t         encoded_length;

    encoded_length = thscps_test_info_encode(p_test_info, encoded_test_info);

    if (PRF_CLI_START_NTF == s_thscps_env.test_info_ntf_cfg[conn_idx]) {
        test_info_ntf.type    = BLE_GATT_NOTIFICATION;
        test_info_ntf.handle  = prf_find_handle_by_idx(THSCPS_IDX_TEST_INFO_VAL,
                                                       s_thscps_env.start_hdl,
                                                       (uint8_t *)&s_thscps_char_mask);
        test_info_ntf.length  = encoded_length;
        test_info_ntf.value   = encoded_test_info;

        return ble_gatts_noti_ind(conn_idx, &test_info_ntf);
    }

    return SDK_ERR_NTF_DISABLED;
}

sdk_err_t thscps_conn_info_send(uint8_t conn_idx, thscps_test_conn_info_t *p_conn_info)
{
    uint8_t          encoded_conn_info[THSCPS_CONN_INFO_VAL_LEN];
    gatts_noti_ind_t test_info_ntf;
    uint16_t         encoded_length;

    encoded_length = thscps_conn_info_encode(p_conn_info, encoded_conn_info);

    if (PRF_CLI_START_NTF == s_thscps_env.conn_info_ntf_cfg[conn_idx]) {
        test_info_ntf.type    = BLE_GATT_NOTIFICATION;
        test_info_ntf.handle  = prf_find_handle_by_idx(THSCPS_IDX_CONN_INFO_VAL,
                                                       s_thscps_env.start_hdl,
                                                       (uint8_t *)&s_thscps_char_mask);
        test_info_ntf.length  = encoded_length;
        test_info_ntf.value   = encoded_conn_info;

        return ble_gatts_noti_ind(conn_idx, &test_info_ntf);
    }

    return SDK_ERR_NTF_DISABLED;
}

sdk_err_t thscps_service_init(thscps_evt_handler_t evt_handler)
{
    s_thscps_env.evt_handler = evt_handler;

    return ble_server_prf_add(&thscps_prf_info);
}

void thscps_test_state_set(thscps_test_state_t test_state)
{
    s_thscps_env.test_state = test_state;
}

