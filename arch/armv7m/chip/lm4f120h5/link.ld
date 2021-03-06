/*
 * Copyright (C) 2013 F4OS Authors
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
 * of the Software, and to permit persons to whom the Software is furnished to do
 * so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

/* Specify the memory areas */
MEMORY
{
  FLASH (rx)      : ORIGIN = 0x00000000, LENGTH = 256K
  RAM (xrw)       : ORIGIN = 0x20000000, LENGTH = 32K
}

/* Highest address of the kernel stack */
_estack = 0x20001FFC;       /* quarter of 32K SRAM */

/* Heap */
_suserheap = CONFIG_SUSERHEAP;
_euserheap = CONFIG_EUSERHEAP;
_skernelheap = CONFIG_SKERNELHEAP;
_ekernelheap = CONFIG_EKERNELHEAP;

SECTIONS {
    /* Vectors first - Always permitted by MPU */
    .isr_vector :
    {
        . = ALIGN(4);
        KEEP(*(.isr_vector))
        . = ALIGN(4);
    } > FLASH

    .kernel :
    {
        . = ALIGN(8);
        _skernel = .;
        *(.kernel*)
        . = ALIGN(8);
        _ekernel = .;
    } > FLASH

    .initializers :
    {
        . = ALIGN(4);
        __begin_early_initializer = .;
        KEEP(*(.initializers.early))
        __end_early_initializer = .;
        . = ALIGN(4);                   /* Although function pointers should always be */
        __begin_core_initializer = .;   /* word aligned, someone could erroneously add */
        KEEP(*(.initializers.core))     /* something not word sized to .initcall.* */
        __end_core_initializer = .;     /* and these otherwise useless aligns would save */
        . = ALIGN(4);                   /* you from a fault! */
        __begin_late_initializer = .;
        KEEP(*(.initializers.late))
        __end_late_initializer = .;
        . = ALIGN(4);
    } > FLASH

    .text :
    {
        . = ALIGN(8);
        _stext = .;
        *(.text*)
        . = ALIGN(8);

        _etext = .;
    } > FLASH

    .rodata :
    {
        . = ALIGN(8);
        *(.rodata*)
        . = ALIGN(8);
    } > FLASH

    /* Special user section reserved only for general
     * use by user programs. */
    .user :
    {
        _user_start = .;
        *(.user*)
        _user_end = .;
        . = ALIGN(8);
    } > FLASH

    .dtb :
    {
        . = ALIGN(8);
        _dtb_start = .;
        *(.dtb*)
        _dtb_end = .;
        . = ALIGN(8);
    } > FLASH

    _sidata = .;
    .data : AT ( _sidata )
    {
        . = ALIGN(4);
        _sdata = .;        /* create a global symbol at data start */
        *(.data*)           /* .data sections */

        . = ALIGN(4);
        _edata = .;        /* define a global symbol at data end */
    } > RAM

    /* Uninitialized data section */
    _sibss = .;
    .bss : AT ( _sibss )
    {
        . = ALIGN(4);
        _sbss = .;         /* define a global symbol at bss start */
        *(.bss*)
        *(COMMON)

        . = ALIGN(4);
        _ebss = .;         /* define a global symbol at bss end */
    } > RAM
}

PROVIDE(end = .);
_end            = .;
