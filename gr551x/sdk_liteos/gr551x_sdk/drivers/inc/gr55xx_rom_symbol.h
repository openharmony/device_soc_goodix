/**
 ****************************************************************************************
 *
 * @file gr55xx_rom_symbol.h
 *
 * @brief PERIPHERAL API ROM SYMBOL DRIVER
 *
 ****************************************************************************************
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
 ****************************************************************************************
 */

#ifndef __GR55xx_ROM_SYMBOL_H__
#define __GR55xx_ROM_SYMBOL_H__

#include "gr55xx_hal.h"
#include "gr55xx_nvds.h"

#ifdef __cplusplus
extern "C" {
#endif

extern exflash_handle_t g_exflash_handle;
extern uint32_t *SVC_Table;
extern uint8_t *g_nvds_buf;

uint16_t gdx_lcp_buf_init(uint32_t buf_addr);
uint8_t nvds_put_patch(NvdsTag_t tag, uint16_t len, const uint8_t *p_buf);
uint8_t nvds_put_rom(NvdsTag_t tag, uint16_t len, const uint8_t *p_buf);
void dfu_cmd_handler_replace_for_encrypt(void);
uint32_t get_patch_rep_addr(uint32_t ori_func);
void __main(void);
void system_platform_init(void);
int main(void);
void $Super$$main(void);
void __iar_program_start(void);
void __iar_data_init3(void);

void hal_pwr_config_timer_wakeup_ext(uint8_t timer_mode, uint32_t load_count);
void hal_pwr_register_timer_elaspsed_handler(pwr_slp_elapsed_handler_t pwr_slp_elapsed_hander);
hal_status_t hal_calendar_init_ext(calendar_handle_t *p_calendar);
hal_status_t hal_calendar_deinit_ext(calendar_handle_t *p_calendar);
void hal_calendar_register_callback(hal_calendar_callback_t *hal_calendar_callback);
void hal_gpio_init_ext(gpio_regs_t *GPIOx, gpio_init_t *p_gpio_init);
void hal_gpio_deinit_ext(gpio_regs_t *GPIOx, uint32_t gpio_pin);
void hal_gpio_register_callback(hal_gpio_callback_t *callback);
hal_status_t hal_pkc_init_ext(pkc_handle_t *p_pkc);
hal_status_t hal_pkc_deinit_ext(pkc_handle_t *p_pkc);
void hal_pkc_register_callback(hal_pkc_callback_t *hal_pkc_callback);
hal_status_t hal_uart_init_ext(uart_handle_t *p_uart);
hal_status_t hal_uart_deinit_ext (uart_handle_t *p_uart);
void hal_uart_register_callback(hal_uart_callback_t *hal_uart_callback);
hal_status_t hal_hmac_init_ext(hmac_handle_t *p_hmac);
hal_status_t hal_hmac_deinit_ext(hmac_handle_t *p_hmac);
void hal_hmac_register_callback(hal_hmac_callback_t *hal_hmac_callback);
hal_status_t hal_xqspi_init_ext(xqspi_handle_t *p_xqspi);
hal_status_t hal_xqspi_deinit_ext(xqspi_handle_t *p_xqspi);
void hal_xqspi_register_callback(hal_xqspi_callback_t *hal_xqspi_callback);
hal_status_t hal_efuse_init_ext(efuse_handle_t *p_efuse);
hal_status_t hal_efuse_deinit_ext(efuse_handle_t *p_efuse);
void hal_efuse_register_callback(hal_efuse_callback_t *hal_efuse_callback);
hal_status_t hal_rng_init_ext(rng_handle_t *p_rng);
hal_status_t hal_rng_deinit_ext(rng_handle_t *p_rng);
void hal_rng_register_callback(hal_rng_callback_t *hal_rng_callback);
hal_status_t hal_init_ext(void);
hal_status_t hal_deinit_ext(void);
void hal_register_callback(hal_callback_t *hal_callback);
hal_status_t hal_aon_wdt_init_ext(aon_wdt_handle_t *p_aon_wdt);
hal_status_t hal_aon_wdt_deinit_ext(aon_wdt_handle_t *p_aon_wdt);
void hal_aon_wdt_register_callback(hal_aon_wdt_callback_t *aon_wdt_callback);
hal_status_t hal_qspi_init_ext(qspi_handle_t *p_qspi);
hal_status_t hal_qspi_deinit_ext(qspi_handle_t *p_qspi);
void hal_qspi_register_callback(hal_qspi_callback_t *hal_qspi_callback);
hal_status_t hal_timer_base_init_ext(timer_handle_t *p_timer);
hal_status_t hal_timer_base_deinit_ext(timer_handle_t *p_timer);
void hal_timer_register_callback(hal_timer_callback_t *hal_timer_callback);
hal_status_t hal_comp_init_ext(comp_handle_t *p_comp);
hal_status_t hal_comp_deinit_ext(comp_handle_t *p_comp);
void hal_comp_register_callback(comp_callback_t *comp_callback);
hal_status_t hal_exflash_init_ext(exflash_handle_t *p_exflash);
hal_status_t hal_exflash_deinit_ext(exflash_handle_t *p_exflash);
void hal_exflash_register_callback(hal_exflash_callback_t *hal_exflash_callback);
hal_status_t hal_exflash_read_rom(exflash_handle_t *p_exflash, uint32_t addr, uint8_t *p_data, uint32_t size);
hal_status_t hal_exflash_read_patch(exflash_handle_t *p_exflash, uint32_t addr, uint8_t *p_data, uint32_t size);
hal_status_t hal_xqspi_init_ext_patch(xqspi_handle_t *p_xqspi);
hal_status_t hal_xqspi_init_ext_rom(xqspi_handle_t *p_xqspi);
hal_status_t hal_pwm_init_ext(pwm_handle_t *p_pwm);
hal_status_t hal_pwm_deinit_ext(pwm_handle_t *p_pwm);
void hal_pwm_register_callback(hal_pwm_callback_t *hal_pwm_callback);
hal_status_t hal_i2c_init_ext(i2c_handle_t *p_i2c);
hal_status_t hal_i2c_deinit_ext(i2c_handle_t *p_i2c);
void hal_i2c_register_callback(hal_i2c_callback_t *hal_i2c_callback);
hal_status_t hal_dual_timer_init_ext(dual_timer_handle_t *p_dual_timer);
hal_status_t hal_dual_timer_deinit_ext(dual_timer_handle_t *p_dual_timer);
void hal_dual_timer_register_callback(hal_dual_timer_callback_t *hal_dual_timer_callback);
hal_status_t hal_wdt_init_ext(wdt_handle_t *p_wdt);
hal_status_t hal_wdt_deinit_ext(wdt_handle_t *p_wdt);
void hal_wdt_register_callback(hal_wdt_callback_t *hal_wdt_callback);
hal_status_t hal_i2s_init_ext(i2s_handle_t *p_i2s);
hal_status_t hal_i2s_deinit_ext(i2s_handle_t *p_i2s);
void hal_i2s_register_callback(hal_i2s_callback_t *hal_i2s_callback);
hal_status_t hal_aes_init_ext(aes_handle_t *p_aes);
hal_status_t hal_aes_deinit_ext(aes_handle_t *p_aes);
void hal_aes_register_callback(aes_callback_t *aes_callback);
void hal_aon_gpio_init_ext(aon_gpio_init_t *p_aon_gpio_init);
void hal_aon_gpio_deinit_ext(uint32_t aon_gpio_pin);
void hal_aon_gpio_register_callback(aon_gpio_callback_t *aon_gpio_callback);
hal_status_t hal_adc_init_ext(adc_handle_t *p_adc);
hal_status_t hal_adc_deinit_ext(adc_handle_t *p_adc);
void hal_adc_register_callback(adc_callback_t *adc_callback);
hal_status_t hal_spi_init_ext(spi_handle_t *p_spi);
hal_status_t hal_spi_deinit_ext(spi_handle_t *p_spi);
void hal_spi_register_callback(hal_spi_callback_t *hal_spi_callback);

#ifdef __cplusplus
}
#endif

#endif /* __GR55xx_ROM_SYMBOL_H__ */
