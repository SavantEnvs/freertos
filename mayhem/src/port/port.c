/*
 * FreeRTOS Kernel <DEVELOPMENT BRANCH>
 * license and copyright intentionally withheld to promote copying into user code.
 */

#include "FreeRTOS.h"
#include "task.h"

#define _GNU_SOURCE
#include <stdlib.h>
#include <pthread.h>
#include <err.h>
#include <sched.h>

typedef struct mayhem_task {
    TaskFunction_t func;
    void *args;
    pthread_t tid;
} mayhem_task_t;

#define MAX_NUMBER_OF_TASKS 512

mayhem_task_t tasks[MAX_NUMBER_OF_TASKS];
size_t total_tasks = 0;


void *run_task(void *arg) {
    printf("starting a task\n");
    mayhem_task_t *self = arg;
    self->func(self->args);
    printf("a task finished\n");
    exit(0);
    return NULL;
}

BaseType_t xPortStartScheduler( void )
{
    for (int i = 0; i < total_tasks; i++) {
        run_task(&tasks[i]);
        /* if (pthread_create(&tasks[i].tid, NULL, run_task, &tasks[i]) != 0) { */
        /*     err(1, "failed to create task"); */
        /* }; */
    }

    return pdTRUE;
}

void vPortEndScheduler( void ) {
    for (int i = 0; i < total_tasks; i++) {
        pthread_join(tasks[i].tid, NULL);

    }
}

StackType_t * pxPortInitialiseStack( StackType_t * pxTopOfStack,
                                     TaskFunction_t pxCode,
                                     void * pvParameters ) {
    if (total_tasks < MAX_NUMBER_OF_TASKS) {
        mayhem_task_t t = {.func=pxCode, .args=pvParameters};
        tasks[total_tasks++] = t;
    }
    return pxTopOfStack;
}

void vPortYield(void) {
}
