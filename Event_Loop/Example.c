#include <stdbool.h>
#include "FreeRTOSConfig.h"
#include "Event.h"

enum event_signals {
    TIMEOUT_SIG = USER_SIG,
    BUTTON_PRESSED_SIG,
    BUTTON_RELEASED_SIG,
};

typedef struct {
    event_loop_handle_t super; 
    enum {
        OFF_STATE,
        ON_STATE
    }state;
    time_event_t time_event;
} blinky_button_loop_handle;

void blinkybutton_event_handler_init(blinky_button_loop_handle * const me);

static void blinkybutton_event_handler(blinky_button_loop_handle * const me, event_t const * const e) 
{

    if(e->sig == INIT_SIG){
        // LED OFF
        time_event_arm(&me->time_event, (200 / portTICK_RATE_MS));
        me->state = OFF_STATE;
        return;
    }

    switch(me->state)
    {
        case OFF_STATE:{
            switch (e->sig) {
                case TIMEOUT_SIG:{
                    // LED ON
                    time_event_arm(&me->time_event, (200 / portTICK_RATE_MS));
                    me->state = ON_STATE;
                    break;
                }
                case BUTTON_PRESSED_SIG:{
                    // led on
                    break;
                }
                case BUTTON_RELEASED_SIG:{
                    break;
                }
            
            }
            break;
        }
        case ON_STATE:{
            switch (e->sig) {
                case TIMEOUT_SIG:{
                    time_event_arm(&me->time_event, (800 / portTICK_RATE_MS));
                    me->state = OFF_STATE;

                    break;
                }
                case BUTTON_PRESSED_SIG:{
                    // led on
                    break;
                }
                case BUTTON_RELEASED_SIG:{
                    break;
                }
            }
            break;
        }
        default: {
            // shouldnt reach here
            configASSERT(0);

        }
    }
}

void blinkybutton_event_handler_init(blinky_button_loop_handle * const me)
{
    event_loop_init(&me->super, (dispatch_handler)&blinkybutton_event_handler);
    me->time_event.type = TYPE_ONE_SHOT;
    time_event_init(&me->time_event, TIMEOUT_SIG, &me->super);
}

static StackType_t blinkyButton_stack[configMINIMAL_STACK_SIZE];     /* task stack */
static event_t *blinkyButton_queue[10];                              // memory buffer for event queue
static blinky_button_loop_handle blinkyButton;                       // Instance of the loop handle
event_loop_handle_t *blinkybutton_loop_handle = &blinkyButton.super; // global pointer to the event loop handle so that other files can post events to it

/* the main function =======================================================*/
int main() {

    BSP_init(); /* initialize the BSP */

    /* create and start the BlinkyButton AO */
    blinkybutton_event_handler_init(&blinkyButton);

    event_loop_args_t loop_args = {
        1U,
        blinkyButton_queue,
        sizeof(blinkyButton_queue)/sizeof(blinkyButton_queue[0]),
        blinkyButton_stack,
        sizeof(blinkyButton_stack),
        0U
    };
    event_loop_start(blinkybutton_loop_handle,&loop_args);

    /* 
        Example Usage :
            static event_t const button_pressed_event = {BUTTON_PRESSED_SIG};
            event_post(blinkybutton_loop_handle,&button_pressed_event);
     */
    vTaskStartScheduler(); /* start the FreeRTOS scheduler... */


    return 0; /* NOTE: the scheduler does NOT return */
}