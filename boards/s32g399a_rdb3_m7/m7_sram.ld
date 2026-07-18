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

/*
 * Explicit PHDRS split so binutils doesn't emit a single RWX PT_LOAD
 * segment (which triggers `ld: warning: ... has a LOAD segment with RWX
 * permissions` on binutils >= 2.39). Vectors + code + rodata go into an
 * R+E segment; .data goes into an R+W segment. Both segments still live
 * inside the SRAM region declared above; PHDRS only affects the ELF
 * program-header flags a loader sees, not the physical memory layout.
 *
 * FLAGS values follow the ELF PF_* bits from the SysV ABI:
 *   PF_X = 1, PF_W = 2, PF_R = 4  → text = 5 (R+E), data = 6 (R+W).
 * NOLOAD sections (.bss, .stack) are not assigned a PHDR — they don't
 * appear in any PT_LOAD segment and the startup code zero-initializes
 * .bss at boot.
 */
PHDRS
{
    text PT_LOAD FLAGS(5);
    data PT_LOAD FLAGS(6);
}

SECTIONS
{
    .vectors : {
        KEEP(*(.vectors))
    } > SRAM :text

    .text : {
        *(.text._reset)
        *(.text*)
        *(.rodata*)
        . = ALIGN(4);
    } > SRAM :text

    .data : ALIGN(4) {
        __data_start = .;
        *(.data*)
        . = ALIGN(4);
        __data_end = .;
    } > SRAM :data
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
