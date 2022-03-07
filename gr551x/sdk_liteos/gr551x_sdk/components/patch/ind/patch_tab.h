#ifndef __PATCH_TAB_H_
#define __PATCH_TAB_H_

// WEAK Functions for platform init & code size optimization
__WEAK void  ble_con_env_init(void) {};
__WEAK void  ble_adv_env_init(void) {};
__WEAK void  ble_per_adv_env_init(void) {};
__WEAK void  ble_scan_env_init(void) {};
__WEAK void  ble_sync_env_init(void) {};

typedef int (*ke_msg_func_t)(uint16_t const msgid, uint16_t const *param,
                             uint16_t const dest_id, uint16_t const src_id);

typedef int (*llm_hci_cmd_hdl_func_t)(uint16_t const *param, uint16_t opcode);

typedef struct {
    ke_msg_func_t ori_func_addr;
    ke_msg_func_t new_func_addr;
} msg_tab_item_t;

typedef struct {
    llm_hci_cmd_hdl_func_t ori_func_addr;
    llm_hci_cmd_hdl_func_t new_func_addr;
} hci_cmd_tab_item_t;

// ble sdk task
extern int host_to_sdk_msg_handler_patch(uint16_t const msgid, uint16_t *param,
                                         uint16_t const dest_id, uint16_t const src_id);
// ble sdk gapm for common
extern int gap_activity_stopped_ind_handler_patch(uint16_t const msgid, uint16_t const *p_param,
                                                  uint16_t const dest_id, uint16_t const src_id);
extern int gapm_cmp_evt_handler_patch(uint16_t const msgid, uint16_t const *param,
                                      uint16_t const dest_id, uint16_t const src_id);
extern int gap_dev_bdaddr_ind_handler_patch(uint16_t const msgid, uint16_t const *param,
                                            uint16_t const dest_id, uint16_t const src_id);
// llm task for common
extern int llm_hci_command_handler_patch(uint16_t const msgid, uint16_t const *param,
                                         uint16_t const dest_id, uint16_t const src_id);
// hci cmd for common
extern int hci_le_add_dev_to_wlst_cmd_handler_patch(uint16_t const *param, uint16_t opcode);
extern int hci_le_rmv_dev_from_wlst_cmd_handler_patch(uint16_t const *param, uint16_t opcode);
extern int hci_le_clear_wlst_cmd_handler_patch(uint16_t const *param, uint16_t opcode);

extern int hci_dbg_ble_reg_wr_cmd_handler_patch(uint16_t const *param, uint16_t opcode);
extern int hci_dbg_ble_reg_rd_cmd_handler_patch(uint16_t const *param, uint16_t opcode);

#if CFG_MAX_CONNECTIONS
// gattc task
extern int l2cc_pdu_recv_ind_handler_patch(uint16_t const msgid, struct l2cc_pdu_recv_ind *param,
                                           uint16_t const dest_id, uint16_t const src_id);
// llc task
extern int llc_loc_llcp_rsp_to_handler_patch(uint16_t const msgid, uint16_t const *param,
                                             uint16_t const dest_id, uint16_t const src_id);
extern int llc_rem_llcp_rsp_to_handler_patch(uint16_t const msgid, uint16_t const *param,
                                             uint16_t const dest_id, uint16_t const src_id);
extern int lld_acl_rx_ind_handler_patch(uint16_t const msgid, uint16_t const *param,
                                        uint16_t const dest_id, uint16_t const src_id);
extern int lld_con_offset_upd_ind_handler_patch(uint16_t const msgid, uint16_t const *param,
                                                uint16_t const dest_id, uint16_t const src_id);
extern int lld_con_param_upd_cfm_handler_patch(uint16_t const msgid, uint16_t const *param,
                                               uint16_t const dest_id, uint16_t const src_id);
extern int lld_llcp_rx_ind_handler_patch(uint16_t const msgid, uint16_t const *param,
                                         uint16_t const dest_id, uint16_t const src_id);
extern int lld_llcp_tx_cfm_handler_patch(uint16_t const msgid, uint16_t const *param,
                                         uint16_t const dest_id, uint16_t const src_id);
extern int lld_ch_map_upd_cfm_handler_patch(uint16_t const msgid, uint16_t const *param,
                                            uint16_t const dest_id, uint16_t const src_id);
extern int lld_phy_upd_cfm_handler_patch(uint16_t const msgid, uint16_t const *param,
                                         uint16_t const dest_id, uint16_t const src_id);
extern int llc_auth_payl_nearly_to_handler_patch(uint16_t const msgid, uint16_t const *param,
                                                 uint16_t const dest_id, uint16_t const src_id);
extern int llc_auth_payl_real_to_handler_patch(uint16_t const msgid, uint16_t const *param,
                                               uint16_t const dest_id, uint16_t const src_id);
// l2cc task
extern int hci_nb_cmp_pkts_evt_handler_patch(uint16_t const msgid, struct hci_nb_cmp_pkts_evt const *event,
                                             uint16_t const dest_id, uint16_t const src_id);
// gapc task
extern int gapc_hci_handler_patch(uint16_t const msgid, uint16_t const *event,
                                  uint16_t dest_id, uint16_t src_id);
// ble sdk task for gapc
extern int gap_connection_req_ind_handler_patch(uint16_t const msgid, struct gapc_connection_req_ind const *param,
                                                uint16_t const dest_id, uint16_t const src_id);
extern int gap_disconnect_ind_handler_patch(uint16_t const msgid, struct gapc_disconnect_ind const *param,
                                            uint16_t const dest_id, uint16_t const src_id);
extern int gap_cmp_evt_handler_patch(uint16_t const msgid, struct gapc_cmp_evt const *param,
                                     uint16_t const dest_id, uint16_t const src_id);
extern int sec_rcv_sec_req_ind_handler_patch(uint16_t const msgid, struct gapc_security_ind const *param,
                                             uint16_t const dest_id, uint16_t const src_id);
extern int sec_rcv_encrypt_req_ind_handler_patch(uint16_t const msgid, struct gapc_encrypt_req_ind const *param,
                                                 uint16_t const dest_id, uint16_t const src_id);
// ble sdk task for gatt
extern int ble_sdk_gatts_svc_changed_cfg_ind_handler_patch(uint16_t const msgid,
                                                           struct gattc_svc_changed_cfg const *param,
                                                           uint16_t const dest_id, uint16_t const src_id);
extern int ble_sdk_gattc_event_ind_handler_patch(uint16_t const msgid, struct gattc_event_ind const *param,
                                                 uint16_t const dest_id, uint16_t const src_id);
#endif

#if CFG_MAX_SCAN
// gapm task for scan
extern int gapm_activity_create_cmd_handler_patch(uint16_t const msgid, struct gapm_activity_create_cmd const *param,
                                                  uint16_t const dest_id, uint16_t const src_id);
extern int gapm_hci_handler_patch(uint16_t const msgid, uint16_t const *event,
                                  uint16_t dest_id, uint16_t opcode);

// hci cmd for scan
extern int hci_le_ext_create_con_cmd_handler_patch(struct hci_le_ext_create_con_cmd *param, uint16_t opcode);
extern int hci_le_set_ext_scan_param_cmd_handler_patch(struct hci_le_set_ext_scan_param_cmd const *param,
                                                       uint16_t opcode);
extern int hci_le_set_ext_scan_en_cmd_handler_patch(struct hci_le_set_ext_scan_en_cmd const *param, uint16_t opcode);
#endif

msg_tab_item_t msg_tab[] = {
    // ble sdk gapm for common
    {(ke_msg_func_t)0x00074235, (ke_msg_func_t)gapm_cmp_evt_handler_patch},
    {(ke_msg_func_t)0x000709e1, (ke_msg_func_t)gap_activity_stopped_ind_handler_patch},
    {(ke_msg_func_t)0x00071f6d, (ke_msg_func_t)gap_dev_bdaddr_ind_handler_patch},
    // llm task for common
    {(ke_msg_func_t)0x00051d09, (ke_msg_func_t)llm_hci_command_handler_patch},
    // ble sdk task for common
    {(ke_msg_func_t)0x000761a9, (ke_msg_func_t)host_to_sdk_msg_handler_patch},

#if CFG_MAX_CONNECTIONS
    // gattc task for conn
    {(ke_msg_func_t)0x00032e7d, (ke_msg_func_t)l2cc_pdu_recv_ind_handler_patch},
    // llc task for conn
    {(ke_msg_func_t)0x000391b5, (ke_msg_func_t)llc_loc_llcp_rsp_to_handler_patch},
    {(ke_msg_func_t)0x0003a795, (ke_msg_func_t)llc_rem_llcp_rsp_to_handler_patch},
    {(ke_msg_func_t)0x00049a6d, (ke_msg_func_t)lld_llcp_rx_ind_handler_patch},
    {(ke_msg_func_t)0x00049c95, (ke_msg_func_t)lld_llcp_tx_cfm_handler_patch},
    {(ke_msg_func_t)0x0003aed9, (ke_msg_func_t)lld_acl_rx_ind_handler_patch},
    {(ke_msg_func_t)0x00045e6d, (ke_msg_func_t)lld_con_param_upd_cfm_handler_patch},
    {(ke_msg_func_t)0x000418c9, (ke_msg_func_t)lld_ch_map_upd_cfm_handler_patch},
    {(ke_msg_func_t)0x0004b67d, (ke_msg_func_t)lld_phy_upd_cfm_handler_patch},
    {(ke_msg_func_t)0x00045da1, (ke_msg_func_t)lld_con_offset_upd_ind_handler_patch},
    {(ke_msg_func_t)0x0003679d, (ke_msg_func_t)llc_auth_payl_nearly_to_handler_patch},
    {(ke_msg_func_t)0x00036801, (ke_msg_func_t)llc_auth_payl_real_to_handler_patch},
    // l2cc task for conn
    {(ke_msg_func_t)0x0002b281, (ke_msg_func_t)hci_nb_cmp_pkts_evt_handler_patch},
    // gapc task for conn
    {(ke_msg_func_t)0x0000f751, (ke_msg_func_t)gapc_hci_handler_patch},
    // ble sdk gapc for conn
    {(ke_msg_func_t)0x00071781, (ke_msg_func_t)gap_connection_req_ind_handler_patch},
    {(ke_msg_func_t)0x00072229, (ke_msg_func_t)gap_disconnect_ind_handler_patch},
    {(ke_msg_func_t)0x0007ac89, (ke_msg_func_t)sec_rcv_sec_req_ind_handler_patch},
    {(ke_msg_func_t)0x00071519, (ke_msg_func_t)gap_cmp_evt_handler_patch},
    {(ke_msg_func_t)0x0007ab15, (ke_msg_func_t)sec_rcv_encrypt_req_ind_handler_patch},
    // ble sdk gatt for conn
    {(ke_msg_func_t)0x0006d0d5, (ke_msg_func_t)ble_sdk_gattc_event_ind_handler_patch},
    {(ke_msg_func_t)0x0006db75, (ke_msg_func_t)ble_sdk_gatts_svc_changed_cfg_ind_handler_patch},
#endif

#if CFG_MAX_SCAN
    // gapm task for scan
    {(ke_msg_func_t)0x00010d2d, (ke_msg_func_t)gapm_activity_create_cmd_handler_patch},
    {(ke_msg_func_t)0x00012751, (ke_msg_func_t)gapm_hci_handler_patch},
#endif
};

extern void reg_hci_cmd_patch_tab(hci_cmd_tab_item_t *hci_cmd_tab, uint16_t hci_cmd_cnt);
extern void reg_msg_patch_tab(msg_tab_item_t *msg_tab, uint16_t msg_cnt);

#endif
