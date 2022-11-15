/**************************************************************************//**
 * @file     gr551x.ld
 * @brief    Linker script for GR551X Device
 * @version  V1.00
 * @date     10. September 2021
 *
 * @note
 *
 ******************************************************************************/
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
 
/* Entry Point */
ENTRY(Reset_Handler)

_Min_Heap_Size = 0x800;	  /* required amount of heap  */
_Min_Stack_Size = 0x2000;	/* required amount of stack */

/* Highest address of the user mode stack */
_estack = ORIGIN(RAM) + LENGTH(RAM);	/* end of "RAM" Ram type memory */
_sstack = _estack - _Min_Stack_Size;

/* Memories definition */
MEMORY
{
  FPB_RAM (xrw)   : ORIGIN = 0x30004000,   LENGTH = 0x20
  RAM     (xrw)   : ORIGIN = 0x00804400,   LENGTH = 239K
  FLASH   (rx)    : ORIGIN = 0x0100B000,   LENGTH = 950K
}

/* Sections */
SECTIONS
{
  /* The startup code into "FLASH" Rom type memory */
  .isr_vector :
  {
    . = ALIGN(4);
    KEEP(*(.isr_vector)) /* Startup code */
    . = ALIGN(4);
  } >FLASH

   /* The program code and other data goes into FLASH */
  .text :
  {
    . = ALIGN(4);
    _stext = .;
    *(.text)           /* .text sections (code) */
    *(.text*)          /* .text* sections (code) */
    *(.glue_7)         /* glue arm to thumb code */
    *(.glue_7t)        /* glue thumb to arm code */
    *(.eh_frame)

	   __zinitcall_bsp_start = .;
	  KEEP (*(.zinitcall.bsp0.init))
	  KEEP (*(.zinitcall.bsp1.init))
	  KEEP (*(.zinitcall.bsp2.init))
	  KEEP (*(.zinitcall.bsp3.init))
	  KEEP (*(.zinitcall.bsp4.init))
	  __zinitcall_bsp_end = .;
	  __zinitcall_device_start = .;
	  KEEP (*(.zinitcall.device0.init))
	  KEEP (*(.zinitcall.device1.init))
	  KEEP (*(.zinitcall.device2.init))
	  KEEP (*(.zinitcall.device3.init))
	  KEEP (*(.zinitcall.device4.init))
	  __zinitcall_device_end = .;
	  __zinitcall_core_start = .;
	  KEEP (*(.zinitcall.core0.init))
	  KEEP (*(.zinitcall.core1.init))
	  KEEP (*(.zinitcall.core2.init))
	  KEEP (*(.zinitcall.core3.init))
	  KEEP (*(.zinitcall.core4.init))
	  __zinitcall_core_end = .;
	  __zinitcall_sys_service_start = .;
	  KEEP (*(.zinitcall.sys.service0.init))
	  KEEP (*(.zinitcall.sys.service1.init))
	  KEEP (*(.zinitcall.sys.service2.init))
	  KEEP (*(.zinitcall.sys.service3.init))
	  KEEP (*(.zinitcall.sys.service4.init))
	  __zinitcall_sys_service_end = .;
	  __zinitcall_sys_feature_start = .;
	  KEEP (*(.zinitcall.sys.feature0.init))
	  KEEP (*(.zinitcall.sys.feature1.init))
	  KEEP (*(.zinitcall.sys.feature2.init))
	  KEEP (*(.zinitcall.sys.feature3.init))
	  KEEP (*(.zinitcall.sys.feature4.init))
	  __zinitcall_sys_feature_end = .;
	  __zinitcall_run_start = .;
	  KEEP (*(.zinitcall.run0.init))
	  KEEP (*(.zinitcall.run1.init))
	  KEEP (*(.zinitcall.run2.init))
	  KEEP (*(.zinitcall.run3.init))
	  KEEP (*(.zinitcall.run4.init))
	  __zinitcall_run_end = .;
	  __zinitcall_app_service_start = .;
	  KEEP (*(.zinitcall.app.service0.init))
	  KEEP (*(.zinitcall.app.service1.init))
	  KEEP (*(.zinitcall.app.service2.init))
	  KEEP (*(.zinitcall.app.service3.init))
	  KEEP (*(.zinitcall.app.service4.init))
	  __zinitcall_app_service_end = .;
	  __zinitcall_app_feature_start = .;
	  KEEP (*(.zinitcall.app.feature0.init))
	  KEEP (*(.zinitcall.app.feature1.init))
	  KEEP (*(.zinitcall.app.feature2.init))
	  KEEP (*(.zinitcall.app.feature3.init))
	  KEEP (*(.zinitcall.app.feature4.init))
	  __zinitcall_app_feature_end = .;
	  __zinitcall_test_start = .;
	  KEEP (*(.zinitcall.test0.init))
	  KEEP (*(.zinitcall.test1.init))
	  KEEP (*(.zinitcall.test2.init))
	  KEEP (*(.zinitcall.test3.init))
	  KEEP (*(.zinitcall.test4.init))
	  __zinitcall_test_end = .;
	  __zinitcall_exit_start = .;
	  KEEP (*(.zinitcall.exit0.init))
	  KEEP (*(.zinitcall.exit1.init))
	  KEEP (*(.zinitcall.exit2.init))
	  KEEP (*(.zinitcall.exit3.init))
	  KEEP (*(.zinitcall.exit4.init))
	  __zinitcall_exit_end = .;

    KEEP (*(.init))
    KEEP (*(.fini))

    . = ALIGN(4);
    _etext = .;        /* define a global symbols at end of code */
  } >FLASH

  /* Constant data into "FLASH" Rom type memory */
  .rodata :
  {
    . = ALIGN(4);
    *(.rodata)         /* .rodata sections (constants, strings, etc.) */
    *(.rodata*)        /* .rodata* sections (constants, strings, etc.) */
    . = ALIGN(4);
  } >FLASH

  .ARM.extab   : {
    . = ALIGN(4);
    *(.ARM.extab* .gnu.linkonce.armextab.*)
    . = ALIGN(4);
  } >FLASH

  .ARM : {
    . = ALIGN(4);
    __exidx_start = .;
    *(.ARM.exidx*)
    __exidx_end = .;
    . = ALIGN(4);
  } >FLASH

  .preinit_array     :
  {
    . = ALIGN(4);
    PROVIDE_HIDDEN (__preinit_array_start = .);
    KEEP (*(.preinit_array*))
    PROVIDE_HIDDEN (__preinit_array_end = .);
    . = ALIGN(4);
  } >FLASH

  .init_array :
  {
    . = ALIGN(4);
    PROVIDE_HIDDEN (__init_array_start = .);
    KEEP (*(SORT(.init_array.*)))
    KEEP (*(.init_array*))
    PROVIDE_HIDDEN (__init_array_end = .);
    . = ALIGN(4);
  } >FLASH

  .fini_array :
  {
    . = ALIGN(4);
    PROVIDE_HIDDEN (__fini_array_start = .);
    KEEP (*(SORT(.fini_array.*)))
    KEEP (*(.fini_array*))
    PROVIDE_HIDDEN (__fini_array_end = .);
    . = ALIGN(4);
  } >FLASH

  .hdf_drivers_entry :
  {
    _hdf_drivers_start = .;
    KEEP(*(.hdf.driver))
    _hdf_drivers_end = .;
  } >FLASH

  /* Used by the startup to initialize data */
  _sidata = LOADADDR(.data);
  __etext = .;

  /* Initialized data sections into "RAM" Ram type memory */
  .data :
  {
    . = ALIGN(4);
    __data_start__ = . ;    /* create a global symbol at data start */
    KEEP (*(.vector))  /* make the vector at the data begining, and it will meet the needs of VTOR  */
    . = ALIGN(4);
    *(.data)           /* .data sections */
    *(.data*)          /* .data* sections */
    *(.RamFunc)        /* .RamFunc sections */
    *(.RamFunc*)       /* .RamFunc* sections */
    *(RAM_CODE)        /* .RamFunc sections */
    *(.ramfunc)        /* .RamFunc sections */
    *(TINY_RAM_SPACE) 
    . = ALIGN(4);
    __data_end__ = . ;/* define a global symbol at data end */
  } >RAM AT> FLASH

  /* Uninitialized data section into "RAM" Ram type memory */
  . = ALIGN(4);
  .bss :
  {
    /* This is used by the startup in order to initialize the .bss section */ 
    __bss_start__ = .;  /* define a global symbol at bss start */
    *(.bss)
    *(.bss*)
    *(COMMON)

    . = ALIGN(4);
    __bss_end__ = .;  /* define a global symbol at bss end */
  } >RAM

   . = ALIGN(16);
  __los_heap_addr_start__ = .;
  __los_heap_addr_end__ = ORIGIN(RAM) + LENGTH(RAM) - _Min_Stack_Size - _Min_Heap_Size - 1;

  .fpb :
  {
    . = ALIGN(0x100);
    *(FPB)            
  } >FPB_RAM AT> FLASH

  .ARM.attributes 0 : { *(.ARM.attributes) }
}
