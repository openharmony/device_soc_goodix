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
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

void SystemPeripheralsInit(void);
void HardwareRandomInit(void);
void OSVectorInit(void);
void FileSystemInit(void);
void OHOS_SystemInit(void); // this is the service loader function for OHOS
void WDT_IRQHandler(void);
void BLE_SDK_Handler(void);
void BLE_IRQHandler(void);
void DMA_IRQHandler(void);
void SPI_M_IRQHandler(void);
void SPI_S_IRQHandler(void);
void EXT0_IRQHandler(void);
void EXT1_IRQHandler(void);
void TIMER0_IRQHandler(void);
void TIMER1_IRQHandler(void);
void QSPI0_IRQHandler(void);
void UART0_IRQHandler(void);
void UART1_IRQHandler(void);
void I2C0_IRQHandler(void);
void I2C1_IRQHandler(void);
void AES_IRQHandler(void);
void HMAC_IRQHandler(void);
void EXT2_IRQHandler(void);
void RNG_IRQHandler(void);
void PMU_IRQHandler(void);
void PKC_IRQHandler(void);
void XQSPI_IRQHandler(void);
void QSPI1_IRQHandler(void);
void PWR_CMD_IRQHandler(void);
void BLESLP_IRQHandler(void);
void SLPTIMER_IRQHandler(void);
void COMP_IRQHandler(void);
void AON_WDT_IRQHandler(void);
void I2S_M_IRQHandler(void);
void I2S_S_IRQHandler(void);
void ISO7816_IRQHandler(void);
void PRESENT_IRQHandler(void);
void CALENDAR_IRQHandler(void);
void DUAL_TIMER_IRQHandler(void);
void SVC_Handler(void);

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
