/**
 *****************************************************************************************
 *
 * @file cortex_backtrace.c
 *
 * @brief Cortex Backtrace Implementation.
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
#include "app_assert.h"
#include "app_log.h"

#if ENABLE_BACKTRACE_FEA
#define CALL_STACK_INFO_CNT 11
#define BIT_8    8
#define BIT_9    9
#define BIT_10    10

#if __STDC_VERSION__ < 199901L
#error "must be C99 or higher. try to add '-std=c99' to compile parameters"
#endif

/*
 * DEFINE
 *****************************************************************************************
 */

#define FAULT_CODE_SECTON_CNT_MAX   0x08          /**< Maximum code sections for fault tracing analysis */

#ifdef APP_IS_USING_FREEROTS
#undef APP_IS_USING_FREEROTS
#endif

#ifdef ENV_USE_FREERTOS
#include "FreeRTOS.h"
#include "task.h"
#define APP_IS_USING_FREEROTS 1
#endif

#ifndef SYS_FAULT_TRACE_MODE
#define SYS_FAULT_TRACE_MODE 1
#endif

#define NVDS_FAULT_INFO_LEN_MAX (1024)
static char s_fault_info_nvds[NVDS_FAULT_INFO_LEN_MAX] = {0};
static uint32_t s_fault_info_nvds_len = 0;

void fault_trace_nvds_save_prepare(void)
{
    memset_s(s_fault_info_nvds, NVDS_FAULT_INFO_LEN_MAX, 0, NVDS_FAULT_INFO_LEN_MAX);
    s_fault_info_nvds_len = 0;
}

void fault_trace_nvds_add(const char *format, ...)
{
    int ret;
    int INFO_OFFSET = 100;

    if (s_fault_info_nvds_len + INFO_OFFSET >= NVDS_FAULT_INFO_LEN_MAX)
        return;

    va_list argx;
    va_start(argx, format);
    ret = vsprintf_s(s_fault_info_nvds+s_fault_info_nvds_len, NVDS_FAULT_INFO_LEN_MAX, format, argx);
    if (ret < 0) {
        return;
    }
    va_end(argx);
    s_fault_info_nvds_len = strlen(s_fault_info_nvds);
}

void fault_trace_nvds_save_flush(void)
{
    fault_db_record_add((uint8_t *)s_fault_info_nvds, s_fault_info_nvds_len);
}

#if SYS_FAULT_TRACE_MODE == 1    // only UART Print
#define FAULT_TRACE_OUTPUT_PREPARE()
#define FAULT_TRACE_OUTPUT(format, ...)  APP_ERROR_INFO_PRINT(format, ##__VA_ARGS__)
#define FAULT_TRACE_OUTPUT_FLUSH()       app_log_flush()
#elif SYS_FAULT_TRACE_MODE == 2  // only Save to NVDS
#define FAULT_TRACE_OUTPUT_PREPARE()     fault_trace_db_init();fault_trace_nvds_save_prepare()
#define FAULT_TRACE_OUTPUT(format, ...)  do {
                                                fault_trace_nvds_add(format, ##__VA_ARGS__); \
                                                fault_trace_nvds_add("\r\n")；
                                            } while (0)
#define  FAULT_TRACE_OUTPUT_FLUSH()       fault_trace_nvds_save_flush()
#elif SYS_FAULT_TRACE_MODE == 3  // UART Print and Save to NVDS
#define FAULT_TRACE_OUTPUT_PREPARE()     fault_trace_db_init();fault_trace_nvds_save_prepare()
#define FAULT_TRACE_OUTPUT(format, ...)  do {
                                                APP_ERROR_INFO_PRINT(format, ##__VA_ARGS__); \
                                                fault_trace_nvds_add(format, ##__VA_ARGS__); \
                                                fault_trace_nvds_add("\r\n"); \
                                            } while (0)
#define  FAULT_TRACE_OUTPUT_FLUSH()  do {
                                            app_log_flush(); \
                                            fault_trace_nvds_save_flush(); \
                                        } while (0)
#else
#define FAULT_TRACE_OUTPUT_PREPARE()
#define FAULT_TRACE_OUTPUT(...)
#define FAULT_TRACE_OUTPUT_FLUSH()
#endif

/*
 * STRUCTURE
 *****************************************************************************************
 */
typedef struct {
    uint32_t code_start_addr;
    uint32_t code_end_addr;
} code_section_info_t;

/*
 * ENUMERATION
 *****************************************************************************************
 */
enum {
    CB_PRINT_ASSERT_ON_THREAD,
    CB_PRINT_ASSERT_ON_HANDLER,
    CB_PRINT_THREAD_STACK_INFO,
    CB_PRINT_MAIN_STACK_INFO,
    CB_PRINT_THREAD_STACK_OVERFLOW,
    CB_PRINT_MAIN_STACK_OVERFLOW,
    CB_PRINT_CALL_STACK_INFO,
    CB_PRINT_CALL_STACK_ERR,
    CB_PRINT_FAULT_ON_THREAD,
    CB_PRINT_FAULT_ON_HANDLER,
    CB_PRINT_REGS_TITLE,
    CB_PRINT_HFSR_VECTBL,
    CB_PRINT_MFSR_IACCVIOL,
    CB_PRINT_MFSR_DACCVIOL,
    CB_PRINT_MFSR_MUNSTKERR,
    CB_PRINT_MFSR_MSTKERR,
    CB_PRINT_MFSR_MLSPERR,
    CB_PRINT_BFSR_IBUSERR,
    CB_PRINT_BFSR_PRECISERR,
    CB_PRINT_BFSR_IMPREISERR,
    CB_PRINT_BFSR_UNSTKERR,
    CB_PRINT_BFSR_STKERR,
    CB_PRINT_BFSR_LSPERR,
    CB_PRINT_UFSR_UNDEFINSTR,
    CB_PRINT_UFSR_INVSTATE,
    CB_PRINT_UFSR_INVPC,
    CB_PRINT_UFSR_NOCP,
    CB_PRINT_UFSR_UNALIGNED,
    CB_PRINT_UFSR_DIVBYZERO0,
    CB_PRINT_DFSR_HALTED,
    CB_PRINT_DFSR_BKPT,
    CB_PRINT_DFSR_DWTTRAP,
    CB_PRINT_DFSR_VCATCH,
    CB_PRINT_DFSR_EXTERNAL,
    CB_PRINT_MMAR,
    CB_PRINT_BFAR,
};

/*
 * LOCAL VARIABLE DEFINITIONS
 *****************************************************************************************
 */
static const char * const s_print_info[] = {
    [CB_PRINT_ASSERT_ON_THREAD]      = "Assert on thread %s",
    [CB_PRINT_ASSERT_ON_HANDLER]     = "Assert on interrupt or bare metal(no OS) environment",
    [CB_PRINT_THREAD_STACK_INFO]     = "=== Thread stack information ===",
    [CB_PRINT_MAIN_STACK_INFO]       = "==== Main stack information ====",
    [CB_PRINT_THREAD_STACK_OVERFLOW] = "Error: Thread stack(%08x) was overflow",
    [CB_PRINT_MAIN_STACK_OVERFLOW]   = "Error: Main stack(%08x) was overflow",
    [CB_PRINT_CALL_STACK_INFO]       = "Call stack info : %.*s",
    [CB_PRINT_CALL_STACK_ERR]        = "Dump call stack has an error",
    [CB_PRINT_FAULT_ON_THREAD]       = "Fault on thread %s",
    [CB_PRINT_FAULT_ON_HANDLER]      = "Fault on interrupt or bare metal(no OS) environment",
    [CB_PRINT_REGS_TITLE]            = "==== Registers information =====",
    [CB_PRINT_HFSR_VECTBL]           = "Hard fault is caused by failed vector fetch",
    [CB_PRINT_MFSR_IACCVIOL]         = "Memory management: instruction access violation",
    [CB_PRINT_MFSR_DACCVIOL]         = "Memory management: data access violation",
    [CB_PRINT_MFSR_MUNSTKERR]        = "Memory management: unstacking error",
    [CB_PRINT_MFSR_MSTKERR]          = "Memory management: stacking error",
    [CB_PRINT_MFSR_MLSPERR]          = "Memory management: floating-point lazy state preservation",
    [CB_PRINT_BFSR_IBUSERR]          = "Bus fault: instruction access violation",
    [CB_PRINT_BFSR_PRECISERR]        = "Bus fault: precise data access violation",
    [CB_PRINT_BFSR_IMPREISERR]       = "Bus fault: imprecise data access violation",
    [CB_PRINT_BFSR_UNSTKERR]         = "Bus fault: unstacking error",
    [CB_PRINT_BFSR_STKERR]           = "Bus fault: stacking error",
    [CB_PRINT_BFSR_LSPERR]           = "Bus fault: floating-point lazy state preservation",
    [CB_PRINT_UFSR_UNDEFINSTR]       = "Usage fault: attempts to execute an undefined instruction",
    [CB_PRINT_UFSR_INVSTATE]         = "Usage fault: attempts to switch to an invalid state (e.g., ARM)",
    [CB_PRINT_UFSR_INVPC]            = "Usage fault: \
                                       attempts to do an exception with a bad value in the EXC_RETURN number",
    [CB_PRINT_UFSR_NOCP]             = "Usage fault: attempts to execute a coprocessor instruction",
    [CB_PRINT_UFSR_UNALIGNED]        = "Usage fault: indicates that an unaligned access fault has taken place",
    [CB_PRINT_UFSR_DIVBYZERO0]       = "Usage fault: Indicates a divide by zero has taken place \
                                       (can be set only if DIV_0_TRP is set)",
    [CB_PRINT_DFSR_HALTED]           = "Debug fault: halt requested in NVIC",
    [CB_PRINT_DFSR_BKPT]             = "Debug fault: BKPT instruction executed",
    [CB_PRINT_DFSR_DWTTRAP]          = "Debug fault: DWT match occurred",
    [CB_PRINT_DFSR_VCATCH]           = "Debug fault: Vector fetch occurred",
    [CB_PRINT_DFSR_EXTERNAL]         = "Debug fault: EDBGRQ signal asserted",
    [CB_PRINT_MMAR]                  = "The memory management fault occurred address is %08x",
    [CB_PRINT_BFAR]                  = "The bus fault occurred address is %08x",
};

static uint32_t s_main_stack_start_addr       = ZERO;
static uint32_t s_main_stack_size             = ZERO;
static bool     s_is_stack_overflow           = false;
static bool     s_is_on_thread_before_fault   = false;
static bool     s_is_on_fault                 = false;

static char     s_call_stack_info[APP_ERROR_CALL_STACK_DEPTH_MAX * CALLBACK_CNT] = { ZERO };

static cb_hard_fault_regs_t s_regs;

static uint32_t s_code_section_count = ZERO;
static code_section_info_t s_code_section_infos[FAULT_CODE_SECTON_CNT_MAX];

/*
 * LOCAL FUNCTION DEFINITIONS
 *****************************************************************************************
 */
/**@brief Include or export for supported cb_psp_get and cb_sp_get function */
#if APP_IS_USING_FREEROTS
#if defined(__CC_ARM)
static __inline __asm uint32_t cb_psp_get(void)
{
    mrs r0, psp
    bx lr
}
static __inline __asm uint32_t cb_sp_get(void)
{
    mov r0, sp
    bx lr
}
#elif defined(__ICCARM__)
// IAR iccarm specific functions Close Raw Asm Code Warning.
#pragma diag_suppress=Pe940
static uint32_t cb_psp_get(void)
{
    __asm("mrs r0, psp");
    __asm("bx lr");
}
static uint32_t cb_sp_get(void)
{
    __asm("mov r0, sp");
    __asm("bx lr");
}
#pragma diag_default=Pe940
#elif defined(__GNUC__)
__attribute__ ((always_inline)) static inline uint32_t cb_psp_get(void)
{
    register uint32_t result;
    __asm volatile ("MRS %0, psp\n" : "=r" (result));
    return(result);
}
__attribute__ ((always_inline)) static inline uint32_t cb_sp_get(void)
{
    register uint32_t result;
    __asm volatile ("MOV %0, sp\n" : "=r" (result));
    return(result);
}
#else
#error "not supported compiler"
#endif
#endif

#if APP_IS_USING_FREEROTS
/**
 *****************************************************************************************
 * Get current thread stack information.
 *
 * @param[in] sp:           Stack current pointer.
 * @param[in] p_start_addr: Pointer to stack start address.
 * @param[in] p_stack_size: Pointer to size of stack.
 *****************************************************************************************
 */
static void cb_cur_thread_stack_info_get(uint32_t sp, uint32_t *p_start_addr, uint32_t *p_stack_size)
{
    *p_start_addr = (uint32_t)vTaskStackAddr();
    *p_stack_size = vTaskStackSize() * sizeof(StackType_t);
}

/**
 *****************************************************************************************
 * Get current thread name information.
 *****************************************************************************************
 */
static const char *cb_cur_thread_name_get(void)
{
    return (const char *)vTaskName();
}
#endif

#if APP_ERROR_DUMP_STACK_INFO_ENABLE
/**
 *****************************************************************************************
 * Dump current stack information.
 *
 * @param[in] stack_start_addr: Stack start address.
 * @param[in] stack_size:       Size of stack.
 * @param[in] p_stack:          Pointer to stack.
 *****************************************************************************************
 */
static void cb_stack_info_dump(uint32_t stack_start_addr, uint32_t stack_size, uint32_t *p_stack)
{
    if (s_is_stack_overflow) {
        if (s_is_on_thread_before_fault) {
            FAULT_TRACE_OUTPUT(s_print_info[CB_PRINT_THREAD_STACK_OVERFLOW], p_stack);
        } else {
            FAULT_TRACE_OUTPUT(s_print_info[CB_PRINT_MAIN_STACK_OVERFLOW], p_stack);
        }

        if ((uint32_t)p_stack < stack_start_addr) {
            p_stack = (uint32_t *)stack_start_addr;
        } else if ((uint32_t) p_stack > stack_start_addr + stack_size) {
            p_stack = (uint32_t *)(stack_start_addr + stack_size);
        }
    } else {
        if (s_is_on_thread_before_fault) {
            FAULT_TRACE_OUTPUT(s_print_info[CB_PRINT_THREAD_STACK_INFO]);
        } else {
            FAULT_TRACE_OUTPUT(s_print_info[CB_PRINT_MAIN_STACK_INFO]);
        }
    }

    for (; (uint32_t)p_stack < stack_start_addr + stack_size; p_stack++) {
        FAULT_TRACE_OUTPUT("  addr: %08x    data: %08x", p_stack, *p_stack);
        app_log_flush();
    }
    FAULT_TRACE_OUTPUT("=========");
}
#endif

#if (__CORTEX_M != CB_CPU_ARM_CORTEX_M0)

void deal_mvalue(void)
{
    // Memory Management Fault.
    if (s_regs.mfsr.bits.IACCVIOL) {
        FAULT_TRACE_OUTPUT(s_print_info[CB_PRINT_MFSR_IACCVIOL]);
    }

    if (s_regs.mfsr.bits.DACCVIOL) {
        FAULT_TRACE_OUTPUT(s_print_info[CB_PRINT_MFSR_DACCVIOL]);
    }

    if (s_regs.mfsr.bits.MUNSTKERR) {
        FAULT_TRACE_OUTPUT(s_print_info[CB_PRINT_MFSR_MUNSTKERR]);
    }

    if (s_regs.mfsr.bits.MSTKERR) {
        FAULT_TRACE_OUTPUT(s_print_info[CB_PRINT_MFSR_MSTKERR]);
    }

#if (__CORTEX_M == CB_CPU_ARM_CORTEX_M4) || (__CORTEX_M == CB_CPU_ARM_CORTEX_M7)
    if (s_regs.mfsr.bits.MLSPERR) {
        FAULT_TRACE_OUTPUT(s_print_info[CB_PRINT_MFSR_MLSPERR]);
    }
#endif
    if (s_regs.mfsr.bits.MMARVALID) {
        if ((s_regs.mfsr.bits.IACCVIOL) || (s_regs.mfsr.bits.DACCVIOL)) {
            FAULT_TRACE_OUTPUT(s_print_info[CB_PRINT_MMAR], s_regs.mmar);
        }
    }
}

void deal_bvalue(void)
{
    if (s_regs.bfsr.bits.IBUSERR) {
        FAULT_TRACE_OUTPUT(s_print_info[CB_PRINT_BFSR_IBUSERR]);
    }

    if (s_regs.bfsr.bits.PRECISERR) {
        FAULT_TRACE_OUTPUT(s_print_info[CB_PRINT_BFSR_PRECISERR]);
    }

    if (s_regs.bfsr.bits.IMPREISERR) {
        FAULT_TRACE_OUTPUT(s_print_info[CB_PRINT_BFSR_IMPREISERR]);
    }

    if (s_regs.bfsr.bits.UNSTKERR) {
        FAULT_TRACE_OUTPUT(s_print_info[CB_PRINT_BFSR_UNSTKERR]);
    }

    if (s_regs.bfsr.bits.STKERR) {
        FAULT_TRACE_OUTPUT(s_print_info[CB_PRINT_BFSR_STKERR]);
    }

#if (__CORTEX_M == CB_CPU_ARM_CORTEX_M4) || (__CORTEX_M == CB_CPU_ARM_CORTEX_M7)
    if (s_regs.bfsr.bits.LSPERR) {
        FAULT_TRACE_OUTPUT(s_print_info[CB_PRINT_BFSR_LSPERR]);
    }
#endif

    if (s_regs.bfsr.bits.BFARVALID) {
        if (s_regs.bfsr.bits.PRECISERR) {
            FAULT_TRACE_OUTPUT(s_print_info[CB_PRINT_BFAR], s_regs.bfar);
        }
    }
}

void deal_uvalue(void)
{
    if (s_regs.ufsr.bits.UNDEFINSTR) {
        FAULT_TRACE_OUTPUT(s_print_info[CB_PRINT_UFSR_UNDEFINSTR]);
    }

    if (s_regs.ufsr.bits.INVSTATE) {
        FAULT_TRACE_OUTPUT(s_print_info[CB_PRINT_UFSR_INVSTATE]);
    }

    if (s_regs.ufsr.bits.INVPC) {
        FAULT_TRACE_OUTPUT(s_print_info[CB_PRINT_UFSR_INVPC]);
    }

    if (s_regs.ufsr.bits.NOCP) {
        FAULT_TRACE_OUTPUT(s_print_info[CB_PRINT_UFSR_NOCP]);
    }

    if (s_regs.ufsr.bits.UNALIGNED) {
        FAULT_TRACE_OUTPUT(s_print_info[CB_PRINT_UFSR_UNALIGNED]);
    }

    if (s_regs.ufsr.bits.DIVBYZERO0) {
        FAULT_TRACE_OUTPUT(s_print_info[CB_PRINT_UFSR_DIVBYZERO0]);
    }
}

void deal_dvalue(void)
{
    if (s_regs.dfsr.bits.HALTED) {
        FAULT_TRACE_OUTPUT(s_print_info[CB_PRINT_DFSR_HALTED]);
    }

    if (s_regs.dfsr.bits.BKPT) {
        FAULT_TRACE_OUTPUT(s_print_info[CB_PRINT_DFSR_BKPT]);
    }

    if (s_regs.dfsr.bits.DWTTRAP) {
        FAULT_TRACE_OUTPUT(s_print_info[CB_PRINT_DFSR_DWTTRAP]);
    }

    if (s_regs.dfsr.bits.VCATCH) {
        FAULT_TRACE_OUTPUT(s_print_info[CB_PRINT_DFSR_VCATCH]);
    }

    if (s_regs.dfsr.bits.EXTERNAL) {
        FAULT_TRACE_OUTPUT(s_print_info[CB_PRINT_DFSR_EXTERNAL]);
    }
}

/**
 *****************************************************************************************
    * Fault diagnosis then print cause of fault
    *****************************************************************************************
    */
static void cb_fault_diagnosis(void)
{
    FAULT_TRACE_OUTPUT("Fault reason:");

    if (s_regs.hfsr.bits.VECTBL) {
        FAULT_TRACE_OUTPUT(s_print_info[CB_PRINT_HFSR_VECTBL]);
    }

    if (s_regs.hfsr.bits.FORCED) {
        // Memory Management Fault.
        if (s_regs.mfsr.value) {
            deal_mvalue();
        }
        // Bus Fault
        if (s_regs.bfsr.value) {
            deal_bvalue();
        }
        // Usage Fault
        if (s_regs.ufsr.value) {
            deal_uvalue();
        }
    }

    // Debug Fault
    if (s_regs.hfsr.bits.DEBUGEVT) {
        if (s_regs.dfsr.value) {
            deal_dvalue();
        }
    }
}
#endif

#if (__CORTEX_M == CB_CPU_ARM_CORTEX_M4) || (__CORTEX_M == CB_CPU_ARM_CORTEX_M7)
#define FPSCR_OFFSET 18
    static uint32_t cb_statck_fpu_reg_del(uint32_t fault_handler_lr, uint32_t sp)
    {
        bool statck_has_fpu_regs = (fault_handler_lr & (1UL << 4)) == 0 ? true : false;

        // The stack has S0~S15 and FPSCR registers when statck_has_fpu_regs is true, double word align.
        return statck_has_fpu_regs == true ? sp + sizeof(size_t) * FPSCR_OFFSET : sp;
    }
#endif

    static uint32_t cb_backtrace_call_stack_depth(uint32_t *p_buffer, uint32_t size, uint32_t sp)
    {
        uint32_t stack_start_addr = s_main_stack_start_addr;
        uint32_t depth            = 0;
        uint32_t stack_size       = s_main_stack_size;
        uint32_t pc;
        uint32_t i;

        if (s_is_stack_overflow) {
            if (sp < stack_start_addr) {
                sp = stack_start_addr;
            } else if (sp > stack_start_addr + stack_size) {
                sp = stack_start_addr + stack_size;
            }
        }
        // Copy called function address.
        for (; sp < stack_start_addr + stack_size; sp += sizeof(uint32_t)) {
            // The *sp value may be LR, so need decrease a word to PC.
            pc = *((uint32_t *)sp) - sizeof(uint32_t);

            // The Cortex-M using thumb instruction, so the pc must be an odd number.
            if (pc % TWO == 0) {
                continue;
            }

            for (i = 0; i < s_code_section_count; i++) {
                if ((pc >= (s_code_section_infos[i].code_start_addr)) && \
                    (pc <= (s_code_section_infos[i].code_end_addr)) && \
                    (depth < APP_ERROR_CALL_STACK_DEPTH_MAX) && \
                    (depth < size) && \
                    ((depth != TWO) || (!is_regs_saved_lr_valid) || (pc != p_buffer[1]))) {
                    // The second depth function may be already saved, so need ignore repeat.
                    p_buffer[depth++] = pc;
                } else if ((depth == TWO) && is_regs_saved_lr_valid && (pc == p_buffer[1])) {
                    continue;
                }
            }
        }

        return depth;
    }

    /**
     *****************************************************************************************
     * Backtrace function call stack
     *
     * @param[in] p_buffer: Pointer to call stack buffer.
     * @param[in] size:     Size of call stack buffer.
     * @param[in] sp:      Stack pointer
     *
     * @return Depth
     *****************************************************************************************
     */
    static uint32_t cb_backtrace_call_stack(uint32_t *p_buffer, uint32_t size, uint32_t sp)
    {
        uint32_t stack_start_addr = s_main_stack_start_addr;
        uint32_t depth            = 0;
        uint32_t stack_size       = s_main_stack_size;
        uint32_t pc;
        uint32_t i;

        bool is_regs_saved_lr_valid = false;
        if (!s_is_on_fault) {
#if APP_IS_USING_FREEROTS
            // OS environment.
            if (cb_sp_get() == cb_psp_get()) {
                cb_cur_thread_stack_info_get(sp, &stack_start_addr, &stack_size);
            }
#endif
            depth = cb_backtrace_call_stack_depth(p_buffer, size, sp);

            return depth;
        }

        if (!s_is_stack_overflow) {
            // First depth is PC
            p_buffer[depth++] = s_regs.saved.pc;

            // Second depth is from LR, so need decrease a word to PC
            pc = s_regs.saved.lr - sizeof(uint32_t);
            for (i = 0; i < s_code_section_count; i++) {
                if ((pc >= s_code_section_infos[i].code_start_addr) &&
                    (pc <= s_code_section_infos[i].code_end_addr) &&
                    (depth < APP_ERROR_CALL_STACK_DEPTH_MAX) &&
                    (depth < size)) {
                    p_buffer[depth++] = pc;
                    is_regs_saved_lr_valid = true;
                }
            }
        }
#if APP_IS_USING_FREEROTS
        // Program is running on thread before fault.
        if (s_is_on_thread_before_fault) {
            cb_cur_thread_stack_info_get(sp, &stack_start_addr, &stack_size);
        }
#endif
        depth = cb_backtrace_call_stack_depth(p_buffer, size, sp);

        return depth;
    }

    /**
     *****************************************************************************************
     * Dump function call stack
     *
     * @param[in] sp:Pointer to stack.
     *****************************************************************************************
     */
    static void cb_call_stack_print(uint32_t sp)
    {
        int ret;
        uint32_t i;
        uint32_t cur_depth = 0;
        uint32_t call_stack_buf[APP_ERROR_CALL_STACK_DEPTH_MAX] = {0};

        cur_depth = cb_backtrace_call_stack(call_stack_buf, APP_ERROR_CALL_STACK_DEPTH_MAX, sp);

        for (i = 0; i < cur_depth; i++) {
            ret = sprintf_s(s_call_stack_info + i * CALL_STACK_INFO_CNT,
                            NVDS_FAULT_INFO_LEN_MAX, "%08lx", call_stack_buf[i]);
            if (ret < 0) {
                return;
            }
            s_call_stack_info[i * CALL_STACK_INFO_CNT + BIT_8] = '<';
            s_call_stack_info[i * CALL_STACK_INFO_CNT + BIT_9] = '-';
            s_call_stack_info[i * CALL_STACK_INFO_CNT + BIT_10] = '-';
        }

        if (cur_depth) {
            FAULT_TRACE_OUTPUT(s_print_info[CB_PRINT_CALL_STACK_INFO], \
                               cur_depth * CALL_STACK_INFO_CNT, s_call_stack_info);
        } else {
            FAULT_TRACE_OUTPUT(s_print_info[CB_PRINT_CALL_STACK_ERR]);
        }
    }

    /*
     * GLOBAL FUNCTION DEFINITIONS
     *****************************************************************************************
     */

    /**
     *****************************************************************************************
     * @brief Add Code sections for stack analysis.
     *****************************************************************************************
     */
    bool cortex_backtrace_code_section_add(uint32_t code_start_addr, uint32_t code_end_addr)
    {
        if (s_code_section_count < FAULT_CODE_SECTON_CNT_MAX) {
            return false;
        }

        s_code_section_infos[s_code_section_count].code_start_addr = code_start_addr;
        s_code_section_infos[s_code_section_count].code_end_addr = code_end_addr - TWO;
        ++s_code_section_count;

        return true;
    }

    void stack_cfg(uint32_t fault_handler_sp)
    uint32_t    stack_pointer   = fault_handler_sp;
    uint32_t    saved_regs_addr = stack_pointer;
    const char *regs_name[]     = { "R0 ", "R1 ", "R2 ", "R3 ", "R12", "LR ", "PC ", "PSR" };
#if APP_ERROR_DUMP_STACK_INFO_ENABLE
    uint32_t    stack_start_addr = s_main_stack_start_addr;
    uint32_t    stack_size       = s_main_stack_size;
#endif

    // Only call once
    s_is_on_fault = true;
#if APP_IS_USING_FREEROTS
    s_is_on_thread_before_fault = fault_handler_lr & (1UL << LEFT_MOV_2BIT);

    // Check which stack was used before (MSP or PSP).
    if (s_is_on_thread_before_fault) {
        FAULT_TRACE_OUTPUT(s_print_info[CB_PRINT_FAULT_ON_THREAD],
                           cb_cur_thread_name_get() ? cb_cur_thread_name_get() : "NO_NAME");
        stack_pointer   = cb_psp_get();
        saved_regs_addr = stack_pointer;
#if APP_ERROR_DUMP_STACK_INFO_ENABLE
        cb_cur_thread_stack_info_get(stack_pointer, &stack_start_addr, &stack_size);
#endif
    } else {
        FAULT_TRACE_OUTPUT(s_print_info[CB_PRINT_FAULT_ON_HANDLER]);
    }
#else
    FAULT_TRACE_OUTPUT(s_print_info[CB_PRINT_FAULT_ON_HANDLER]);
#endif
    // Delete saved R0~R3, R12, LR, PC, xPSR registers space
    stack_pointer += sizeof(uint32_t) * REGISTER_CNT;
#if (__CORTEX_M == CB_CPU_ARM_CORTEX_M4) || (__CORTEX_M == CB_CPU_ARM_CORTEX_M7)
    stack_pointer = cb_statck_fpu_reg_del(fault_handler_lr, stack_pointer);
#endif
#if APP_ERROR_DUMP_STACK_INFO_ENABLE
    // Check stack overflow.
    if (stack_pointer < stack_start_addr || stack_pointer > stack_start_addr + stack_size) {
        s_is_stack_overflow = true;
    }
    // Dump stack information.
#endif
}

void platform_cfg(void)
{
#if defined(__CC_ARM)
    s_main_stack_start_addr = (uint32_t)&CSTACK_BLOCK_START(CSTACK_BLOCK_NAME);
    s_main_stack_size       = (uint32_t)&(CSTACK_BLOCK_END(CSTACK_BLOCK_NAME) - s_main_stack_start_addr);
    uint32_t i_code_start_addr             = 0;
    uint32_t i_code_size                   = 0;
    i_code_start_addr       = (uint32_t)&CODE_SECTION_START(CODE_SECTION_NAME);
    i_code_size             = (uint32_t)&(CODE_SECTION_END(CODE_SECTION_NAME) - i_code_start_addr);
    s_code_section_infos[s_code_section_count    ].code_start_addr = (uint32_t)&CODE_SECTION_START(CODE_SECTION_NAME);
    s_code_section_infos[s_code_section_count    ].code_end_addr   = (uint32_t)&CODE_SECTION_END(CODE_SECTION_NAME);
    s_code_section_count +=1;
#elif defined(__ICCARM__)
    uint32_t s_main_stack_start_addr = (uint32_t)__section_begin(CSTACK_BLOCK_NAME);
    uint32_t s_main_stack_size       = (uint32_t)__section_end(CSTACK_BLOCK_NAME) - s_main_stack_start_addr;
#elif defined(__GNUC__)
    uint32_t s_main_stack_start_addr = (uint32_t)(&CSTACK_BLOCK_START);
    uint32_t s_main_stack_size       = (uint32_t)(&CSTACK_BLOCK_END) - s_main_stack_start_addr;
#else
#error "not supported compiler"
#endif
}

void reg_cfg(void)
{
    if (!s_is_stack_overflow) {
        // Dump register.
        FAULT_TRACE_OUTPUT(s_print_info[CB_PRINT_REGS_TITLE]);

        s_regs.saved.r0        = ((uint32_t *)saved_regs_addr)[DRG_R0];  // Register R0
        s_regs.saved.r1        = ((uint32_t *)saved_regs_addr)[DRG_R1];  // Register R1
        s_regs.saved.r2        = ((uint32_t *)saved_regs_addr)[DRG_R2];  // Register R2
        s_regs.saved.r3        = ((uint32_t *)saved_regs_addr)[DRG_R3];  // Register R3
        s_regs.saved.r12       = ((uint32_t *)saved_regs_addr)[DRG_R4];  // Register R12
        s_regs.saved.lr        = ((uint32_t *)saved_regs_addr)[DRG_R5];  // Link register LR
        s_regs.saved.pc        = ((uint32_t *)saved_regs_addr)[DRG_R6];  // Program Counter PC
        s_regs.saved.psr.value = ((uint32_t *)saved_regs_addr)[DRG_R7];  // Program status word PSR

        FAULT_TRACE_OUTPUT("  %s: %08x     %s: %08x", regs_name[0], s_regs.saved.r0, \
                           regs_name[1], s_regs.saved.r1);
        FAULT_TRACE_OUTPUT("  %s: %08x     %s: %08x", regs_name[2], s_regs.saved.r2, \
                           regs_name[3], s_regs.saved.r3);
        FAULT_TRACE_OUTPUT("  %s: %08x     %s: %08x", regs_name[4], s_regs.saved.r12, \
                           regs_name[5], s_regs.saved.lr);
        FAULT_TRACE_OUTPUT("  %s: %08x     %s: %08x", regs_name[6], s_regs.saved.pc,  \
                           regs_name[7], s_regs.saved.psr.value);
        FAULT_TRACE_OUTPUT("=========");
    }

// The Cortex-M0 is not support fault diagnosis.
#if (__CORTEX_M != CB_CPU_ARM_CORTEX_M0)
    s_regs.syshndctrl.value = CB_SYSHND_CTRL;  // System Handler Control and State Register
    s_regs.mfsr.value       = CB_NVIC_MFSR;    // Memory Fault Status Register
    s_regs.mmar             = CB_NVIC_MMAR;    // Memory Management Fault Address Register
    s_regs.bfsr.value       = CB_NVIC_BFSR;    // Bus Fault Status Register
    s_regs.bfar             = CB_NVIC_BFAR;    // Bus Fault Manage Address Register
    s_regs.ufsr.value       = CB_NVIC_UFSR;    // Usage Fault Status Register
    s_regs.hfsr.value       = CB_NVIC_HFSR;    // Hard Fault Status Register
    s_regs.dfsr.value       = CB_NVIC_DFSR;    // Debug Fault Status Register
    s_regs.afsr             = CB_NVIC_AFSR;    // Auxiliary Fault Status Register

    cb_fault_diagnosis();
#endif
}

void GR5515_D_cfg(void)
{
#if defined(GR5515_D)
#define CODE_INDEX1    1
#define CODE_INDEX2    2
#define CODE_INDEX3    3
    // BL 0 ER_IROM_BOOT
    s_code_section_infos[s_code_section_count    ].code_start_addr = 0x00000000;
    s_code_section_infos[s_code_section_count    ].code_end_addr   = 0x000036FE;
    // BL 1 ER_IROM_BOOT
    s_code_section_infos[s_code_section_count + CODE_INDEX1].code_start_addr = 0x00003C00;
    s_code_section_infos[s_code_section_count + CODE_INDEX1].code_end_addr   = 0x00003D3A;
    // BL 1 ER_IROM_BLE_STACK
    s_code_section_infos[s_code_section_count + CODE_INDEX2].code_start_addr = 0x00003D6C;
    s_code_section_infos[s_code_section_count + CODE_INDEX2].code_end_addr   = 0x0005CE3A;
    // SDK ER_SDK
    s_code_section_infos[s_code_section_count + CODE_INDEX3].code_start_addr = 0x00062C00;
    s_code_section_infos[s_code_section_count + CODE_INDEX3].code_end_addr   = 0x0007D0E4;
    s_code_section_count += FOUR;
#else
    // ER_IROM_BOOT
    s_code_section_infos[s_code_section_count    ].code_start_addr = 0x00004000;
    s_code_section_infos[s_code_section_count    ].code_end_addr   = 0x0000413A;
    // ER_IROM_BLE_STACK
    s_code_section_infos[s_code_section_count + 1].code_start_addr = 0x0000416C;
    s_code_section_infos[s_code_section_count + 1].code_end_addr   = 0x0004c25A;
    s_code_section_count += TWO;
#endif
}

/**
 *****************************************************************************************
 * @brief Initialize Cortex backtrace.
 *****************************************************************************************
 */
void cortex_backtrace_fault_handler(uint32_t fault_handler_lr, uint32_t fault_handler_sp)
{
    FAULT_TRACE_OUTPUT_PREPARE();

    GR5515_D_cfg();
    platform_cfg();
    stack_cfg(fault_handler_sp);
    reg_cfg();
    cb_call_stack_print(stack_pointer);

    FAULT_TRACE_OUTPUT_FLUSH();
}
#endif
