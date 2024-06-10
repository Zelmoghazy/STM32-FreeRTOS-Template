
#ifndef EVENT_H
#define EVENT_H

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "timers.h"

/*---------------------------------------------------------------------------*/
/* event_t facilities... */

typedef uint16_t signal_t; /* event signal */

enum reserved_signals {
    INIT_SIG, /* dispatched to the handle before entering event-loop */
    USER_SIG  /* first signal available to the users */
};

typedef enum {
    TYPE_ONE_SHOT = pdFALSE,    /* One-shot timer */
    TYPE_PERIODIC = pdTRUE      /* Periodic timer */
} timer_type_t;

/* Event base class */
typedef struct {
    signal_t sig; /* event signal */
    /* Can fit other event parameters*/
} event_t;

/*---------------------------------------------------------------------------*/
/* Actvie Object facilities... */

typedef struct event_loop_handle_t event_loop_handle_t; /* forward declaration */

/* Pointer to a function that returns nothing and takes two arguments */
typedef void (*dispatch_handler)(event_loop_handle_t * const me, event_t const * const event);

/* Supports both static and dynamic allocation */
struct event_loop_handle_t {
    TaskHandle_t thread;        /* private thread */
    StaticTask_t thread_cb;     /* thread control-block (static) */

    QueueHandle_t queue;        /* private message queue */
    StaticQueue_t queue_cb;     /* queue control-block (static) */

    dispatch_handler dispatch;  /* pointer to the dispatch() virtual function */

    /* active object data added in subclasses of Active */
};

typedef struct event_loop_args_t{
    uint8_t     priority;     
    event_t     **queue_buffer;
    uint32_t    queue_len;
    void        *stack_buffer;
    uint32_t    stack_size;
    uint16_t    opt;
}event_loop_args_t;

void event_loop_init(event_loop_handle_t * const me, dispatch_handler dispatch);
void event_loop_start(event_loop_handle_t * const me,event_loop_args_t * loop_args);
void event_post(event_loop_handle_t * const me, event_t const * const event);
void event_postFromISR(event_loop_handle_t * const me, event_t const * const event, BaseType_t *pxHigherPriorityTaskWoken);

/* Time event_t class */
typedef struct {
    event_t super;                           // inherit event_t 
    event_loop_handle_t *loop_handle;        // the event_loop that requested this time_event_t 
    TimerHandle_t timer;                     // private timer handle 
    StaticTimer_t timer_cb;                  // timer control-block (FreeRTOS static alloc) 
    timer_type_t  type;                      // timer type, periodic or one-shot 
} time_event_t;

void time_event_init(time_event_t * const me, signal_t sig, event_loop_handle_t *loop_handle);
void time_event_arm(time_event_t * const me, uint32_t millisec);
void time_event_disarm(time_event_t * const me);

/* static (i.e., class-wide) operation */
void time_event_tickFromISR(BaseType_t *pxHigherPriorityTaskWoken);

/*---------------------------------------------------------------------------*/
/* Assertion facilities... */

#define Q_ASSERT(check_)                   \
    if (!(check_)) {                       \
        Q_onAssert(this_module, __LINE__); \
    } else (void)0

void Q_onAssert(char const *module, int loc);

#endif /* EVENT_H */