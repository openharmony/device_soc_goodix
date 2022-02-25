/**************************************************************************//**
 * @file     system_gr55xx.h
 * @brief    CMSIS Cortex-M# Device Peripheral Access Layer Header File for
 *           Device GR55xx
 * @version  V1.00
 * @date     12. June 2018
 ******************************************************************************/
/*
 * Copyright (c) 2016-2018, Shenzhen Huiding Technology Co., Ltd
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Licensed under the Apache License, Version 2.0 (the License); you may
 * not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an AS IS BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef __SYSTEM_GR55xx_H__
#define __SYSTEM_GR55xx_H__

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define CLK_64M        64000000
#define CLK_48M        48000000
#define CLK_32M        32000000
#define CLK_24M        24000000
#define CLK_16M        16000000

typedef enum {
    XO_S16M_CLK   = 2,
    CPLL_S16M_CLK = 4,
    CPLL_T24M_CLK = 3,
    CPLL_T32M_CLK = 5,
    CPLL_F48M_CLK = 1,
    CPLL_S64M_CLK = 0,
    CLK_TYPE_NUM  = 6,
} mcu_clock_type_t;

typedef enum {
    QSPI_64M_CLK  = 0,
    QSPI_48M_CLK  = 1,
    QSPI_32M_CLK  = 2,
    QSPI_24M_CLK  = 3,
    QSPI_16M_CLK  = 4,
    QSPI_CLK_TYPE_NUM  = 5,
} qspi_clock_type_t;

/** @addtogroup Peripheral_interrupt_number_definition
  * @{
  */

/**
 * @brief GR55xx Interrupt Number Definition, according to the selected device
 *        in @ref Library_configuration_section
 */

/* ================================================================================================================= */
/* ================                           Interrupt Number Definition                           ================ */
/* ================================================================================================================= */
typedef enum IRQn {
/* ==================================  ARM Cortex-M# Specific Interrupt Numbers  =================================== */

    NonMaskableInt_IRQn       = -14,  /**< -14  Non maskable Interrupt, cannot be stopped or preempted               */
    HardFault_IRQn            = -13,  /**< -13  Hard Fault, all classes of Fault                                     */
    MemoryManagement_IRQn     = -12,  /**< -12  Memory Management, MPU mismatch, including Access Violation
                                                and No Match                                                         */
    BusFault_IRQn             = -11,  /**< -11  Bus Fault, Pre-Fetch-, Memory Access Fault, other address/memory
                                                related Fault                                                        */
    UsageFault_IRQn           = -10,  /**< -10  Usage Fault, i.e. Undef Instruction, Illegal State Transition        */
    SVCall_IRQn               =  -5,  /**< -5 System Service Call via SVC instruction                                */
    DebugMonitor_IRQn         =  -4,  /**< -4 Debug Monitor                                                          */
    PendSV_IRQn               =  -2,  /**< -2 Pendable request for system service                                    */
    SysTick_IRQn              =  -1,  /**< -1 System Tick Timer                                                      */

/* ======================================  <Device> Specific Interrupt Numbers  ==================================== */
    WDT_IRQn                  =   0,  /**< Watchdog Timer Interrupt                                                  */
    BLE_SDK_IRQn              =   1,  /**< BLE_SDK_SCHEDULE Interrupt                                                */
    BLE_IRQn                  =   2,  /**< BLE Interrupt                                                             */
    DMA_IRQn                  =   3,  /**< DMA Interrupt                                                             */
    SPI_M_IRQn                =   4,  /**< SPI_M Interrupt                                                           */
    SPI_S_IRQn                =   5,  /**< SPI_S Interrupt                                                           */
    EXT0_IRQn                 =   6,  /**< EXT0 Interrupt                                                            */
    EXT1_IRQn                 =   7,  /**< EXT1 Interrupt                                                            */
    TIMER0_IRQn               =   8,  /**< Timer0 Interrupt                                                          */
    TIMER1_IRQn               =   9,  /**< Timer1 Interrupt                                                          */
    DUAL_TIMER_IRQn           =  10,  /**< Dual_Timer Interrupt                                                      */
    QSPI0_IRQn                =  11,  /**< QSPI0 Interrupt                                                           */
    UART0_IRQn                =  12,  /**< UART0 Interrupt                                                           */
    UART1_IRQn                =  13,  /**< UART1 Interrupt                                                           */
    I2C0_IRQn                 =  14,  /**< I2C0 Interrupt                                                            */
    I2C1_IRQn                 =  15,  /**< I2C1 Interrupt                                                            */
    AES_IRQn                  =  16,  /**< AES Interrupt                                                             */
    HMAC_IRQn                 =  17,  /**< HMAC Interrupt                                                            */
    EXT2_IRQn                 =  18,  /**< EXT2 Interrupt                                                            */
    RNG_IRQn                  =  19,  /**< RNG Interrupt                                                             */
    PMU_IRQn                  =  20,  /**< PMU Interrupt                                                             */
    PKC_IRQn                  =  21,  /**< PKC Interrupt                                                             */
    XQSPI_IRQn                =  22,  /**< XQSPI Interrupt                                                           */
    QSPI1_IRQn                =  23,  /**< QSPI1 Interrupt                                                           */
    PWR_CMD_IRQn              =  24,  /**< POWER CMD ACK Interrupt                                                   */
    BLESLP_IRQn               =  25,  /**< BLE Sleep Interrupt                                                       */
    SLPTIMER_IRQn             =  26,  /**< Sleep Timer Interrupt                                                     */
    COMP_EXT_IRQn             =  27,  /**< Comparator and External Wakeup Interrupt                                  */
    AON_WDT_IRQn              =  28,  /**< Always on Watchdog Interrupt                                              */
    I2S_M_IRQn                =  29,  /**< I2S_M Interrupt                                                           */
    I2S_S_IRQn                =  30,  /**< I2S_S Interrupt                                                           */
    ISO7816_IRQn              =  31,  /**< ISO7816 Interrupt                                                         */
    PRESENT_IRQn              =  32,  /**< Presnet Done Interrupt                                                    */
    CALENDAR_IRQn             =  33,  /**< AON Calendar Timer Interrupt                                              */
    MAX_NUMS_IRQn             =  34,  /**< Last Interrupt                                                            */
} IRQn_Type;

/** @} */ /* End of group Peripheral     _interrupt_number_definition */

/* ================================================================================================================= */
/* ================                        Processor and Core Peripheral Section                    ================ */
/* ================================================================================================================= */

/* ===================================  Start of section using anonymous unions  =================================== */

/* ======================  Configuration of the ARM Cortex-M4 Processor and Core Peripherals  ====================== */
#define __CM4_REV                 0x0001U   /* Core revision r0p1 */
#define __MPU_PRESENT             1         /* MPU present */
#define __VTOR_PRESENT            1         /* VTOR present */
#define __NVIC_PRIO_BITS          8         /* Number of Bits used for Priority Levels */
#define __Vendor_SysTickConfig    0         /* Set to 1 if different SysTick Config is used */
#define __FPU_PRESENT             1         /* FPU present */

extern uint32_t SystemCoreClock;     /*!< System Clock Frequency (Core Clock) */

/** @brief Setup the microcontroller system.

    Initialize the System and update the SystemCoreClock variable.
 */
extern void SystemInit(void);

/** \brief  Update SystemCoreClock variable.

    Updates the SystemCoreClock with current core Clock
    retrieved from cpu registers.
 */
extern void SystemCoreSetClock(mcu_clock_type_t clock);

/** \brief  Get SystemCoreClock variable.

    Get the SystemCoreClock with current core Clock
    retrieved from cpu registers.
 */
extern void SystemCoreGetClock(mcu_clock_type_t *clock);


extern void SystemCoreUpdateClock(void);


#ifdef __cplusplus
}
#endif

#endif /* __SYSTEM_GR55xx_H__ */
