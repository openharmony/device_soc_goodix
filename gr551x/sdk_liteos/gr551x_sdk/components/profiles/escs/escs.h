/**
 *****************************************************************************************
 *
 * @file escs.h
 *
 * @brief Eddystone Configuration Service API
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
/**
* @addtogroup BLE_SRV BLE Services
* @{
* @brief Definitions and prototypes for the BLE Service interface.
*/
/**
 * @defgroup BLE_SDK_ESCS BLE Eddystone Configuration Service (ESCS)
 * @{
 * @brief Eddystone Configuration Service module.
 */
#ifndef _ESCS_H_
#define _ESCS_H_
#include <stdbool.h>
#include <stdint.h>
#include "es_app_config.h"
#include "es.h"
#include "ble_prf_types.h"
#include "ble_sdk_error.h"
#include "es_utility.h"
#include "es_nvds.h"
#include "es_gatts_read_write.h"

/**
 * @defgroup ESCS_ENUM Enumerations
 * @{
 */
/** @brief ESCS Service Attributes Indexes. */
enum escs_attr_idx_t {
    ESEC_IDX_SVC,
    ESCS_BROADCAST_CAP_RD_CHAR,
    ESCS_BROADCAST_CAP_RD_VALUE,
    ESCS_ACTIVE_SLOT_RW_CHAR,
    ESCS_ACTIVE_SLOT_RW_VALUE,
    ESCS_ADV_INTERVAL_RW_CHAR,
    ESCS_ADV_INTERVAL_RW_VALUE,
    ESCS_RADIO_TX_PWR_RW_CHAR,
    ESCS_RADIO_TX_PWR_RW_VALUE,
    ESCS_ADV_TX_PWR_RW_CHAR,
    ESCS_ADV_TX_PWR_RW_VALUE,
    ESCS_LOCK_STATE_RW_CHAR,
    ESCS_LOCK_STATE_RW_VALUE,
    ESCS_UNLOCK_RW_CHAR,
    ESCS_UNLOCK_RW_VALUE,
    ESCS_PUBLIC_ECDH_KEY_RD_CHAR,
    ESCS_PUBLIC_ECDH_KEY_RD_VALUE,
    ESCS_EID_ID_KEY_RD_CHAR,
    ESCS_EID_ID_KEY_RD_VALUE,
    ESCS_RW_ADV_SLOT_RW_CHAR,
    ESCS_RW_ADV_SLOT_RW_VALUE,
    ESCS_FACTORY_RESET_SET_CHAR,
    ESCS_FACTORY_RESET_SET_VALUE,
    ESCS_REMAIN_CONNECTABLE_RW_CHAR,
    ESCS_REMAIN_CONNECTABLE_RW_VALUE,
    ESCSS_IDX_NB,
};
/** @} */

/**
 * @defgroup ESCS_STRUCT Structures
 * @{
 */
/** @} */
/**@brief Structure for storing a slot key. */
typedef struct {
    uint8_t security_key[ESCS_LOCK_CODE_WRITE_LENGTH];                  /**< security key.*/
    uint8_t k_scaler;                                                   /**< K rotation scaler.*/
} slot_lock_code_t;

/**@brief Structure for storing EID slot key. */
typedef struct {
    uint8_t pub_ecdh_key[ESCS_ECDH_KEY_SIZE];   /**< public ecdh key.*/
    uint8_t k_scaler;                           /**< K rotation scaler.*/
} escs_eid_t;

/**@brief EddyStone Configuration Service environment variable. */
typedef struct {
    uint8_t          active_slot_no;                    /**< curret active slot number.*/
    uint8_t          lock_state;                        /**< beacon lock state.*/
    uint16_t         adv_interval;                      /**< advertising interval.*/
    int8_t           slot_tx_power[APP_MAX_ADV_SLOTS];  /**< radio tx power.*/
    int8_t           adv_tx_power;                      /**< advertised tx Power.*/
    bool             remain_connectable;                /**< whether to remain connectable.*/
    slot_lock_code_t beacon_lock_code;                  /**< beacon lock code.*/
#if(APP_IS_EID_SUPPORTED)
    escs_eid_t       eid_slot_data[APP_MAX_ADV_SLOTS];
#endif // APP_IS_EID_SUPPORTED
} ble_escs_init_params_t;

/**
 * @defgroup ESCE_FUNCTION Functions
 * @{
 */
/**
 *****************************************************************************************
 * @brief  get eddystone remain connectable state
 *
 * @retval true beacon connectable function is on.
 * @retval false beacon connectable function is off.
 *
 *****************************************************************************************
 */
bool es_adv_remain_connectable_get(void);

/**
 *****************************************************************************************
 * @brief set eddystone remain connectable state
 *
 * @param[in] remain_connectable: turn on or off beacon connectable function.
 *
 *****************************************************************************************
 */
void es_adv_remain_connectable_set(bool remain_connectable);

/**
 *****************************************************************************************
 * @brief get public ecdh key
 *
 * @param[out] p_ecdh_key_buf: pointer to key buffer.
 *
 *****************************************************************************************
 */
void es_public_ecdh_key_get (uint8_t* p_ecdh_key_buf);

/**
 *****************************************************************************************
 * @brief set public ecdh key
 *
 * @param[in] p_ecdh_key_buf: pointer of key buffer to set.
 *
 *****************************************************************************************
 */
void es_public_ecdh_key_set (uint8_t* p_ecdh_key_buf);

/**
 *****************************************************************************************
 * @brief set security key for slot
 *
 * @param[in] p_security_key: pointer to setting key data.
 * @param[in] is_eid_write:   true if the key for EID slot.
 *
 *****************************************************************************************
 */
void es_security_key_set(uint8_t* p_security_key, bool is_eid_write);

/**
 *****************************************************************************************
 * @brief to konw if a beacon has EID slot
 *
 * @return      true if has EID slot.
 *
 *****************************************************************************************
 */
bool es_beacon_has_eid_adv(void);

/**
 *****************************************************************************************
 * @brief set number of the actived slot
 *
 * @param[in]   slot_no: the number to be set to actived slot.
 *
 *****************************************************************************************
 */
void es_active_slot_number_set (uint8_t slot_no);

/**
 *****************************************************************************************
 * @brief get number of the actived slot
 *
 * @return      slot number of the actived slot.
 *
 *****************************************************************************************
 */
uint8_t es_active_slot_number_get (void);

/**
 *****************************************************************************************
 * @brief to know if the actived slot is a n EID slot
 *
 *****************************************************************************************
 */
bool is_active_slot_eid(void);

/**
 *****************************************************************************************
 * @brief lock a beacon
 *
 *****************************************************************************************
 */
void set_beacon_locked(void);

/**
 *****************************************************************************************
 * @brief unlock a beacon
 *
 *****************************************************************************************
 */
void set_beacon_unlocked(void);

/**
 *****************************************************************************************
 * @brief get the radio tx power
 *
 * @return      tx power(dBm).
 *
 *****************************************************************************************
 */
int8_t es_adv_tx_power_get(void);

/**
 *****************************************************************************************
 * @brief set the radio tx power
 *
 * @param[in]   adv_tx_power: tx power(dBm) to set.
 *
 *****************************************************************************************
 */
void es_adv_tx_power_set(int8_t adv_tx_power);

/**
 *****************************************************************************************
 * @brief get advertised tx power of a slot
 *
 * @return      advertised tx power(dBm).
 *
 *****************************************************************************************
 */
int8_t es_slot_tx_power_get(void);

/**
 *****************************************************************************************
 * @brief set advertised tx power of a slot
 *
 * @param[in]   tx_power: advertised tx power(dBm).
 *
 *****************************************************************************************
 */
void es_slot_tx_power_set(int8_t tx_power);

/**
 *****************************************************************************************
 * @brief get advertising interval for slot
 *
 * @return     advertising interval(mS).
 *
 *****************************************************************************************
 */
uint16_t es_adv_interval_get(void);

/**
 *****************************************************************************************
 * @brief set advertising interval for slot
 *
 * @param[in]   adv_interval: advertising interval(mS).
 *
 *****************************************************************************************
 */
void es_adv_interval_set (uint16_t adv_interval);

/**
 *****************************************************************************************
 * @brief Initialize Eddystone Configuration Service,and data for the service.
 *
 * @param[in] p_escs_init:  pointer to a initializing data structure.
 *
 *****************************************************************************************
 */
sdk_err_t esec_service_init(ble_escs_init_params_t *p_escs_init);
/** @} */

#endif
/** @} */
/** @} */
