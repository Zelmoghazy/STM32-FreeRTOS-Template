void vATaskFunction(void *pvParameters) {
    /*
     * Stack-allocated variables can be declared normally when inside a
     * function. Each instance of a task created using this example function
     * will have its own separate instance of lStackVariable allocated on the
     * task's stack.
     */
    long lStackVariable = 0;
    /*
     * In contrast to stack allocated variables, variables declared with the
     * `static` keyword are allocated to a specific location in memory by the
     * linker. This means that all tasks calling vATaskFunction will share the
     * same instance of lStaticVariable.
     */
    static long lStaticVariable = 0;

    for (;;) {
        /* The code to implement the task functionality will go here. */
    }
    /*
     * If the task implementation ever exits the above loop, then the task
     * must be deleted before reaching the end of its implementing function.
     * When NULL is passed as a parameter to the vTaskDelete() API function,
     * this indicates that the task to be deleted is the calling (this) task.
     */
    vTaskDelete(NULL);

    /* Note: If a task uses the vTaskDelete() API function to delete itself 
     * then it is essential that the Idle task is not starved of processing time.
     * This is because the Idle task is responsible for cleaning up kernel resources
     * used by tasks that deleted themselves.
     */
}

void vTask1(void *pvParameters) {
    /* ulCount is declared volatile to ensure it is not optimized out. */
    volatile unsigned long ulCount;

    for (;;) {
        /* Print out the name of the current task task. */
        vPrintLine("Task 1 is running");
        /* Delay for a period. */
        for (ulCount = 0; ulCount < mainDELAY_LOOP_COUNT; ulCount++) {
            /*
             * This loop is just a very crude delay implementation. There is
             * nothing to do in here. Later examples will replace this crude
             * loop with a proper delay/sleep function.
             */
        }
    }
}

void vTask2(void *pvParameters) {
    /* ulCount is declared volatile to ensure it is not optimized out. */
    volatile unsigned long ulCount;
    /* As per most tasks, this task is implemented in an infinite loop. */
    for (;;) {
        /* Print out the name of this task. */
        vPrintLine("Task 2 is running");
        /* Delay for a period. */
        for (ulCount = 0; ulCount < mainDELAY_LOOP_COUNT; ulCount++) {
            /*
             * This loop is just a very crude delay implementation. There is
             * nothing to do in here. Later examples will replace this crude
             * loop with a proper delay/sleep function.
             */
        }
    }
}
int main(void) {
    /*
     * Variables declared here may no longer exist after starting the FreeRTOS
     * scheduler. Do not attempt to access variables declared on the stack used
     * by main() from tasks.
     */

    /*
     * Create one of the two tasks. Note that a real application should check
     * the return value of the xTaskCreate() call to ensure the task was
     * created successfully.
     */
    xTaskCreate(vTask1,   /* Pointer to the function that implements the task.*/
                "Task 1", /* Text name for the task. */
                1000,     /* Stack depth in words. */
                NULL,     /* This example does not use the task parameter. */
                1,        /* This task will run at priority 1. */
                NULL);    /* This example does not use the task handle. A task handle is only required 
                           * when a task wishes to reference a task other than itself */
    /* Create the other task in exactly the same way and at the same priority.*/
    xTaskCreate(vTask2, "Task 2", 1000, NULL, 1, NULL);

    /* Start the scheduler so the tasks start executing. */
    vTaskStartScheduler();
    /*
     * If all is well main() will not reach here because the scheduler will now
     * be running the created tasks. If main() does reach here then there was
     * not enough heap memory to create either the idle or timer tasks
     * (described later in this book). Chapter 3 provides more information on
     * heap memory management.
     */
    for (;;)
        ;
}



void vTaskFunction(void *pvParameters) {
    char *pcTaskName;

    volatile unsigned long ul; /* volatile to ensure ul is not optimized away. */
    /*
     * The string to print out is passed in via the parameter. Cast this to a
     * character pointer.
     */
    pcTaskName = (char *) pvParameters;

    /* As per most tasks, this task is implemented in an infinite loop. */
    for (;;) 
    {
        /* Print out the name of this task. */
        vPrintLine(pcTaskName);
        /* Delay for a period. */
        for (ul = 0; ul < mainDELAY_LOOP_COUNT; ul++) {
            /*
             * This loop is just a very crude delay implementation. There is
             * nothing to do in here. Later exercises will replace this crude
             * loop with a proper delay/sleep function
             */
        }
    }
}

/*
 * Define the strings that will be passed in as the task parameters. These are
 * defined const and not on the stack used by main() to ensure they remain
 * valid when the tasks are executing.
 */
static const char *pcTextForTask1 = "Task 1 is running";
static const char *pcTextForTask2 = "Task 2 is running";

int main(void) 
{
    /*
     * Variables declared here may no longer exist after starting the FreeRTOS
     * scheduler. Do not attempt to access variables declared on the stack used
     * by main() from tasks.
     */

    /* Create one of the two tasks. */
    xTaskCreate(vTaskFunction,           /* Pointer to the function thatimplements the task. */
                "Task 1",                /* Text name for the task. This is to facilitate debugging only. */
                1000,                    /* Stack depth - small microcontrollers will use much less stack than this.*/
                (void *) pcTextForTask1, /* Pass the text to be printed into the task using the task parameter. */
                1,                       /* This task will run at priority 1. */
                NULL);                   /* The task handle is not used in this example. */
    /*
     * Create the other task in exactly the same way. Note this time that
     * multiple tasks are being created from the SAME task implementation
     * (vTaskFunction). Only the value passed in the parameter is different.
     * Two instances of the same task definition are being created.
     */
    xTaskCreate(vTaskFunction, 
                "Task 2",
                1000,
                (void *) pcTextForTask2,
                1,
                NULL);

    
    /* There must always be at least one task that can enter the Running state.
     * To ensure this is the case, the scheduler automatically creates an Idle task
     * when vTaskStartScheduler() is called.
     */

    /* Start the scheduler so the tasks start executing. */
    vTaskStartScheduler();
    /*
     * If all is well main() will not reach here because the scheduler will
     * now be running the created tasks. If main() does reach here then there
     * was not enough heap memory to create either the idle or timer tasks
     * (described later in this book). Chapter 3 provides more information on
     * heap memory management.
     */
    for (;;) {
    }
}


void vTaskFunction(void *pvParameters) 
{
    char *pcTaskName;
    /*
    * pdMS_TO_TICKS() macro converts a time specified in milliseconds into a time specified in ticks
    * Using pdMS_TO_TICKS() to specify times in milliseconds, rather than directly as ticks, ensures times specified within the application do not change if the tick frequency is changed.
    * pdMS_TO_TICKS() takes a time in milliseconds as its only parameter,
    * and evaluates to the equivalent time in tick periods. This example shows
    * xTimeInTicks being set to the number of tick periods that are equivalent to 250 milliseconds.
    */
    const TickType_t xDelay250ms = pdMS_TO_TICKS(250);
    /*
     * The string to print out is passed in via the parameter. Cast this to a
     * character pointer.
     */
    pcTaskName = (char *) pvParameters;
    /* As per most tasks, this task is implemented in an infinite loop. */
    for (;;) {
        /* Print out the name of this task. */
        vPrintLine(pcTaskName);
        /*
         * Delay for a period. This time a call to vTaskDelay() is used which
         * places the task into the Blocked state until the delay period has
         * expired. The parameter takes a time specified in 'ticks', and the
         * pdMS_TO_TICKS() macro is used (where the xDelay250ms constant is
         * declared) to convert 250 milliseconds into an equivalent time in
         * ticks.
         */
        vTaskDelay(xDelay250ms);
    }
}

void vTaskFunction(void *pvParameters) 
{
    char *pcTaskName;
    TickType_t xLastWakeTime;
    /*
     * The string to print out is passed in via the parameter. Cast this to a
     * character pointer.
     */
    pcTaskName = (char *) pvParameters;
    /*
     * The xLastWakeTime variable needs to be initialized with the current tick
     * count. Note that this is the only time the variable is written to
     * explicitly. After this xLastWakeTime is automatically updated within
     * vTaskDelayUntil().
     */
    xLastWakeTime = xTaskGetTickCount();
    /* As per most tasks, this task is implemented in an infinite loop. */
    for (;;) 
    {
        /* Print out the name of this task. */
        vPrintLine(pcTaskName);
        /*
         * vTaskDelayUntil() is the API function to use when a fixed execution period is required
         * (where you want your task to execute periodically with a fixed frequency),
         * as the time at which the calling task is unblocked is absolute, rather than relative to when the function
         * was called (as is the case with vTaskDelay()). 
        */

        /*
         * This task should execute every 250 milliseconds exactly. As per
         * the vTaskDelay() function, time is measured in ticks, and the
         * pdMS_TO_TICKS() macro is used to convert milliseconds into ticks.
         * xLastWakeTime is automatically updated within vTaskDelayUntil(), so
         * is not explicitly updated by the task.
         */
        vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(250));
    }
}


#ifdef CHANGING_TASK_PRIORITY

void vTask1(void *pvParameters) 
{
    UBaseType_t uxPriority;
    /*
     * This task will always run before Task 2 as it is created with the higher
     * priority. Neither Task 1 nor Task 2 ever block so both will always be in
     * either the Running or the Ready state.
     */
    /*
    * Query the priority at which this task is running - passing in NULL means
    * "return the calling task's priority".
     */
    uxPriority = uxTaskPriorityGet(NULL);

    for (;;) {
        /* Print out the name of this task. */
        vPrintLine("Task 1 is running");
        /*
         * Setting the Task 2 priority above the Task 1 priority will cause
         * Task 2 to immediately start running (as then Task 2 will have the
         * higher priority of the two created tasks). Note the use of the
         * handle to task 2 (xTask2Handle) in the call to vTaskPrioritySet().
         * Listing 4.25 shows how the handle was obtained.
         */
        vPrintLine("About to raise the Task 2 priority");
        vTaskPrioritySet(xTask2Handle, (uxPriority + 1));
        /*
         * Task 1 will only run when it has a priority higher than Task 2.
         * Therefore, for this task to reach this point, Task 2 must already
         * have executed and set its priority back down to below the priority
         * of this task.
         */
    }
}
void vTask2(void *pvParameters) 
{
    UBaseType_t uxPriority;
    /*
     * Task 1 will always run before this task as Task 1 is created with the
     * higher priority. Neither Task 1 nor Task 2 ever block so will always be
     * in either the Running or the Ready state.
     *
     * Query the priority at which this task is running - passing in NULL means
     * "return the calling task's priority".
     */
    uxPriority = uxTaskPriorityGet(NULL);
    for (;;) {
        /*
        * For this task to reach this point Task 1 must have already run and
        * set the priority of this task higher than its own.
        Mastering the FreeRTOS Real Time Kernel
        68
         */
        /* Print out the name of this task. */
        vPrintLine("Task 2 is running");
        /*
         * Set the priority of this task back down to its original value.
         * Passing in NULL as the task handle means "change the priority of the
         * calling task". Setting the priority below that of Task 1 will cause
         * Task 1 to immediately start running again – preempting this task.
         */
        vPrintLine("About to lower the Task 2 priority");
        vTaskPrioritySet(NULL, (uxPriority - 2));
    }
}

/* Declare a variable that is used to hold the handle of Task 2. */
TaskHandle_t xTask2Handle = NULL;

int main(void) {
    /*
     * Create the first task at priority 2. The task parameter is not used
     * and set to NULL. The task handle is also not used so is also set to
     * NULL.
     */
    xTaskCreate(vTask1, "Task 1", 1000, NULL, 2, NULL);
    /* The task is created at priority 2 ______^. */

    /*
     * Create the second task at priority 1 - which is lower than the priority
     * given to Task 1. Again the task parameter is not used so is set to NULL-
     * BUT this time the task handle is required so the address of xTask2Handle
     * is passed in the last parameter.
     */
    xTaskCreate(vTask2, "Task 2", 1000, NULL, 1, &xTask2Handle);
    /* The task handle is the last parameter _____^^^^^^^^^^^^^ */
    /* Start the scheduler so the tasks start executing. */
    vTaskStartScheduler();

    /*
        * If all is well main() will not reach here because the scheduler will
        * now be running the created tasks. If main() does reach here then
        * there was not enough heap memory to create either the idle or timer
        * tasks (described later in this book). Chapter 2 provides more
        * information on heap memory management.
        */
    for (;;) {

    }
}
#endif

#ifdef DELETE_TASK
TaskHandle_t xTask2Handle = NULL;

void vTask1(void *pvParameters) {
    const TickType_t xDelay100ms = pdMS_TO_TICKS(100UL);

    for (;;) {
        /* Print out the name of this task. */
        vPrintLine("Task 1 is running");
        /*
         * Create task 2 at a higher priority.
         * Pass the address of xTask2Handle as the pxCreatedTask parameter so
         * that xTaskCreate write the resulting task handle to that variable.
         */
        xTaskCreate(vTask2, "Task 2", 1000, NULL, 2, &xTask2Handle);
        /*
         * Task 2 has/had the higher priority. For Task 1 to reach here, Task 2
         * must have already executed and deleted itself.
         */
        vTaskDelay(xDelay100ms);
    }
}

void vTask2(void *pvParameters) {
    /*
     * Task 2 immediately deletes itself upon starting.
     * To do this it could call vTaskDelete() using NULL as the parameter.
     * For demonstration purposes, it instead calls vTaskDelete() with its own
     * task handle.
     */
    vPrintLine("Task 2 is running and about to delete itself");

    /* If a task that was created using dynamic memory allocation later deletes itself,
     * the Idle task is responsible for freeing the memory allocated for use, 
     * such as the deleted task's data structure and stack.
     *  So it is important that applications do not completely starve the 
     * Idle task of all processing time when this is the case 
     */
    vTaskDelete(xTask2Handle);
}

int main(void) {
    /* Create the first task at priority 1. */
    xTaskCreate(vTask1, "Task 1", 1000, NULL, 1, NULL);
    /* Start the scheduler so the task starts executing. */
    vTaskStartScheduler();
    /* main() should never reach here as the scheduler has been started. */
    for (;;) {
    }
}

#endif /* DELETE_TASK */

