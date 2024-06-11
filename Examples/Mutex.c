#include "FreeRTOS.h"
#include "task.h"
#include "timers.h"
#include "queue.h"
#include "semphr.h"
#include "event_groups.h"


/* 
    To ensure data consistency is maintained at all times, access to a resource that is shared between tasks, or is
    shared between tasks and interrupts, must be managed using a 'mutual exclusion' technique.

    The goal is to ensure that, once a task starts to access a shared resource that is not re-entrant and not 
    thread-safe, the same task has exclusive access to the resource until the resource has been returned to a 
    consistent state.
 */

 /* 

    When used in a mutual exclusion scenario, the mutex can be thought of as a token that is 
    associated with the resource being shared.

    For a task to access the resource legitimately, it must first successfully 'take' the token 
    (be the token holder). When the token holder has finished with the resource, it must 'give' the token back. 

  */

/* 
    Only when the token has been returned can another task successfully take the token, and then safely access
    the same shared resource. A task is not permitted to access the shared resource unless it holds the token. 

    Priority inheritance works by temporarily raising the priority of the mutex holder to the priority of the highest
    priority task that is attempting to obtain the same mutex. The low priority task that holds the mutex 'inherits' the
    priority of the task waiting for the mutex

    The priority of the mutex holder is reset automatically to its original value when it gives the mutex back.

    It is also possible for a task to deadlock with itself. This will happen if a task attempts to take the same mutex
    more than once, without first returning the mutex.

    the task is in the Blocked state to wait for the mutex to be returned, but the task is
    already the mutex holder. A deadlock has occurred because the task is in the Blocked state to wait for itself.

    This type of deadlock can be avoided by using a recursive mutex in place of a standard mutex. A recursive mutex
    can be 'taken' more than once by the same task, and will be returned only after one call to 'give' the recursive
    mutex has been executed for every preceding call to 'take' the recursive mutex.
 */

static void prvNewPrintString(const char *pcString) 
{
    /* 
        The mutex is created before the scheduler is started, so already exists
        by the time this task executes.
        Attempt to take the mutex, blocking indefinitely to wait for the mutex
        if it is not available straight away. The call to xSemaphoreTake() will
        only return when the mutex has been successfully obtained, so there is
        no need to check the function return value. If any other delay period
        was used then the code must check that xSemaphoreTake() returns pdTRUE
        before accessing the shared resource (which in this case is standard
        out). As noted earlier in this book, indefinite time outs are not
        recommended for production code. 
    */
    xSemaphoreTake(xMutex, portMAX_DELAY);
    {
        /* 
            The following line will only execute once the mutex has been
            successfully obtained. Standard out can be accessed freely now as
            only one task can have the mutex at any one time.
        */
        printf("%s", pcString);
        fflush(stdout);
        /* The mutex MUST be given back! */
    }
    xSemaphoreGive(xMutex);
}

static void prvPrintTask(void *pvParameters) 
{
    char *pcStringToPrint;
    const TickType_t xMaxBlockTimeTicks = 0x20;
    /*
        Two instances of this task are created. The string printed by the task
        is passed into the task using the task's parameter. The parameter is
        cast to the required type. 
    */
    pcStringToPrint = (char *) pvParameters;

    for (;;) 
    {
        /* Print out the string using the newly defined function. */
        prvNewPrintString(pcStringToPrint);
        /* 
            Wait a pseudo random time. Note that rand() is not necessarily
            reentrant, but in this case it does not really matter as the code
            does not care what value is returned. In a more secure application
            a version of rand() that is known to be reentrant should be used -
            or calls to rand() should be protected using a critical section.
        */
        vTaskDelay((rand() % xMaxBlockTimeTicks));
    }
}

int main(void) 
{
    /* 
        Before a semaphore is used it must be explicitly created. In this
        example a mutex type semaphore is created.
    */

    xMutex = xSemaphoreCreateMutex();

    /* Check the semaphore was created successfully before creating the tasks. */

    if (xMutex != NULL) 
    {
        /* 
            Create two instances of the tasks that write to stdout. The string
            they write is passed in to the task as the task's parameter. The
            tasks are created at different priorities so some pre-emption will occur.
        */
        xTaskCreate(prvPrintTask, "Print1", 1000,
                    "Task 1 ***************************************\r\n", 1,
                    NULL);

        xTaskCreate(prvPrintTask, "Print2", 1000,
                    "Task 2 ---------------------------------------\r\n", 2,
                    NULL);

        /* Start the scheduler so the created tasks start executing. */
        vTaskStartScheduler();
    }
    for (;;)
        ;
}

#define RECURSIVE_MUTEX

#ifdef RECURSIVE_MUTEX

/* Recursive mutexes are variables of type SemaphoreHandle_t. */
SemaphoreHandle_t xRecursiveMutex;
/* The implementation of a task that creates and uses a recursive mutex. */
void vTaskFunction(void *pvParameters) 
{
    const TickType_t xMaxBlock20ms = pdMS_TO_TICKS(20);
    /* Before a recursive mutex is used it must be explicitly created. */
    xRecursiveMutex = xSemaphoreCreateRecursiveMutex();
    /* Check the semaphore was created successfully. configASSERT() is
    described in section 11.2. */
    configASSERT(xRecursiveMutex);
    /* As per most tasks, this task is implemented as an infinite loop. */
    for (;;) {
        /* ... */
        /* Take the recursive mutex. */
        if (xSemaphoreTakeRecursive(xRecursiveMutex, xMaxBlock20ms) == pdPASS) {
            /* The recursive mutex was successfully obtained. The task can now
            access the resource the mutex is protecting. At this point the
            recursive call count (which is the number of nested calls to
            xSemaphoreTakeRecursive()) is 1, as the recursive mutex has
            only been taken once. */
            /* While it already holds the recursive mutex, the task takes the
            mutex again. In a real application, this is only likely to occur
            inside a sub-function called by this task, as there is no
            practical reason to knowingly take the same mutex more than
            once. The calling task is already the mutex holder, so the
            second call to xSemaphoreTakeRecursive() does nothing more than
            increment the recursive call count to 2. */
            xSemaphoreTakeRecursive(xRecursiveMutex, xMaxBlock20ms);
            /* ... */
            /* The task returns the mutex after it has finished accessing the
            resource the mutex is protecting. At this point the recursive
            call count is 2, so the first call to xSemaphoreGiveRecursive()
            does not return the mutex. Instead, it simply decrements the
            recursive call count back to 1. */
            xSemaphoreGiveRecursive(xRecursiveMutex);
            /* The next call to xSemaphoreGiveRecursive() decrements the
            recursive call count to 0, so this time the recursive mutex is
            returned. */
            xSemaphoreGiveRecursive(xRecursiveMutex);
            /* Now one call to xSemaphoreGiveRecursive() has been executed for
            every proceeding call to xSemaphoreTakeRecursive(), so the task
            is no longer the mutex holder. */
        }
    }
}
#endif /* RECURSIVE_MUTEX */