/*
 * Linker script for QEMU mps2-an500 (Cortex-M7).
 *
 * Memory map from QEMU hw/arm/mps2.c for FPGA_AN500:
 *   SSRAM1  0x00000000 (4 MB)   Zero-wait-state boot RAM, code + rodata + vectors
 *   SSRAM2  0x20000000 (4 MB)   Data + bss + stack
 *
 * The 4 MB code region is more than we need. .vectors lands at 0x00000000
 * so QEMU's `-kernel` loader picks up the initial SP and reset PC in the
 * first 8 bytes.
 */

ENTRY(_reset)

MEMORY
{
    FLASH (rx)  : ORIGIN = 0x00000000, LENGTH = 4M
    RAM   (rwx) : ORIGIN = 0x20000000, LENGTH = 4M
}

SECTIONS
{
    .vectors : {
        KEEP(*(.vectors))
    } > FLASH

    .text : {
        *(.text._reset)
        *(.text*)
        *(.rodata*)
        . = ALIGN(4);
    } > FLASH

    .data : ALIGN(4) {
        __data_start = .;
        *(.data*)
        . = ALIGN(4);
        __data_end = .;
    } > RAM AT > FLASH
    __data_lma = LOADADDR(.data);

    .bss (NOLOAD) : ALIGN(4) {
        __bss_start = .;
        *(.bss*)
        *(COMMON)
        . = ALIGN(4);
        __bss_end = .;
    } > RAM

    .stack (NOLOAD) : ALIGN(8) {
        . = . + 8K;
        . = ALIGN(8);
        __stack_top = .;
    } > RAM
}
