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

#include <arch/system.h>
#include <time.h>
#include <kernel/sched.h>
#include <kernel/sched_internals.h>
#include "sched.h"

/* System tick interrupt handler */
void systick_handler(void) {
    system_ticks++;

    /* Call PendSV to do switching */
    *SCB_ICSR |= SCB_ICSR_PENDSVSET;
}

/* PendSV interrupt handler */
void pendsv_handler(void){
    /* Update periodic tasks */
    rtos_tick();

    /* Run the scheduler */
    task_switch(NULL);
}

uint32_t *get_user_stack_pointer(void) {
    return PSP();
}

void set_user_stack_pointer(uint32_t *stack_addr) {
    SET_PSP(stack_addr);
}

void create_context(task_ctrl* task, void (*lptr)(void)) {
    /* AAPCS requires an 8-byte aligned SP */
    uintptr_t stack = (uintptr_t) task->stack_top;
    stack -= stack % 8;
    task->stack_top = (uint32_t *) stack;

    asm volatile(
#ifdef CONFIG_HAVE_FPU
                 "stmdb   %[stack]!, {%[zero]}  /* Empty for 8byte aligned SP */\n\
                  stmdb   %[stack]!, {%[zero]}  /* FPSCR */                     \n\
                  stmdb   %[stack]!, {%[zero]}  /* S15 */                       \n\
                  stmdb   %[stack]!, {%[zero]}  /* S14 */                       \n\
                  stmdb   %[stack]!, {%[zero]}  /* S13 */                       \n\
                  stmdb   %[stack]!, {%[zero]}  /* S12 */                       \n\
                  stmdb   %[stack]!, {%[zero]}  /* S11 */                       \n\
                  stmdb   %[stack]!, {%[zero]}  /* S10 */                       \n\
                  stmdb   %[stack]!, {%[zero]}  /* S9 */                        \n\
                  stmdb   %[stack]!, {%[zero]}  /* S8 */                        \n\
                  stmdb   %[stack]!, {%[zero]}  /* S7 */                        \n\
                  stmdb   %[stack]!, {%[zero]}  /* S6 */                        \n\
                  stmdb   %[stack]!, {%[zero]}  /* S5 */                        \n\
                  stmdb   %[stack]!, {%[zero]}  /* S4 */                        \n\
                  stmdb   %[stack]!, {%[zero]}  /* S3 */                        \n\
                  stmdb   %[stack]!, {%[zero]}  /* S2 */                        \n\
                  stmdb   %[stack]!, {%[zero]}  /* S1 */                        \n\
                  stmdb   %[stack]!, {%[zero]}  /* S0 */                        \n"
#endif
                 "stmdb   %[stack]!, {%[psr]}   /* xPSR */                      \n\
                  stmdb   %[stack]!, {%[pc]}    /* PC */                        \n\
                  stmdb   %[stack]!, {%[lr]}    /* LR */                        \n\
                  stmdb   %[stack]!, {%[zero]}  /* R12 */                       \n\
                  stmdb   %[stack]!, {%[zero]}  /* R3 */                        \n\
                  stmdb   %[stack]!, {%[zero]}  /* R2 */                        \n\
                  stmdb   %[stack]!, {%[zero]}  /* R1 */                        \n\
                  stmdb   %[stack]!, {%[zero]}  /* R0 */                        \n\
                  stmdb   %[stack]!, {%[zero]}  /* R11 */                       \n\
                  stmdb   %[stack]!, {%[zero]}  /* R10 */                       \n\
                  stmdb   %[stack]!, {%[zero]}  /* R9 */                        \n\
                  stmdb   %[stack]!, {%[zero]}  /* R8 */                        \n\
                  stmdb   %[stack]!, {%[frame]} /* R7 - Frame Pointer*/         \n\
                  stmdb   %[stack]!, {%[zero]}  /* R6 */                        \n\
                  stmdb   %[stack]!, {%[zero]}  /* R5 */                        \n\
                  stmdb   %[stack]!, {%[zero]}  /* R4 */                        \n"
#ifdef CONFIG_HAVE_FPU
                 "stmdb   %[stack]!, {%[zero]}  /* S31 */                       \n\
                  stmdb   %[stack]!, {%[zero]}  /* S30 */                       \n\
                  stmdb   %[stack]!, {%[zero]}  /* S29 */                       \n\
                  stmdb   %[stack]!, {%[zero]}  /* S28 */                       \n\
                  stmdb   %[stack]!, {%[zero]}  /* S27 */                       \n\
                  stmdb   %[stack]!, {%[zero]}  /* S26 */                       \n\
                  stmdb   %[stack]!, {%[zero]}  /* S25 */                       \n\
                  stmdb   %[stack]!, {%[zero]}  /* S24 */                       \n\
                  stmdb   %[stack]!, {%[zero]}  /* S23 */                       \n\
                  stmdb   %[stack]!, {%[zero]}  /* S22 */                       \n\
                  stmdb   %[stack]!, {%[zero]}  /* S21 */                       \n\
                  stmdb   %[stack]!, {%[zero]}  /* S20 */                       \n\
                  stmdb   %[stack]!, {%[zero]}  /* S19 */                       \n\
                  stmdb   %[stack]!, {%[zero]}  /* S18 */                       \n\
                  stmdb   %[stack]!, {%[zero]}  /* S17 */                       \n\
                  stmdb   %[stack]!, {%[zero]}  /* S16 */"
#endif
                  /* Output */
                  :[stack] "+r" (task->stack_top)
                  /* Input */
                  :[pc] "r" (task->fptr), [lr] "r" (lptr), [frame] "r" (task->stack_limit),
                   [zero] "r" (0), [psr] "r" (0x01000000) /* Set the Thumb bit */
                  /* Clobber */
                  :);
}
