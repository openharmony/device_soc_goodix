/**
 *****************************************************************************************
 *
 * @file thscps.h
 *
 * @brief Throughput Control Point Service API
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
 * @brief Definitions and prototypes for the BLE Service Interface.
 */

/**
 * @defgroup BLE_SDK_THSCPS Throughput Control Point Client (THSCPS)
 * @{
 * @brief THS Control Point Interface module.
 *
 * @details The Throughput Control Point Service contains the APIs and types, which can be used by the
 *          application to perform throughput parameters set.
 *
 *          After \ref thscps_evt_handler_t variable is initialized, the application must
 *          call \ref thscps_service_init() to add the Throughput Control Point Service and
 *          THS Control Point, THS Control Point Response characteristics to the BLE Stack database.
 */

#ifndef __THSCPS_H__
#define __THSCPS_H__

#include "ble_prf_types.h"
#include "gr55xx_sys.h"
#include "custom_config.h"

/**
 * @defgroup THSCPS_MACRO Defines
 * @{
 */
#define THSCPS_CONNECTION_MAX       (10 < CFG_MAX_CONNECTIONS ? \
                                     10 : CFG_MAX_CONNECTIONS) /**< Maximum number of THSCPS connections. */
#define THSCPS_CTRL_PT_VAL_LEN       20  /**< Length of the value of Control Point characteristic. */
#define THSCPS_TEST_SETTING_VAL_LEN  20  /**< Length of the value of Test Setting characteristic. */
#define THSCPS_TEST_INFO_VAL_LEN     20  /**< Length of the value of Test Information characteristic. */
#define THSCPS_CONN_INFO_VAL_LEN     20  /**< Length of the value of Connection Information characteristic. */
#define THSCPS_SERVICE_UUID         0x1B, 0xD7, 0x90, 0xEC, 0xE8, 0xB9, 0x75, 0x80, 0x0A, 0x46, 0x44, 0xD3, \
                                    0x01, 0x07, 0xED, 0xA6 /**< The UUID of THS Control Point Control Point Service \
                                                                for setting advertising data. */
#define THSCPS_ACTION_ON            0x01                                               /**< Start the action. */
#define THSCPS_ACTION_OFF           0x00                                               /**< Stop the action. */
#define THSCPS_CTRL_PT_RSP_CODE     0xff                                               /**< Response code. */
/** @} */

/**
 * @defgroup THSCPS_ENUM Enumerations
 * @{
 */
/**@brief Local device GAP Role Type. */
typedef enum {
    THSCPS_TEST_ROLE_INVALID,   /**< Test role: Invalid. */
    THSCPS_TEST_ROLE_SLAVE,     /**< Test role: Slave. */
    THSCPS_TEST_ROLE_MASTER,    /**< Test role: Master. */
} thscps_test_role_t;

/**
 * @brief Specify PHY. */
typedef enum {
    THSCPS_LEGACY_ADV_PHY, /**< Undefined LE PHY. */
    THSCPS_1MBPS_PHY,      /**< LE 1M PHY. */
    THSCPS_CODED_PHY,      /**< LE Coded PHY. */
} thscps_adv_phy_t;

/**@brief Throughput Test State. */
typedef enum {
    THSCPS_TEST_STOP,          /**< Throughput Test is not ongoing. */
    THSCPS_TEST_STARTED,       /**< Throughput Test is ongoing. */
} thscps_test_state_t;

/**@brief THS Control Point Service Control Point IDs. */
typedef enum {
    THSCPS_CTRL_PT_INVALID,                /**< Invalid cmd id. */
    THSCPS_CTRL_PT_TEST_ROLE,              /**< Test role set. */
    THSCPS_CTRL_PT_ADV_PARAM,              /**< Advertising parameters set. */
    THSCPS_CTRL_PT_ADV_ACTION,             /**< Advertiding action. */
    THSCPS_CTRL_PT_SCAN_ACTION,            /**< Scan action. */
    THSCPS_CTRL_PT_TEST_READY,             /**< Ready for test. */
    THSCPS_CTRL_PT_SETTING,
} thscps_ctrl_pt_id_t;

/**@brief Throughput service settings types. */
typedef enum {
    THSCPS_SETTINGS_TYPE_CI,           /**< BLE Connection Interval parameter. */
    THSCPS_SETTINGS_TYPE_MTU,          /**< MTU Size. */
    THSCPS_SETTINGS_TYPE_PDU,          /**< PDU Size. */
    THSCPS_SETTINGS_TYPE_PHY,          /**< Radio Phy mode, 1M, 2M, Encoded. */
    THSCPS_SETTINGS_TYPE_TRANS_MODE,   /**< Data transmission mode. */
    THSCPS_SETTINGS_TYPE_TX_POWER,     /**< Connect Tx power. */
    THSCPS_SETTINGS_TYPE_TOGGLE,       /**< Throughput toggle state of sending the data. */
} thscps_settings_type_t;

/**@brief THS Control Point Response Types. */
typedef enum {
    THSCPS_RSP_ID_SUCCESS,          /**< Success. */
    THSCPS_RSP_ID_UNSUPPORT,        /**< Failed because of unsupport command. */
    THSCPS_RSP_ID_STATUS_ERR,       /**< Failed because of disallowed status. */
    THSCPS_RSP_ID_PARAM_ERR,        /**< Failed because of parameter error. */
    THSCPS_RSP_ID_TEST_ROLE_ERR,    /**< Failed becaude of Test role error. */
    THSCPS_RSP_ID_SDK_ERR,          /**< Failed becaude of SDK error Size. */
} thscps_status_rsp_t;

/**@brief THS Control Point Service Event Type. */
typedef enum {
    THSCPS_EVT_INVALID,                     /**< Throughput Control Point Service invalid event. */
    THSCPS_EVT_CTRL_PT_IND_ENABLE,          /**< THS Control Point indicaiton is enabled. */
    THSCPS_EVT_CTRL_PT_IND_DISABLE,         /**< THS Control Point indicaiton is disabled. */
    THSCPS_EVT_TSET_SET_NTF_ENABLE,         /**< THS Test Information notification is enabled. */
    THSCPS_EVT_TSET_SET_NTF_DISABLE,        /**< THS Test Information notification is disabled. */
    THSCPS_EVT_TSET_INFO_NTF_ENABLE,        /**< THS Test Information notification is enabled. */
    THSCPS_EVT_TSET_INFO_NTF_DISABLE,       /**< THS Test Information notification is disabled. */
    THSCPS_EVT_CONN_INFO_NTF_ENABLE,        /**< THS Connection Information notification is enabled. */
    THSCPS_EVT_CONN_INFO_NTF_DISABLE,       /**< THS Connection Information notification is disabled. */
    THSCPS_EVT_TEST_ROLE_SET,               /**< Set Test role. */
    THSCPS_EVT_ADV_PRAM_SET,                /**< Set advertising parameters. */
    THSCPS_EVT_ADV_ACTION,                  /**< Set advertising action. */
    THSCPS_EVT_SCAN_ACTION,                 /**< Set scan action. */
    THSCPS_EVT_SETTING_SET,                 /**< Throughput setting. */
} thscps_evt_type_t;
/** @} */

/**
 * @defgroup THSCPS_STRUCT Structures
 * @{
 */
/**@brief THS Setting Information value. */
typedef struct {
    uint16_t        length;  /**< THS Setting Information length. */
    const uint8_t  *p_data;  /**< THS Setting Information data. */
} thscps_setting_t;

/**@brief THS Test Information value. */
typedef struct {
    int8_t     rssi;         /**< RX rssi. */
    uint8_t    right_rate;   /**< RX right rate. */
    uint16_t   instant_val;  /**< Instant throughput value. */
    uint16_t   average_val;  /**< Average throughput value. */
    uint16_t   packets_val;  /**< Recieved Packects value. */
} thscps_test_info_t;

/**@brief THS connect Information value. */
typedef struct {
    uint16_t    ci;          /**< Connect Interval. */
    uint16_t    pdu;         /**< PDU. */
    uint16_t    mtu;         /**< MTU. */
    uint8_t     tx_phy;      /**< TX PHY. */
    uint8_t     rx_phy;      /**< RX PHY. */
    int8_t      tx_power;    /**< TX Power. */
    uint8_t     ths_mode;    /**< THS Mode. */
} thscps_test_conn_info_t;

/**@brief THS Control Point Advertising Parameters. */
typedef struct {
    thscps_adv_phy_t phy;           /**< Specify what PHY the Controller has changed for TX/RX. */
    uint16_t           interval;      /**< Advertising interval. */
    uint16_t           duration;      /**< Advertising duration. */
    int8_t             tx_power;      /**< Advertising tx power. */
} thscps_adv_param_t;

/**@brief THS Control Point Response value. */
typedef struct {
    uint8_t              cmd_id;        /**< Commander ID. */
    thscps_status_rsp_t  status;        /**< Status. */
    uint8_t              conn_idx;      /**< Connection of cmd set. */
} thscps_rsp_val_t;

/**@brief THS Control Point Service event. */
typedef struct {
    uint8_t                 conn_idx;            /**< The connection index. */
    thscps_evt_type_t       evt_type;            /**< THS client event type. */
    union {
        uint8_t             action_set;          /**< Set action. */
        thscps_test_role_t  test_role;           /**< GAP role type. */
        thscps_adv_param_t  adv_param;           /**< Advertising parameters. */
        thscps_setting_t    setting_info;        /**< Setting infomation. */
    } param;                                     /**< Event parameters. */
} thscps_evt_t;
/** @} */

/**
 * @defgroup THSCPS_TYPEDEF Typedefs
 * @{
 */
/**@brief  THS Control Point Service event handler type. */
typedef void (*thscps_evt_handler_t)(thscps_evt_t *p_evt);
/** @} */

/**
 * @defgroup THSCPS_FUNCTION Functions
 * @{
 */
/**
 *****************************************************************************************
 * @brief Initialize a THS Control Point Service instance and add in the DB.
 *
 * @param[in] evt_handler: THS Control Point Service event handler.
 *
 * @return Result of service initialization.
 *****************************************************************************************
 */
sdk_err_t thscps_service_init(thscps_evt_handler_t evt_handler);

/**
 *****************************************************************************************
 * @brief Send Control Point Response if its indicaiton has been enabled.
 *
 * @param[in] conn_idx:  Connnection index.
 * @param[in] p_rsp_val: Pointer to Response value.
 *
 * @return Result of indicate value.
 *****************************************************************************************
 */
sdk_err_t thscps_ctrl_pt_rsp_send(uint8_t conn_idx, thscps_rsp_val_t *p_rsp_val);

/**
 *****************************************************************************************
 * @brief Send Settings Response if its notification has been enabled.
 *
 * @param[in] conn_idx:  Connnection index.
 * @param[in] p_rsp_val: Pointer to Response value.
 *
 * @return Result of indicate value.
 *****************************************************************************************
 */
sdk_err_t thscps_test_setting_rsp_send(uint8_t conn_idx, thscps_rsp_val_t *p_rsp_val);

/**
 *****************************************************************************************
 * @brief Send Throughput Test Information if its notification has been enabled.
 *
 * @param[in] conn_idx:    Connnection index.
 * @param[in] p_test_info: Pointer to test information value.
 *
 * @return Result of notify value.
 *****************************************************************************************
 */
sdk_err_t thscps_test_info_send(uint8_t conn_idx, thscps_test_info_t *p_test_info);

/**
 *****************************************************************************************
 * @brief Send Throughput Test Connect Information if its notification has been enabled.
 *
 * @param[in] conn_idx:    Connnection index.
 * @param[in] p_conn_info: Pointer to connection information value.
 *
 * @return Result of notify value.
 *****************************************************************************************
 */
sdk_err_t thscps_conn_info_send(uint8_t conn_idx, thscps_test_conn_info_t *p_conn_info);

/**
 *****************************************************************************************
 * @brief Set throughput state set.
 *
 * @param[in] test_state:  Throughput Test state.
 *****************************************************************************************
 */
void thscps_test_state_set(thscps_test_state_t test_state);
/** @} */
#endif
/** @} */
/** @} */
