/*
******************************************************************************
**
** Combined linker script:
** STM32N6570 USB CDC + Face Detection
**
******************************************************************************
*/

/* Entry Point */
ENTRY(Reset_Handler)

/* Heap / Stack */
_Min_Heap_Size  = 0x800;
_Min_Stack_Size = 0x4000;

/*
 * AXISRAM1:
 *   Application code / rodata
 *
 * AXISRAM2:
 *   Normal initialized/uninitialized data
 *
 * USB_RAM:
 *   Dedicated non-cacheable / USBX buffers
 *
 * FLASH:
 *   Neural-network flash blobs
 *
 * PSRAM:
 *   Large frame buffers
 */

MEMORY
{
  AXISRAM1_S (xrw) : ORIGIN = 0x34000400, LENGTH = 1023K /*Last byte location 0x340FFFFF */

  RAM      (xrw) : ORIGIN = 0x34100000, LENGTH = 936K
  ETH_RAM  (xrw) : ORIGIN = 0x341EA000, LENGTH = 56K
  USB_RAM  (xrw) : ORIGIN = 0x341F8000, LENGTH = 32K

  FLASH      (xrw) : ORIGIN = 0x71380000, LENGTH = 2M
  PSRAM      (xrw) : ORIGIN = 0x91000000, LENGTH = 16M
}

/* Stack at top of normal AXISRAM2 area, before USB reserved region */
_estack = ORIGIN(RAM) + LENGTH(RAM);
_sstack = _estack - _Min_Stack_Size;

SECTIONS
{
  /* Vector table */
  .isr_vector :
  {
    . = ALIGN(4);
    KEEP(*(.isr_vector))
    . = ALIGN(4);
  } >AXISRAM1_S

  /* Program code */
  .text :
  {
    . = ALIGN(4);

    *(.text)
    *(.text*)

    *(.glue_7)
    *(.glue_7t)

    *(.eh_frame)

    *(.RamFunc)
    *(.RamFunc*)

    KEEP(*(.init))
    KEEP(*(.fini))
    KEEP(*(.keep_me))

    . = ALIGN(4);

    _etext = .;
  } >AXISRAM1_S

  /* Secure Gateway */
  .gnu.sgstubs :
  {
    . = ALIGN(4);
    *(.gnu.sgstubs*)
    . = ALIGN(4);
  } >AXISRAM1_S

  /* Constants */
  .rodata :
  {
    . = ALIGN(4);

    *(.rodata)
    *(.rodata*)

    . = ALIGN(4);
  } >AXISRAM1_S

  .ARM.extab (READONLY) :
  {
    . = ALIGN(4);

    *(.ARM.extab* .gnu.linkonce.armextab.*)

    . = ALIGN(4);
  } >AXISRAM1_S

  .ARM (READONLY) :
  {
    . = ALIGN(4);

    __exidx_start = .;

    *(.ARM.exidx*)

    __exidx_end = .;

    . = ALIGN(4);
  } >AXISRAM1_S

  .preinit_array (READONLY) :
  {
    . = ALIGN(4);

    PROVIDE_HIDDEN(__preinit_array_start = .);

    KEEP(*(.preinit_array*))

    PROVIDE_HIDDEN(__preinit_array_end = .);

    . = ALIGN(4);
  } >AXISRAM1_S

  .init_array (READONLY) :
  {
    . = ALIGN(4);

    PROVIDE_HIDDEN(__init_array_start = .);

    KEEP(*(SORT(.init_array.*)))
    KEEP(*(.init_array*))

    PROVIDE_HIDDEN(__init_array_end = .);

    . = ALIGN(4);
  } >AXISRAM1_S

  .fini_array (READONLY) :
  {
    . = ALIGN(4);

    PROVIDE_HIDDEN(__fini_array_start = .);

    KEEP(*(SORT(.fini_array.*)))
    KEEP(*(.fini_array*))

    PROVIDE_HIDDEN(__fini_array_end = .);

    . = ALIGN(4);
  } >AXISRAM1_S

  /*
   * Initial values for .data live in AXISRAM1 image,
   * startup copies them into AXISRAM2 RAM.
   */
  _sidata = LOADADDR(.data);

  .data :
  {
    . = ALIGN(4);

    _sdata = .;

    *(.data)
    *(.data*)

    . = ALIGN(4);

    _edata = .;
  } >RAM AT>AXISRAM1_S

  /* Normal BSS */
  .bss :
  {
    . = ALIGN(4);

    _sbss = .;
    __bss_start__ = _sbss;

    *(.bss)
    *(.bss*)
    *(COMMON)

    . = ALIGN(4);

    _ebss = .;
    __bss_end__ = _ebss;
  } >RAM

  /* Existing USB non-cacheable data */
  .noncacheable :
  {
    . = ALIGN(8);

    __snoncacheable = .;

    KEEP(*(.noncacheable))

    . = ALIGN(8);

    __enoncacheable = .;
  } >RAM

  /* Heap + Stack */
  ._user_heap_stack :
  {
    . = ALIGN(8);

    PROVIDE(end = .);
    PROVIDE(_end = .);

    . = . + _Min_Heap_Size;
    . = . + _Min_Stack_Size;

    . = ALIGN(8);
  } >RAM

  /* ETHERNET Dedicated Memory */
  .tcp_sec (NOLOAD) :
  {
    . = ABSOLUTE(0x341EAE80);
    KEEP(*(.RxDecripSection)) /*192 byte */

    . = ABSOLUTE(0x341EAF40); 
    KEEP(*(.TxDecripSection)) /*192 byte */
  } >ETH_RAM

  .nx_data (NOLOAD) :
  {
    . = ABSOLUTE(0x341EB000);
    KEEP(*(.NetXPoolSection)) /*52kbyte */
  } >ETH_RAM


  /*
   * USB dedicated memory.
   * Preserve original fixed addresses.
   */
  .usbx_data 0x341F8000 (NOLOAD) :
  {
    . = ALIGN(8);

    KEEP(*(.UsbHpcdSection))

    . = ALIGN(8);
  } >USB_RAM

  .uart_bss 0x341F9000 (NOLOAD) :
  {
    . = ALIGN(8);

    KEEP(*(.UsbxAppSection))

    . = ALIGN(8);
  } >USB_RAM

  .usbx_bss 0x341FA000 (NOLOAD) :
  {
    . = ALIGN(8);

    KEEP(*(.UsbxPoolSection))

    . = ALIGN(8);
  } >USB_RAM

  /*
   * Face Detection / Neural Network blob.
   * Generated network code places large constants here.
   */
  .flash_section :
  {
    . = ALIGN(32);

    *(.flash_blob)

    . = ALIGN(32);
  } >FLASH

  /*
   * Large frame buffers / LCD buffers.
   * NOLOAD because PSRAM is initialized at runtime.
   */
  .psram_section (NOLOAD) :
  {
    . = ALIGN(32);

    *(.psram_bss)

    . = ALIGN(32);
  } >PSRAM

  /DISCARD/ :
  {
    libc.a(*)
    libm.a(*)
    libgcc.a(*)
  }

  .ARM.attributes 0 :
  {
    *(.ARM.attributes)
  }
}