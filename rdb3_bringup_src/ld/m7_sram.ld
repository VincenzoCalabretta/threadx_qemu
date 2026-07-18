/*
 * S32G3 M7_0 SRAM link script.
 *
 * ORIGIN 0x34000000 is the Internal SRAM region of the S32G3 memory
 * map from RM attachment S32G3_memory_map.xlsx sheet
 * "Overview Memory Map 40bit" row 51: 0x34000000 - 0x353FFFFF, 20 MB.
 *
 * LENGTH 128 KB matches the Lauterbach S32G3 M7 demo makefile
 *   ~/t32/demo/arm/hardware/s32g3/s32g3-procevb3/s32g-procevb3-m7/makefile
 *   RAMSTART = 0x34000000
 *   RAMSIZE  = 0x20000
 *
 * SRAMC ECC requires the region to be initialized  before the
 * first read; the load script t32/load_m7_0.cmm runs
 * ~/t32/demo/arm/hardware/s32g3/misc/s32g3_init_sram.cmm to do that before
 * Data load.
 */

ENTRY(_reset)

MEMORY
{
    SRAM (rwx) : ORIGIN = 0x34000000, LENGTH = 128K
}

SECTIONS
{
    .vectors : {
        KEEP(*(.vectors))
    } > SRAM

    .text : {
        *(.text._reset)
        *(.text*)
        *(.rodata*)
        . = ALIGN(4);
    } > SRAM

    .data : ALIGN(4) {
        __data_start = .;
        *(.data*)
        . = ALIGN(4);
        __data_end = .;
    } > SRAM
    __data_lma = LOADADDR(.data);

    .bss (NOLOAD) : ALIGN(4) {
        __bss_start = .;
        *(.bss*)
        *(COMMON)
        . = ALIGN(4);
        __bss_end = .;
    } > SRAM

    .stack (NOLOAD) : ALIGN(8) {
        . = . + 4K;
        . = ALIGN(8);
        __stack_top = .;
    } > SRAM
}
