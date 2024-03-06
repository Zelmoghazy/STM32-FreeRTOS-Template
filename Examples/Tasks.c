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
                NULL);    /* This example does not use the task handle. */
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