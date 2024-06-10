
#include "Event.h" /* Free Active Object interface */

/*..........................................................................*/
void event_loop_init(event_loop_handle_t * const me, dispatch_handler dispatch) {
    me->dispatch = dispatch; /* assign the dispatch handler */
}

/*..........................................................................*/
static void event_loop(void *pvParameters) 
{
    event_loop_handle_t *me = (event_loop_handle_t *)pvParameters;
    static event_t const initial_event = { INIT_SIG };

    configASSERT(me); 

    /* dispatch initial event */
    (*me->dispatch)(me, &initial_event);

    for (;;) 
    {  
        event_t const *event; 

        /* wait for any event and receive it into object 'event' */
        xQueueReceive(me->queue, &event, portMAX_DELAY);

        configASSERT(event != (event_t const *)0);

        /* Dispatch received event */
        (*me->dispatch)(me, event); /* NO BLOCKING! */
    }
}

/*..........................................................................*/
void event_loop_start(event_loop_handle_t * const me, event_loop_args_t * loop_args)
{
    StackType_t *stack_buffer = loop_args->stack_buffer;
    uint32_t stack_depth = (loop_args->stack_size / sizeof(StackType_t));

    (void)loop_args->opt; /* unused parameter do this for warnings */

    me->queue = xQueueCreateStatic(loop_args->queue_len,                    // queue length
                                   sizeof(event_t *),                       // item size 
                                   (uint8_t *)loop_args->queue_buffer,      // queue storage - provided by user 
                                   &me->queue_cb);                          // queue control block 

    configASSERT(me->queue);           

    me->thread = xTaskCreateStatic(&event_loop,                             // the thread function
                                   "Main Event Loop" ,                      // the name of the task
                                   stack_depth,                             // stack depth 
                                   me,                                      // the 'pvParameters' parameter 
                                   loop_args->priority + tskIDLE_PRIORITY,  // FreeRTOS priority 
                                   stack_buffer,                            // stack storage - provided by user 
                                   &me->thread_cb);                         // task control block 

    configASSERT(me->thread);       
}

/*..........................................................................*/
void event_post(event_loop_handle_t * const me, event_t const * const event) 
{
    BaseType_t status = xQueueSendToBack(me->queue, (void *)&event, (TickType_t)0);

    configASSERT(status == pdTRUE);
}

/*..........................................................................*/
void event_postFromISR(event_loop_handle_t * const me, event_t const * const event, BaseType_t *pxHigherPriorityTaskWoken)
{
    BaseType_t status = xQueueSendToBackFromISR(me->queue, (void *)&event, pxHigherPriorityTaskWoken);

    configASSERT(status == pdTRUE);
}

/*--------------------------------------------------------------------------*/
/* Time event_t services... */
static void time_event_callback(TimerHandle_t xTimer);

/*..........................................................................*/
void time_event_init(time_event_t * const me, signal_t sig, event_loop_handle_t *loop_handle) {
    me->super.sig = sig;
    me->loop_handle = loop_handle;

    /* Create a timer object */
    me->timer = xTimerCreateStatic("TE", 1U, me->type, me, time_event_callback, &me->timer_cb);
    configASSERT(me->timer);            /* timer must be created */
}

/*..........................................................................*/
void time_event_arm(time_event_t * const me, uint32_t millisec) {
    TickType_t ticks;
    BaseType_t status;
    BaseType_t xHigherPriorityTaskWoken;

    ticks = (millisec / portTICK_PERIOD_MS);
    if (ticks == 0U) {
        ticks = 1U;
    }

    if (xPortIsInsideInterrupt() == pdTRUE) {
        xHigherPriorityTaskWoken = pdFALSE;

        status = xTimerChangePeriodFromISR(me->timer, ticks, &xHigherPriorityTaskWoken);
        configASSERT(status == pdPASS); 

        portEND_SWITCHING_ISR(xHigherPriorityTaskWoken);
    }
    else {
        status = xTimerChangePeriod(me->timer, ticks, 0);
        configASSERT(status == pdPASS);
    }
}

/*..........................................................................*/
void time_event_disarm(time_event_t * const me) {
    BaseType_t xHigherPriorityTaskWoken;
    BaseType_t status;

    if (xPortIsInsideInterrupt() == pdTRUE) {
        xHigherPriorityTaskWoken = pdFALSE;
        status = xTimerStopFromISR(me->timer, &xHigherPriorityTaskWoken);
        configASSERT(status == pdPASS);

        portEND_SWITCHING_ISR(xHigherPriorityTaskWoken);
    }
    else {
        status = xTimerStop(me->timer, 0);
        configASSERT(status == pdPASS);
    }
}

/*..........................................................................*/
/* Use this macro to get the container of time_event_t struct
 *  since xTimer pointing to timer_cb
 */
#define GET_TIME_EVENT_HEAD(ptr) \
    (time_event_t*)((uintptr_t)(ptr) - offsetof(time_event_t, timer_cb))

static void time_event_callback(TimerHandle_t xTimer) {
    /* Also can use pvTimerGetTimerID(xTimer) */
    time_event_t * const t = GET_TIME_EVENT_HEAD(xTimer);

    /* Callback always called from non-interrupt context so no need
     * to check xPortIsInsideInterrupt
     */
    event_post(t->loop_handle, &t->super);
}