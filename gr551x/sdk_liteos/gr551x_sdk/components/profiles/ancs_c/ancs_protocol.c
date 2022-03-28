/**
 *******************************************************************************
 *
 * @file ancs_protocol.c
 *
 * @brief ANCS protocal implementation.
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

#include <stdio.h>
#include <string.h>
#include "app_log.h"
#include "ancs_c.h"
#include "ancs_protocol.h"

#define LEN_2 2
#define LEN_4 4
#define LEN_6 6
#define LEN_8 8
#define INDEX_5 5
#define INDEX_6 6
#define INDEX_7 7
#define INDEX_8 8
#define OFFSET_8 8

/*
 * LOCAL VARIABLE DEFINITIONS
 *****************************************************************************************
 */
static uint32_t s_uid;

/**@brief String literals for the iOS notification Category id types. Used then printing to UART. */
static char const * lit_catid[] = {
    "Other",
    "Incoming Call",
    "Missed Call",
    "Voice Mail",
    "Social",
    "Schedule",
    "Email",
    "News",
    "Health And Fitness",
    "Business And Finance",
    "Location",
    "Entertainment"
};

/**@brief String literals for the iOS notification event types. Used then printing to UART. */
static char const * lit_eventid[] = {
    "Added",
    "Modified",
    "Removed"
};

/**@brief String literals for the iOS notification attribute types. Used when printing to UART. */
static char const * lit_attrid[] = {
    "App Identifier",
    "Title",
    "Subtitle",
    "Message",
    "Message Size",
    "Date",
    "Positive Action Label",
    "Negative Action Label"
};
static uint8_t s_attr_buf[CFG_ANCS_ATTRIBUTE_MAXLEN];
static uint16_t s_attr_size;
static uint16_t s_buf_index;


/**
 *****************************************************************************************
 * @brief Print data source message.
 *
 *****************************************************************************************
 */
static void ancs_notify_attr_print(void)
{
    uint8_t commd_id = s_attr_buf[0];
    uint16_t UID;
    uint8_t attr_id;
    uint16_t attr_size = 0;
    uint8_t ret;
    ret = memcpy_s(&UID, LEN_4, &s_attr_buf[1], LEN_4);
    if (ret < 0) {
        return;
    }
    ret = memcpy_s(&attr_size, LEN_2, &s_attr_buf[INDEX_6], LEN_2);
    if (ret < 0) {
        return;
    }

    if (commd_id == CTRL_POINT_GET_NTF_ATTRIBUTE) {
        attr_id = s_attr_buf[INDEX_5];
        APP_LOG_INFO("UID=%d, ATTR_ID: %s, ATTR_SIZE=%d", UID, lit_attrid[attr_id], attr_size);
        for (uint16_t idx = 0; idx < attr_size; idx++) {
            printf("%c", s_attr_buf[INDEX_8 + idx]);
        }
        printf("\r\n");
    } else if (commd_id == CTRL_POINT_GET_APP_ATTRIBUTE) {
        for (uint16_t idx = 0; idx < attr_size; idx++) {
            printf("%c", s_attr_buf[idx]);
        }
        printf("\r\n");
    }
}
/**
 *****************************************************************************************
 * @brief Decode data source message.
 *
 * @param[in] p_data: Pointer to the parameters of the read request.
 * @param[in] length: Length of read data
 *
 *****************************************************************************************
 */
void ancs_decode_data_source(uint8_t *p_data, uint16_t length)
{
    uint8_t ret;
    // It's the begginning of a attr info.
    if (s_buf_index == 0) {
        ret = memcpy_s(&s_attr_size, LEN_2, &p_data[INDEX_6], LEN_2);
        if (ret < 0) {
            return;
        }
        ret = memcpy_s(&s_attr_size, length, p_data, length);
        if (ret < 0) {
            return;
        }
        // It's the end of the attr info, a complete attr info is already stored in the buffer.
        if (s_attr_size == length - LEN_8) {
            ancs_notify_attr_print();
            s_buf_index = 0;
        } else {         // It's not the end of the attr info.
            s_buf_index = length;
        }
    } else {     // It isn't the begginning of a attr info.
        ret = memcpy_s(&s_attr_buf[s_buf_index], length, p_data, length);
        if (ret < 0) {
            return;
        }
        // It's the end of the attr info, print the buffer.
        if (s_attr_size == (s_buf_index - INDEX_8) + length) {
            ancs_notify_attr_print();
            s_buf_index = 0;
        } else {         // It's not the end of the attr info.
            s_buf_index = s_buf_index + length;
        }
    }
}

/**
 *****************************************************************************************
 * @brief Get notification attribute
 *
 * @param[in] uid: The uid of notify message
 * @param[in] noti_attr: The notification attribute
 *
 *****************************************************************************************
 */
void ancs_notify_attr_get(int uid, char noti_attr)
{
    int len = 0;
    uint8_t buf[8];
    uint8_t ret;
    buf[0] = CTRL_POINT_GET_NTF_ATTRIBUTE;
    ret = memcpy_s(&buf[1], LEN_4, &uid, LEN_4);
    if (ret < 0) {
        return;
    }
    buf[INDEX_5] = noti_attr;
    if (ANCS_NOTIF_ATTR_ID_TITLE == noti_attr || ANCS_NOTIF_ATTR_ID_SUBTITLE== noti_attr
            || ANCS_NOTIF_ATTR_ID_MESSAGE== noti_attr) {
        len = CFG_ANCS_ATTRIBUTE_MAXLEN;
        buf[INDEX_6] = (len & 0xff);
        buf[INDEX_7] = (len >> OFFSET_8) & 0xff;
        ancs_c_write_control_point(0, buf, LEN_8);
    } else {
        ancs_c_write_control_point(0, buf, LEN_6);
    }
}

/**
 *****************************************************************************************
 * @brief ancs perform action
 *
 * @param[in] uid: The uid of notify message
 * @param[in] action: The action status defined by specification
 *****************************************************************************************
 */
void ancs_action_perform(int uid, int action)
{
    uint8_t buf[6];
    uint8_t ret;
    buf[0] = CTRL_POINT_PERFORM_NTF_ACTION;
    ret = memcpy_s(&buf[1], LEN_4, &uid, LEN_4);
    if (ret < 0) {
        return;
    }
    buf[INDEX_5] = action;
    ancs_c_write_control_point(0, buf, LEN_6);
}

/**
 *****************************************************************************************
 * @brief print notification content with format
 *
 * @param[in] p_notif: Pointer to the parameters of the notification source data buffer
 *****************************************************************************************
 */
static void notification_content_print(ntf_source_pdu_t *p_notif)
{
    APP_LOG_INFO("\r\nNotification");
    APP_LOG_INFO("Event:       %s", lit_eventid[p_notif->event_id]);
    APP_LOG_INFO("Category ID: %s", lit_catid[p_notif->category_id]);
    APP_LOG_INFO("Category Cnt:%u", (unsigned int) p_notif->category_count);
    APP_LOG_INFO("UID:         %u", (unsigned int) p_notif->notification_uid);
    APP_LOG_INFO("Flags: ");
    if (p_notif->event_flags.silent == 1) {
        APP_LOG_INFO(" Silent");
    }
    if (p_notif->event_flags.important == 1) {
        APP_LOG_INFO(" Important");
    }
    if (p_notif->event_flags.pre_existing == 1) {
        APP_LOG_INFO(" Pre-existing");
    }
    if (p_notif->event_flags.positive_action == 1) {
        APP_LOG_INFO(" Positive Action");
    }
    if (p_notif->event_flags.negative_action == 1) {
        APP_LOG_INFO(" Negative Action");
    }
}

/**
 *****************************************************************************************
 * @brief set phone call notify message uid
 *
 * @param[in] uid: the uid of notify message
 *****************************************************************************************
 */
static void ancs_set_uid(int uid)
{
    s_uid = uid;
}

/**
 *****************************************************************************************
 * @brief get ancs phone call uid
 *
 * @return phone call notify message uid
 *****************************************************************************************
 */
int ancs_get_uid(void)
{
    return s_uid;
}

/**
 *****************************************************************************************
 * @brief Decode notification source message.
 *
 * @param[in] p_data: Pointer to the parameters of the read request.
 * @param[in] length: The Length of read value
 *****************************************************************************************
 */
void ancs_decode_notification_source(uint8_t *p_data, uint16_t length)
{
    ntf_source_pdu_t *pdu = (ntf_source_pdu_t*)p_data;
    notification_content_print(pdu);

    ancs_set_uid((unsigned int) pdu->notification_uid);
}
