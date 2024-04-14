
#ifdef TIMER_1
/* The periods assigned to the one-shot and auto-reload timers are 3.333
   second and half a second respectively. */
#define mainONE_SHOT_TIMER_PERIOD pdMS_TO_TICKS( 3333 )
#define mainAUTO_RELOAD_TIMER_PERIOD pdMS_TO_TICKS( 500 )

int main(void)
{
    TimerHandle_t xAutoReloadTimer;
    TimerHandle_t xOneShotTimer;

    BaseType_t xTimer1Started, xTimer2Started;

    /* Create the one shot timer, storing the handle to the created timer in xOneShotTimer. */
    xOneShotTimer = xTimerCreate("OneShot",
                                 mainONE_SHOT_TIMER_PERIOD,
                                 pdFALSE,  // one-shot software timer.
                                 0,        // no timer id.
                                 prvOneShotTimerCallback); // Callback function to used by timer.

    /* Create the auto-reload timer, storing the handle to the created timer in xAutoReloadTimer. */
    xAutoReloadTimer = xTimerCreate("AutoReload",
                                    mainAUTO_RELOAD_TIMER_PERIOD,
                                    pdTRUE,  // auto-reload timer
                                    0,
                                    prvAutoReloadTimerCallback);

    /* Check the software timers were created. */
    if ((xOneShotTimer != NULL) && (xAutoReloadTimer != NULL)) 
    {
        /* Start the software timers, using a block time of 0 (no block time).
           The scheduler has not been started yet so any block time specified
           here would be ignored anyway. */
        xTimer1Started = xTimerStart(xOneShotTimer, 0);
        xTimer2Started = xTimerStart(xAutoReloadTimer, 0);

        /* The implementation of xTimerStart() uses the timer command queue,
           and xTimerStart() will fail if the timer command queue gets full.
           The timer service task does not get created until the scheduler is
           started, so all commands sent to the command queue will stay in the
           queue until after the scheduler has been started. Check both calls
           to xTimerStart() passed. */
        if ((xTimer1Started == pdPASS) && (xTimer2Started == pdPASS)) {
            /* Start the scheduler. */
            vTaskStartScheduler();
        }
    }
    /* As always, this line should not be reached. */
    for (;;);
}

static void prvOneShotTimerCallback( TimerHandle_t xTimer )
{
    TickType_t xTimeNow;

    /* Obtain the current tick count. */
    xTimeNow = xTaskGetTickCount();

    /* Output a string to show the time at which the callback was executed. */
    vPrintStringAndNumber( "One-shot timer callback executing", xTimeNow );

    ulCallCount++;
}

static void prvAutoReloadTimerCallback( TimerHandle_t xTimer )
{
    TickType_t xTimeNow;
    /* Obtain the current tick count. */
    xTimeNow = xTaskGetTickCount();
    /* Output a string to show the time at which the callback was executed. */
    vPrintStringAndNumber( "Auto-reload timer callback executing", xTimeNow);
    ulCallCount++;
}

#endif /* TIMER_1 */




#ifdef SINGLE_CALLBACK

/* The periods assigned to the one-shot and auto-reload timers are 3.333
   second and half a second respectively. */
#define mainONE_SHOT_TIMER_PERIOD pdMS_TO_TICKS( 3333 )
#define mainAUTO_RELOAD_TIMER_PERIOD pdMS_TO_TICKS( 500 )

int main(void)
{
    TimerHandle_t xAutoReloadTimer;
    TimerHandle_t xOneShotTimer;

    BaseType_t xTimer1Started, xTimer2Started;

    /* Create the one shot timer software timer, storing the handle in xOneShotTimer. */
    xOneShotTimer = xTimerCreate("OneShot", mainONE_SHOT_TIMER_PERIOD,pdFALSE,NULL,prvTimerCallback); // prvTimerCallback() is used by both timers. 

    /* Create the auto-reload software timer, storing the handle in xAutoReloadTimer */
    xAutoReloadTimer = xTimerCreate("AutoReload", mainAUTO_RELOAD_TIMER_PERIOD,pdTRUE,NULL,prvTimerCallback); // prvTimerCallback() is used by both timers. 


    /* Check the software timers were created. */
    if ((xOneShotTimer != NULL) && (xAutoReloadTimer != NULL)) 
    {
        /* Start the software timers, using a block time of 0 (no block time).
           The scheduler has not been started yet so any block time specified
           here would be ignored anyway. */
        xTimer1Started = xTimerStart(xOneShotTimer, 0);
        xTimer2Started = xTimerStart(xAutoReloadTimer, 0);

        /* The implementation of xTimerStart() uses the timer command queue,
           and xTimerStart() will fail if the timer command queue gets full.
           The timer service task does not get created until the scheduler is
           started, so all commands sent to the command queue will stay in the
           queue until after the scheduler has been started. Check both calls
           to xTimerStart() passed. */
        if ((xTimer1Started == pdPASS) && (xTimer2Started == pdPASS)) {
            /* Start the scheduler. */
            vTaskStartScheduler();
        }
    }
    /* As always, this line should not be reached. */
    for (;;);
}

static void prvTimerCallback(TimerHandle_t xTimer) 
{
    TickType_t xTimeNow;
    uint32_t ulExecutionCount;

    /* A count of the number of times this software timer has expired is
       stored in the timer's ID. Obtain the ID, increment it, then save it as
       the new ID value. The ID is a void pointer, so is cast to a uint32_t. */
    ulExecutionCount = (uint32_t) pvTimerGetTimerID(xTimer);

    ulExecutionCount++;

    vTimerSetTimerID(xTimer, (void *) ulExecutionCount);

    /* Obtain the current tick count. */
    xTimeNow = xTaskGetTickCount();

    /* The handle of the one-shot timer was stored in xOneShotTimer when the
       timer was created. Compare the handle passed into this function with
       xOneShotTimer to determine if it was the one-shot or auto-reload timer
       that expired, then output a string to show the time at which the
       callback was executed. */
    if (xTimer == xOneShotTimer) {
        vPrintStringAndNumber("One-shot timer callback executing", xTimeNow);
    } else {
        /* xTimer did not equal xOneShotTimer, so it must have been the
           auto-reload timer that expired. */
        vPrintStringAndNumber("Auto-reload timer callback executing", xTimeNow);
        if (ulExecutionCount == 5) {
            /* Stop the auto-reload timer after it has executed 5 times. This
               callback function executes in the context of the RTOS daemon
               task so must not call any functions that might place the daemon
               task into the Blocked state. Therefore a block time of 0 is
               used. */
            xTimerStop(xTimer, 0);
        }
    }
}
#endif /* SINGLE_CALLBACK */


#ifdef CHANGE_PERIOD

/*  The check timer is created with a period of 3000 milliseconds, resulting
    in the LED toggling every 3 seconds. If the self-checking functionality
    detects an unexpected state, then the check timer's period is changed to
    just 200 milliseconds, resulting in a much faster toggle rate. */

const TickType_t xHealthyTimerPeriod = pdMS_TO_TICKS( 3000 );
const TickType_t xErrorTimerPeriod   = pdMS_TO_TICKS( 200 );

/* The callback function used by the check timer. */
static void prvCheckTimerCallbackFunction(TimerHandle_t xTimer) 
{
    static BaseType_t xErrorDetected = pdFALSE;

    if (xErrorDetected == pdFALSE) 
    {
        /*  No errors have yet been detected. Run the self-checking function
            again. The function asks each task created by the example to report
            its own status, and also checks that all the tasks are actually
            still running (and so able to report their status correctly). */
        if (CheckTasksAreRunningWithoutError() == pdFAIL) 
        {
            /*  One or more tasks reported an unexpected status. An error might
                have occurred. Reduce the check timer's period to increase the
                rate at which this callback function executes, and in so doing
                also increase the rate at which the LED is toggled. This
                callback function is executing in the context of the RTOS daemon
                task, so a block time of 0 is used to ensure the Daemon task
                never enters the Blocked state. */
            xTimerChangePeriod(
                xTimer,            /* The timer being updated */
                xErrorTimerPeriod, /* The new period for the timer */
                0);                /* Do not block when sending this command */
        }
        /* Latch that an error has already been detected. */
        xErrorDetected = pdTRUE;
    }
    /*  Toggle the LED. The rate at which the LED toggles will depend on how
        often this function is called, which is determined by the period of the
        check timer. The timer's period will have been reduced from 3000ms to
        just 200ms if CheckTasksAreRunningWithoutError() has ever returned
        pdFAIL. */
    ToggleLED();
}
#endif /* CHANGE_PERIOD */

#define RESET
#ifdef RESET
/*
    This example simulates the behavior of the backlight on a cell phone. The backlight:
        - Turns on when a key is pressed.
        - Remains on provided further keys are pressed within a certain time period.
        - Automatically turns off if no key presses are made within a certain time period
 */

static void prvBacklightTimerCallback(TimerHandle_t xTimer) 
{
    TickType_t xTimeNow = xTaskGetTickCount();
    /* The backlight timer expired, turn the backlight off. */
    xSimulatedBacklightOn = pdFALSE;
    /* Print the time at which the backlight was turned off. */
    vPrintStringAndNumber("Timer expired, turning backlight OFF at time\t\t", xTimeNow);
}

static void vKeyHitTask(void *pvParameters) 
{
    const TickType_t xShortDelay = pdMS_TO_TICKS(50);

    TickType_t xTimeNow;

    vPrintString("Press a key to turn the backlight on.\r\n");
    /* Ideally an application would be event driven, and use an interrupt to
       process key presses. It is not practical to use keyboard interrupts
       when using the FreeRTOS Windows port, so this task is used to poll for
       a key press. */
    for (;;) 
    {
        /* Has a key been pressed? */
        if (_kbhit() != 0) 
        {
            /* A key has been pressed. Record the time. */
            xTimeNow = xTaskGetTickCount();
            if (xSimulatedBacklightOn == pdFALSE) 
            {
                /*  The backlight was off, so turn it on and print the time at
                    which it was turned on. */
                xSimulatedBacklightOn = pdTRUE;
                vPrintStringAndNumber("Key pressed, turning backlight ON at time\t\t", xTimeNow);
            } else {
                /* The backlight was already on, so print a message to say the
                timer is about to be reset and the time at which it was
                reset. */
                vPrintStringAndNumber("Key pressed, resetting software timer at time\t\t",xTimeNow);
            }
            /*   Reset the software timer. If the backlight was previously off,
                then this call will start the timer. If the backlight was
                previously on, then this call will restart the timer. A real
                application may read key presses in an interrupt. If this
                function was an interrupt service routine then
                xTimerResetFromISR() must be used instead of xTimerReset(). */
            xTimerReset(xBacklightTimer, xShortDelay);
            /* Read and discard the key that was pressed – it is not required
               by this simple example. */
            (void) _getch();
        }
    }
}
#endif /* RESET */