#include "FreeRTOS.h"
#include "task.h"
#include "timers.h"
#include "queue.h"
#include "semphr.h"
#include "event_groups.h"

/*
    -   If a context switch should be performed, then the interrupt safe API function will set
        *pxHigherPriorityTaskWoken to pdTRUE. 

    -   To be able to detect this has happened, the variable pointed to by pxHigherPriorityTaskWoken 
        must be initialized to pdFALSE before it is used for the first time. 

    -   FreeRTOS API functions can only set *pxHighPriorityTaskWoken to pdTRUE.       

    -   If an ISR calls more than one FreeRTOS API function, then the same variable can be passed as the pxHigherPriorityTaskWoken
        parameter in each API function call, and the variable only needs to be initialized to pdFALSE before it is
        used for the first time. 
    
    -   If the portYIELD_FROM_ISR() xHigherPriorityTaskWoken parameter is pdFALSE (zero), then a context switch
        is not requested, and the macro has no effect. 

    -   If the portYIELD_FROM_ISR() xHigherPriorityTaskWoken parameter is not pdFALSE, then a context switch is requested, 
        and the task in the Running state might change.
        
    -   The interrupt will always return to the task in the Running state, even if the task in the Running state changed
        while the interrupt was executing.

    -   Deferring interrupt processing to a task also allows the application writer to prioritize the processing relative to
        other tasks in the application, and use all the FreeRTOS API functions.

    -   If the priority of the task to which interrupt processing is deferred is above the priority of any other task, then the
        processing will be performed immediately, just as if the processing had been performed in the ISR itself.
 */ 


 /* 
        The binary semaphore can be considered conceptually as a queue with a length of one.
        The queue can contain a maximum of one item at any time, so is always either empty or full (hence, binary).

        -   By calling xSemaphoreTake(), the task to which interrupt processing is deferred effectively attempts to read
            from the queue with a block time, causing the task to enter the Blocked state if the queue is empty.

        -   When the event occurs, the ISR uses the xSemaphoreGiveFromISR() function to place a token (the semaphore) into
            the queue, making the queue full. This causes the task to exit the Blocked state and remove the token, leaving
            the queue empty once more.

        -   When the task has completed its processing, it once more attempts to read from the queue and, finding the queue empty,
            re-enters the Blocked state to wait for the next event. 

        -   It is possible that a single semaphore will have one or more tasks blocked on it waiting for the semaphore
            to become available. 

        -   Calling xSemaphoreGiveFromISR() can make the semaphore available, and so cause a task that was waiting for 
            the semaphore to leave the Blocked state.

        -   If calling xSemaphoreGiveFromISR() causes a task to leave the Blocked state, and the unblocked task has a 
            priority higher than the currently executing task (the task that was interrupted), then, internally, 
            xSemaphoreGiveFromISR() will set *pxHigherPriorityTaskWoken to pdTRUE.

        -   If xSemaphoreGiveFromISR() sets this value to pdTRUE, then normally a context switch should be
            performed before the interrupt is exited. 

        -   This will ensure that the interrupt returns directly to the highest priority Ready state task.    

        Just as binary semaphores can be thought of as queues that have a length of one, counting semaphores can be
        thought of as queues that have a length of more than one. Tasks are not interested in the data that is stored in
        the queue—just the number of items in the queue.
  */

/* 
    The number of the software interrupt used in this example. The code
    shown is from the Windows project, where numbers 0 to 2 are used by the
    FreeRTOS Windows port itself, so 3 is the first number available to the
    application. 
*/

#ifdef DEFERRED_INT_1

#define mainINTERRUPT_NUMBER 3

static void vPeriodicTask(void *pvParameters) 
{
    const TickType_t xDelay500ms = pdMS_TO_TICKS(500UL);

    for (;;) 
    {
        /* Block until it is time to generate the software interrupt again. */
        vTaskDelay(xDelay500ms);

        /* 
            Generate the interrupt, printing a message both before and after
            the interrupt has been generated, so the sequence of execution is
            evident from the output.

            The syntax used to generate a software interrupt is dependent on
            the FreeRTOS port being used. The syntax used below can only be
            used with the FreeRTOS Windows port, in which such interrupts are
            only simulated. 
        */
        vPrintString("Periodic task - About to generate an interrupt.\r\n");

        vPortGenerateSimulatedInterrupt(mainINTERRUPT_NUMBER);

        vPrintString("Periodic task - Interrupt generated.\r\n\r\n\r\n");
    }
}

static void vHandlerTask(void *pvParameters) 
{
    for (;;) 
    {
        /* 
            Use the semaphore to wait for the event. The semaphore was created
            before the scheduler was started, so before this task ran for the
            first time. The task blocks indefinitely, meaning this function
            call will only return once the semaphore has been successfully
            obtained - so there is no need to check the value returned by
            xSemaphoreTake(). 
        */
        xSemaphoreTake(xBinarySemaphore, portMAX_DELAY);

        /* 
            To get here the event must have occurred. Process the event (in
            this Case, just print out a message).
        */
        vPrintString("Handler task - Processing event.\r\n");
    }
}

static uint32_t ulExampleInterruptHandler( void )
{
    BaseType_t xHigherPriorityTaskWoken;

    /* 
        The xHigherPriorityTaskWoken parameter must be initialized to
        pdFALSE as it will get set to pdTRUE inside the interrupt safe
        API function if a context switch is required. 
    */

    xHigherPriorityTaskWoken = pdFALSE;

    /* 
        'Give' the semaphore to unblock the task, passing in the address of
        xHigherPriorityTaskWoken as the interrupt safe API function's
        pxHigherPriorityTaskWoken parameter.
    */

    xSemaphoreGiveFromISR( xBinarySemaphore, &xHigherPriorityTaskWoken );

    /* 
        Pass the xHigherPriorityTaskWoken value into portYIELD_FROM_ISR().
        If xHigherPriorityTaskWoken was set to pdTRUE inside
        xSemaphoreGiveFromISR() then calling portYIELD_FROM_ISR() will request
        a context switch. If xHigherPriorityTaskWoken is still pdFALSE then
        calling portYIELD_FROM_ISR() will have no effect. Unlike most FreeRTOS
        ports, the Windows port requires the ISR to return a value - the return
        statement is inside the Windows version of portYIELD_FROM_ISR(). 
    */
    portYIELD_FROM_ISR( xHigherPriorityTaskWoken );
}

int main(void) {
    /* 
        Before a semaphore is used it must be explicitly created. In this
        example a binary semaphore is created.

        The semaphore is created in the ‘empty’ state, meaning the semaphore must
        first be given before it can be taken (obtained) using the xSemaphoreTake() function.
     */

    xBinarySemaphore = xSemaphoreCreateBinary();

    /* Check the semaphore was created successfully. */
    if (xBinarySemaphore != NULL) 
    {
        /* 
            Create the 'handler' task, which is the task to which interrupt
            processing is deferred. This is the task that will be synchronized
            with the interrupt. The handler task is created with a high priority
            to ensure it runs immediately after the interrupt exits. In this
            case a priority of 3 is chosen. 
        */
        xTaskCreate(vHandlerTask, "Handler", 1000, NULL, 3, NULL);

        /*
            Create the task that will periodically generate a software
            interrupt. This is created with a priority below the handler task
            to ensure it will get preempted each time the handler task exits
            the Blocked state.
        */
        xTaskCreate(vPeriodicTask, "Periodic", 1000, NULL, 1, NULL);

        /* 
            Install the handler for the software interrupt. The syntax necessary
            to do this is dependent on the FreeRTOS port being used. The syntax
            shown here can only be used with the FreeRTOS windows port, where
            such interrupts are only simulated.
        */
        vPortSetInterruptHandler(mainINTERRUPT_NUMBER, ulExampleInterruptHandler);

        /* Start the scheduler so the created tasks start executing. */
        vTaskStartScheduler();
    }
    for (;;)
        ;
}
#endif /* DEFERRED_INT_1 */



#if RECOMMENDED

static void vUARTReceiveHandlerTask(void *pvParameters) 
{
    /* xMaxExpectedBlockTime holds the maximum time expected between two interrupts. */
    const TickType_t xMaxExpectedBlockTime = pdMS_TO_TICKS(500);
    
    for (;;) 
    {
        /* 
            The semaphore is 'given' by the UART's receive (Rx) interrupt.
            Wait a maximum of xMaxExpectedBlockTime ticks for the next
            interrupt. 
        */
        if (xSemaphoreTake(xBinarySemaphore, xMaxExpectedBlockTime) == pdPASS) {

            /*
                The semaphore was obtained. Process ALL pending Rx events before
                calling xSemaphoreTake() again. Each Rx event will have placed a
                character in the UART's receive FIFO, and UART_RxCount() is
                assumed to return the number of characters in the FIFO.
            */
            while (UART_RxCount() > 0) {
                /* 
                    UART_ProcessNextRxEvent() is assumed to process one Rx
                    character, reducing the number of characters in the FIFO by 1.
                */
                UART_ProcessNextRxEvent();
            }
            /* 
                No more Rx events are pending (there are no more characters in
                the FIFO), so loop back and call xSemaphoreTake() to wait for
                the next interrupt. Any interrupts occurring between this point
                in the code and the call to xSemaphoreTake() will be latched in
                the semaphore, so will not be lost. */
        } else {
            /* 
                An event was not received within the expected time. Check for,
                and if necessary clear, any error conditions in the UART that
                might be preventing the UART from generating any more interrupts.
            */
            UART_ClearErrors();
        }
    }
}

#endif /* RECOMMENDED */


#ifdef COUNTING

#define mainINTERRUPT_NUMBER 3

static void vPeriodicTask(void *pvParameters) 
{
    const TickType_t xDelay500ms = pdMS_TO_TICKS(500UL);

    for (;;) 
    {
        vTaskDelay(xDelay500ms);
        vPrintString("Periodic task - About to generate an interrupt.\r\n");

        vPortGenerateSimulatedInterrupt(mainINTERRUPT_NUMBER);

        vPrintString("Periodic task - Interrupt generated.\r\n\r\n\r\n");
    }
}

static void vHandlerTask(void *pvParameters) 
{
    for (;;) 
    {
        xSemaphoreTake(xCountingSemaphore, portMAX_DELAY);

        vPrintString("Handler task - Processing event.\r\n");
    }
}

static uint32_t ulExampleInterruptHandler( void )
{
    BaseType_t xHigherPriorityTaskWoken;

    xHigherPriorityTaskWoken = pdFALSE;

    /*
        'Give' the semaphore multiple times. The first will unblock the deferred
        interrupt handling task, the following 'gives' are to demonstrate that
        the semaphore latches the events to allow the task to which interrupts
        are deferred to process them in turn, without events getting lost. This
        simulates multiple interrupts being received by the processor, even though 
        in this case the events are simulated within a single interrupt occurrence.
    */

    xSemaphoreGiveFromISR( xCountingSemaphore, &xHigherPriorityTaskWoken );
    xSemaphoreGiveFromISR( xCountingSemaphore, &xHigherPriorityTaskWoken );
    xSemaphoreGiveFromISR( xCountingSemaphore, &xHigherPriorityTaskWoken );

    portYIELD_FROM_ISR( xHigherPriorityTaskWoken );
}

int main(void) 
{

    xCountingSemaphore = xSemaphoreCreateCounting( 10, 0 );

    /* Check the semaphore was created successfully. */
    if (xCountingSemaphore != NULL) 
    {
        xTaskCreate(vHandlerTask, "Handler", 1000, NULL, 3, NULL);

        xTaskCreate(vPeriodicTask, "Periodic", 1000, NULL, 1, NULL);

        vPortSetInterruptHandler(mainINTERRUPT_NUMBER, ulExampleInterruptHandler);

        vTaskStartScheduler();
    }
    for (;;)
        ;
}

#endif /* COUNTING */