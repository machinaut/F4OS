#include <stddef.h>

#include <kernel/semaphore.h>
#include <kernel/sched.h>
#include <kernel/fault.h>
#include "sched_internals.h"

struct list free_task_list = INIT_LIST(free_task_list);

void kernel_task(void) {
    /* Does cleanup that can't be done from outside a task (ie. in an interrupt) */

    while (!list_empty(&free_task_list)) {
        struct list *element = list_pop(&free_task_list);
        struct task_ctrl *task = list_entry(element, struct task_ctrl, free_task_list);

        /* Free abandoned semaphores */
        for (int i = 0; i < HELD_SEMAPHORES_MAX; i++) {
            if (task->held_semaphores[i]) {
                release(task->held_semaphores[i]);
            }
        }

        free_task(task);
    }
}

void sleep_task(void) {
    /* Run when there is nothing else to run */
    while (1) {
        asm("wfe\n");
    }
}
