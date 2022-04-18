/**
 *******************************************************************************
 *
 * @file hrs.c
 *
 * @brief Heart Rate Profile Sensor implementation.
 *
 *******************************************************************************
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
#include "escs.h"
#include "ble_prf_types.h"
#include "ble_prf_utils.h"
#include "utility.h"

/*
 * STRUCT DEFINE
 *******************************************************************************
 */
/**@brief EddyStone Configuration Service environment variable. */
typedef struct {
    ble_escs_init_params_t escs_init;       /**< EddyStone Configuration Service initialization variables. */
    uint16_t               start_hdl;       /**< EddyStone Configuration Service start handle. */
} escs_env_t;

/*
* LOCAL FUNCTION DECLARATION
*****************************************************************************************
*/
static sdk_err_t   escs_init (void);
static sdk_err_t   escs_write_att_cb (uint8_t conn_idx, const gatts_write_req_cb_t *p_param);
static sdk_err_t   escs_read_att_cb (uint8_t conn_idx, const gatts_read_req_cb_t *p_param);
static void        escs_gatts_cmp_cb (uint8_t conn_idx, const gatt_op_cmp_evt_t *p_param);

/*
 * LOCAL VARIABLE DEFINITIONS
 *******************************************************************************
 */
uint16_t    my_escs_service_start_handle;

/**@brief ESCS Information. */
static escs_env_t s_escs_env;

static const attm_desc_128_t escs_att_db[ESCSS_IDX_NB] = {
    [ESEC_IDX_SVC] = {ATT_128_PRIMARY_SERVICE, READ_PERM_UNSEC, 0, 0},
    [ESCS_BROADCAST_CAP_RD_CHAR] = {ATT_128_CHARACTERISTIC, READ_PERM_UNSEC, 0, 0},
    [ESCS_BROADCAST_CAP_RD_VALUE]  = {BLE_UUID_ESCS_BROADCAST_CAP_CHAR, READ_PERM_UNSEC,
                                      ATT_VAL_LOC_USER | ATT_UUID_TYPE_SET (UUID_TYPE_128),
                                      ESEC_CAP_DEFAULT_LEN + ESCS_NUM_OF_SUPPORTED_TX_POWER},
    [ESCS_ACTIVE_SLOT_RW_CHAR] = {ATT_128_CHARACTERISTIC, READ_PERM_UNSEC, 0, 0},
    [ESCS_ACTIVE_SLOT_RW_VALUE]  = {BLE_UUID_ESCS_ACTIVE_SLOT_CHAR, READ_PERM_UNSEC | WRITE_REQ_PERM_UNSEC,
                                    ATT_VAL_LOC_USER | ATT_UUID_TYPE_SET (UUID_TYPE_128), 1},
    [ESCS_ADV_INTERVAL_RW_CHAR] = {ATT_128_CHARACTERISTIC, READ_PERM_UNSEC, 0, 0},
    [ESCS_ADV_INTERVAL_RW_VALUE]  = {BLE_UUID_ESCS_ADV_INTERVAL_CHAR, READ_PERM_UNSEC | WRITE_REQ_PERM_UNSEC,
                                     ATT_VAL_LOC_USER | ATT_UUID_TYPE_SET (UUID_TYPE_128), 2},
    [ESCS_RADIO_TX_PWR_RW_CHAR] = {ATT_128_CHARACTERISTIC, READ_PERM_UNSEC, 0, 0},
    [ESCS_RADIO_TX_PWR_RW_VALUE]  = {BLE_UUID_ESCS_RADIO_TX_PWR_CHAR, READ_PERM_UNSEC | WRITE_REQ_PERM_UNSEC,
                                     ATT_VAL_LOC_USER | ATT_UUID_TYPE_SET (UUID_TYPE_128), 1},
    [ESCS_ADV_TX_PWR_RW_CHAR] = {ATT_128_CHARACTERISTIC, READ_PERM_UNSEC, 0, 0},
    [ESCS_ADV_TX_PWR_RW_VALUE]  = {BLE_UUID_ESCS_ADV_TX_PWR_CHAR, READ_PERM_UNSEC | WRITE_REQ_PERM_UNSEC,
                                   TT_VAL_LOC_USER | ATT_UUID_TYPE_SET (UUID_TYPE_128), 1},
    [ESCS_LOCK_STATE_RW_CHAR] = {ATT_128_CHARACTERISTIC, READ_PERM_UNSEC, 0, 0},
    [ESCS_LOCK_STATE_RW_VALUE]  = {BLE_UUID_ESCS_LOCK_STATE_CHAR, READ_PERM_UNSEC | WRITE_REQ_PERM_UNSEC,
                                   ATT_VAL_LOC_USER | ATT_UUID_TYPE_SET (UUID_TYPE_128), ESCS_LOCK_CODE_WRITE_LENGTH},
    [ESCS_UNLOCK_RW_CHAR] = {ATT_128_CHARACTERISTIC, READ_PERM_UNSEC, 0, 0},
    [ESCS_UNLOCK_RW_VALUE]  = {BLE_UUID_ESCS_UNLOCK_CHAR, READ_PERM_UNSEC | WRITE_REQ_PERM_UNSEC,
                               ATT_VAL_LOC_USER | ATT_UUID_TYPE_SET (UUID_TYPE_128), ESCS_AES_KEY_SIZE},
    [ESCS_PUBLIC_ECDH_KEY_RD_CHAR] = {ATT_128_CHARACTERISTIC, READ_PERM_UNSEC, 0, 0},
    [ESCS_PUBLIC_ECDH_KEY_RD_VALUE]  = {BLE_UUID_ESCS_PUBLIC_ECDH_KEY_CHAR, READ_PERM_UNSEC,
                                        ATT_VAL_LOC_USER | ATT_UUID_TYPE_SET (UUID_TYPE_128), ESCS_ECDH_KEY_SIZE},
    [ESCS_EID_ID_KEY_RD_CHAR] = {ATT_128_CHARACTERISTIC, READ_PERM_UNSEC, 0, 0},
    [ESCS_EID_ID_KEY_RD_VALUE]  = {BLE_UUID_ESCS_EID_ID_KEY_CHAR, READ_PERM_UNSEC,
                                   ATT_VAL_LOC_USER | ATT_UUID_TYPE_SET (UUID_TYPE_128), ESCS_AES_KEY_SIZE},
    [ESCS_RW_ADV_SLOT_RW_CHAR] = {ATT_128_CHARACTERISTIC, READ_PERM_UNSEC, 0, 0},
    [ESCS_RW_ADV_SLOT_RW_VALUE]  = {BLE_UUID_ESCS_RW_ADV_SLOT_CHAR, READ_PERM_UNSEC | WRITE_REQ_PERM_UNSEC,
                                    ATT_VAL_LOC_USER | ATT_UUID_TYPE_SET (UUID_TYPE_128),
                                    ESCS_ADV_SLOT_CHAR_LENGTH_MAX},
    [ESCS_FACTORY_RESET_SET_CHAR] = {ATT_128_CHARACTERISTIC, READ_PERM_UNSEC, 0, 0},
    [ESCS_FACTORY_RESET_SET_VALUE]  = {BLE_UUID_ESCS_FACTORY_RESET_CHAR, WRITE_REQ_PERM_UNSEC,
                                       ATT_VAL_LOC_USER | ATT_UUID_TYPE_SET (UUID_TYPE_128), 1},
    [ESCS_REMAIN_CONNECTABLE_RW_CHAR] = {ATT_128_CHARACTERISTIC, READ_PERM_UNSEC, 0, 0},
    [ESCS_REMAIN_CONNECTABLE_RW_VALUE]  = {BLE_UUID_ESCS_REMAIN_CONNECTABLE_CHAR,
                                           READ_PERM_UNSEC | WRITE_REQ_PERM_UNSEC,
                                           ATT_VAL_LOC_USER | ATT_UUID_TYPE_SET (UUID_TYPE_128), 1},
};

/**@brief ESCS interface required by profile manager. */
static ble_prf_manager_cbs_t escs_tack_cbs = {
    (prf_init_func_t) escs_init,
    NULL,
    NULL
};

/**@brief ESCS GATT server Callbacks. */
static gatts_prf_cbs_t escs_cb_func = {
    escs_read_att_cb,
    escs_write_att_cb,
    NULL,
    escs_gatts_cmp_cb,
};

/**@brief ESCS Information. */
static const prf_server_info_t escs_prf_info = {
    .max_connection_nb = ESCS_CONNECTION_MAX,
    .manager_cbs       = &escs_tack_cbs,
    .gatts_prf_cbs          = &escs_cb_func
};

/*
 * LOCAL FUNCTION DEFINITIONS
 *****************************************************************************************
 */
/**
 *****************************************************************************************
 * @brief Initialize Immediate Alert Service and create DB in ATT.
 *
 * @return Error code to know if service initialization succeed or not.
 *****************************************************************************************
 */
static sdk_err_t   escs_init (void)
{
    const uint8_t     escs_svc_uuid[] = {EDDYSTONE_CONFIGURATION_SERVICE_UUID};
    sdk_err_t         error_code         = SDK_SUCCESS;
    uint16_t          start_hdl          = 0;
    gatts_create_db_t gatts_db;

    uint32_t escs_char_mask = ESCS_DEFAULT_CHAR_MASK;

    error_code = memset_s(&gatts_db, sizeof(gatts_db), 0, sizeof(gatts_db));
    if (error_code < 0) {
        return error_code;
    }
    gatts_db.shdl                  = &start_hdl;
    gatts_db.uuid                  = escs_svc_uuid;
    gatts_db.attr_tab_cfg          = (uint8_t *) (&escs_char_mask);
    gatts_db.max_nb_attr           = ESCSS_IDX_NB;
    gatts_db.srvc_perm             = SRVC_UUID_TYPE_SET (UUID_TYPE_128);
    gatts_db.attr_tab_type         = SERVICE_TABLE_TYPE_128;
    gatts_db.attr_tab.attr_tab_128 = escs_att_db;

    error_code = ble_gatts_srvc_db_create (&gatts_db);
    ERROR_CODE_PRINT (error_code, BLE_SUCCESS);

    if (SDK_SUCCESS == error_code) {
        my_escs_service_start_handle = * (gatts_db.shdl);
    }

    return error_code;
}

/**
 *****************************************************************************************
 * @brief Handles reception of the attribute info request message.
 *
 * @param[in] conn_idx: Connection index
 * @param[in] p_param:  Pointer to the parameters of the read request.
 *
 * @retval ::BLE_SDK_SUCCESS Send write confirm status to stack successfully.
 * @retval ::BLE_SDK_ERR_BAD_PARAM Conidx is invalid or param is NULL.
 *
 *****************************************************************************************
 */
static sdk_err_t   escs_read_att_cb (uint8_t conn_idx, const gatts_read_req_cb_t *p_param)
{
    sdk_err_t   error_code;
    uint8_t          handle    = p_param->handle;
    uint8_t          tab_index = 0;
    gatts_read_cfm_t cfm;
    uint8_t att_data_array[ESEC_MAX_READ_LEN];
    uint32_t escs_char_mask = ESCS_DEFAULT_CHAR_MASK;
    tab_index = prf_find_idx_by_handle (handle,
                                        my_escs_service_start_handle,
                                        ESCSS_IDX_NB,
                                        (uint8_t *) (&escs_char_mask));
    cfm.handle = handle;
    cfm.status = BLE_SUCCESS;
    cfm.value = att_data_array;

    escs_attribute_value_set (tab_index, &cfm, s_escs_env.escs_init.lock_state);

    error_code = ble_gatts_read_cfm (conn_idx, &cfm);
    RETURN_IF_ERROR (error_code, BLE_SUCCESS);

    return error_code;
}

/**
 *****************************************************************************************
 * @brief Handles reception of the write request.
 *
 * @param[in] conn_idx: Connection index
 * @param[in] p_param:  Pointer to the parameters of the write request.
 *
 * @retval ::BLE_SDK_SUCCESS Send write confirm status to stack successfully.
 * @retval ::BLE_SDK_ERR_BAD_PARAM Conidx is invalid or param is NULL.
 *
 *****************************************************************************************
 */
static sdk_err_t   escs_write_att_cb (uint8_t conn_idx, const gatts_write_req_cb_t *p_param)
{
    uint8_t           handle     = p_param->handle;
    uint8_t           tab_index  = 0;
    gatts_write_cfm_t cfm;
    cfm.handle = handle;
    cfm.status = BLE_SUCCESS;
    uint32_t escs_char_mask = ESCS_DEFAULT_CHAR_MASK;

    tab_index = prf_find_idx_by_handle (handle,
                                        my_escs_service_start_handle,
                                        ESCSS_IDX_NB,
                                        (uint8_t *) (&escs_char_mask));

    cfm.status = escs_attribute_value_get (tab_index, (gatts_write_req_cb_t*) p_param, s_escs_env.escs_init.lock_state);
    return ble_gatts_write_cfm (conn_idx, &cfm);
}

/**
 *****************************************************************************************
 * @brief Handles reception of the complete event.
 *
 * @param[in] conn_idx: Connection index
 * @param[in] p_param:  Pointer to the parameters of the complete event.
 *
 *****************************************************************************************
 */
static void escs_gatts_cmp_cb (uint8_t conn_idx, const gatt_op_cmp_evt_t *p_param)
{
}

/*
 * GLOBAL FUNCTION DEFINITIONS
 *****************************************************************************************
 */
void esec_service_init (ble_escs_init_params_t *p_escs_init)
{
    sdk_err_t   error_code;

    error_code = memcpy_s(&s_escs_env.escs_init, sizeof(ble_escs_init_params_t),
                          p_escs_init, sizeof(ble_escs_init_params_t));
    if (error_code < 0) {
        return;
    }

    error_code = ble_server_prf_add (&escs_prf_info);
    ERROR_CODE_PRINT (error_code, BLE_SUCCESS);
}

void set_beacon_locked (void)
{
    s_escs_env.escs_init.lock_state = ESCS_LOCK_STATE_LOCKED;
}

void set_beacon_unlocked (void)
{
    s_escs_env.escs_init.lock_state = ESCS_LOCK_STATE_UNLOCKED;
}

uint8_t es_active_slot_number_get (void)
{
    return s_escs_env.escs_init.active_slot_no;
}

void es_active_slot_number_set (uint8_t slot_no)
{
    s_escs_env.escs_init.active_slot_no = slot_no;
}

uint16_t es_adv_interval_get (void)
{
    if (s_escs_env.escs_init.adv_interval > APP_CONFIG_ADV_INTERVAL_MS_MAX) {
        s_escs_env.escs_init.adv_interval = APP_CONFIG_ADV_INTERVAL_MS_MAX;
    } else if (s_escs_env.escs_init.adv_interval < APP_CONFIG_ADV_FRAME_SPACING_MS_MIN) {
        s_escs_env.escs_init.adv_interval = APP_CONFIG_ADV_FRAME_SPACING_MS_MIN;
    }

    return s_escs_env.escs_init.adv_interval;
}

void es_adv_interval_set (uint16_t adv_interval)
{
    if (adv_interval > APP_CONFIG_ADV_INTERVAL_MS_MAX) {
        s_escs_env.escs_init.adv_interval = APP_CONFIG_ADV_INTERVAL_MS_MAX;
    } else if (adv_interval < APP_CONFIG_ADV_FRAME_SPACING_MS_MIN) {
        s_escs_env.escs_init.adv_interval = APP_CONFIG_ADV_FRAME_SPACING_MS_MIN;
    } else {
        s_escs_env.escs_init.adv_interval = adv_interval;
    }
}

int8_t es_slot_tx_power_get (void)
{
    uint8_t slot_no;
    slot_no = es_active_slot_number_get();
    return s_escs_env.escs_init.slot_tx_power[slot_no];
}

void es_slot_tx_power_set (int8_t tx_power)
{
    uint8_t slot_no;
    int8_t supported_tx[ESCS_NUM_OF_SUPPORTED_TX_POWER] = ESCS_SUPPORTED_TX_POWER;
    int8_t tx_power_suitable_setting;
    int8_t i;
    uint32_t a, b;

    slot_no = es_active_slot_number_get();
    tx_power_suitable_setting = supported_tx[0];

    if (strstr((const char*)supported_tx, (const char*)(&tx_power))) {
        tx_power_suitable_setting = tx_power;
    } else if (tx_power < supported_tx[0]) {
        tx_power_suitable_setting = supported_tx[0];
    } else if (tx_power > supported_tx[ESCS_NUM_OF_SUPPORTED_TX_POWER - 1]) {
        tx_power_suitable_setting = supported_tx[ESCS_NUM_OF_SUPPORTED_TX_POWER - 1];
    } else {
        for (i = 0; i < (ESCS_NUM_OF_SUPPORTED_TX_POWER - 1); i++) {
            if ((tx_power >= supported_tx[i]) && (tx_power <= supported_tx[i + 1])) {
                a = ABS (tx_power - supported_tx[i]);
                b = ABS (tx_power - supported_tx[i + 1]);
                if (a > b) {
                    tx_power_suitable_setting = supported_tx[i + 1];
                } else {
                    tx_power_suitable_setting = supported_tx[i];
                }
                break;
            }
        }
    }

    s_escs_env.escs_init.slot_tx_power[slot_no] = tx_power_suitable_setting;
}

int8_t es_adv_tx_power_get (void)
{
    return s_escs_env.escs_init.adv_tx_power;
}

void es_adv_tx_power_set (int8_t adv_tx_power)
{
    s_escs_env.escs_init.adv_tx_power = adv_tx_power;
}

void es_security_key_set (uint8_t* p_security_key, bool is_eid_write)
{
    uint8_t ret;
#if(APP_IS_EID_SUPPORTED)
    if (is_eid_write) {
        s_escs_env.escs_init.beacon_lock_code.k_scaler = p_security_key[ESCS_AES_KEY_SIZE];
    }
#endif // APP_IS_EID_SUPPORTED
    ret = memcpy_s(s_escs_env.escs_init.beacon_lock_code.security_key, ESCS_AES_KEY_SIZE,
                   p_security_key, ESCS_AES_KEY_SIZE);
    if (ret < 0) {
        return;
    }
    es_nvds_lock_key_set (p_security_key);
}

bool is_active_slot_eid (void)
{
#if(APP_IS_EID_SUPPORTED)
    return true;
#else // APP_IS_EID_SUPPORTED
    return false;
#endif // APP_IS_EID_SUPPORTED
}

void es_adv_remain_connectable_set (bool remain_connectable)
{
    s_escs_env.escs_init.remain_connectable = remain_connectable;
}

bool es_adv_remain_connectable_get (void)
{
    return s_escs_env.escs_init.remain_connectable;
}

void es_public_ecdh_key_get (uint8_t* p_ecdh_key_buf)
{
    uint8_t ret;
#if(APP_IS_EID_SUPPORTED)
    ret = memcpy_s(p_ecdh_key_buf, ESCS_ECDH_KEY_SIZE,
                   s_escs_env.escs_init.eid_slot_data[s_escs_env.escs_init.active_slot_no].pub_ecdh_key,
                   ESCS_ECDH_KEY_SIZE);
    if (ret < 0) {
        return;
    }
#endif // APP_IS_EID_SUPPORTED
}

void es_public_ecdh_key_set (uint8_t* p_ecdh_key_buf)
{
    uint8_t ret;
#if(APP_IS_EID_SUPPORTED)
    ret = memcpy_s(s_escs_env.escs_init.eid_slot_data[s_escs_env.escs_init.active_slot_no].pub_ecdh_key,
                   ESCS_ECDH_KEY_SIZE, p_ecdh_key_buf + 1, ESCS_ECDH_KEY_SIZE);
    if (ret < 0) {
        return;
    }
    s_escs_env.escs_init.eid_slot_data[s_escs_env.escs_init.active_slot_no].k_scaler = \
                p_ecdh_key_buf[ESCS_ECDH_KEY_SIZE + 1];
#endif // APP_IS_EID_SUPPORTED
}

bool es_beacon_has_eid_adv (void)
{
#if(APP_IS_EID_SUPPORTED)
    return true;
#endif // APP_IS_EID_SUPPORTED
    return false;
}

