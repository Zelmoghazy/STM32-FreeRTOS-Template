

#ifdef SENDER_RECIEVER

static void vSenderTask(void *pvParameters) 
{
    int32_t lValueToSend;
    BaseType_t xStatus;

    /* Two instances of this task are created so the value that is sent to
    the queue is passed in via the task parameter - this way each instance
    can use a different value. The queue was created to hold values of type
    int32_t, so cast the parameter to the required type. */
    lValueToSend = (int32_t) pvParameters;

    for (;;) {
        /* Send the value to the queue.

        The first parameter is the queue to which data is being sent. The
        queue was created before the scheduler was started, so before this
        task started to execute.

        The second parameter is the address of the data to be sent, in this
        case the address of lValueToSend.

        The third parameter is the Block time – the time the task should be
        kept in the Blocked state to wait for space to become available on
        the queue should the queue already be full. In this case a block
        time is not specified because the queue should never contain more
        than one item, and therefore never be full. */

        /* xQueueSend() is equivalent to, and exactly the same as,  xQueueSendToBack(). */

        xStatus = xQueueSendToBack(xQueue,          // The handle of the queue to which the data is being sent (written).
                                   &lValueToSend,   // A pointer to the data to be copied into the queue.
                                   0);              // The maximum amount of time the task should remain in the
                                                    // Blocked state to wait for space to become available on the
                                                    // queue, should the queue already be full.

        /* Setting xTicksToWait to portMAX_DELAY will cause the task to wait
         * indefinitely (without timing out) */

        if (xStatus != pdPASS) 
        {
            /* The send operation could not complete because the queue was full-
            this must be an error as the queue should never contain more than
            one item! */
            vPrintString("Could not send to the queue.\r\n");
        }
    }
}

static void vReceiverTask(void *pvParameters) {
    /* Declare the variable that will hold the values received from the queue. */
    int32_t lReceivedValue;
    BaseType_t xStatus;

    const TickType_t xTicksToWait = pdMS_TO_TICKS(100);

    for (;;) 
    {
        /* This call should always find the queue empty because this task will
           immediately remove any data that is written to the queue. */
        if (uxQueueMessagesWaiting(xQueue) != 0)   // uxQueueMessagesWaiting() queries the number of items currently in a queue
        {
            vPrintString("Queue should have been empty!\r\n");
        }
        /* Receive data from the queue.

        The first parameter is the queue from which data is to be received.
        The queue is created before the scheduler is started, and therefore
        before this task runs for the first time.

        The second parameter is the buffer into which the received data will
        be placed. In this case the buffer is simply the address of a
        variable that has the required size to hold the received data.

        The last parameter is the block time – the maximum amount of time
        that the task will remain in the Blocked state to wait for data to
        be available should the queue already be empty. */

        xStatus = xQueueReceive(xQueue,            // The handle of the queue from which the data is being received (read).
                                &lReceivedValue,   // A pointer to the memory into which the received data will be copied.
                                xTicksToWait);     // The maximum amount of time the task should
                                                   // remain in the Blocked state to wait for data to
                                                   // become available on the queue, if the queue is
                                                   // already be empty.

        if (xStatus == pdPASS) {
            /* Data was successfully received from the queue, print out the received value. */
            vPrintStringAndNumber("Received = ", lReceivedValue);
        } else {
            /* Data was not received from the queue even after waiting for 100ms.
               This must be an error as the sending tasks are free running and will be continuously writing to the queue. 
            */
            vPrintString("Could not receive from the queue.\r\n");
        }
    }
}

/* 
    Declare a variable of type QueueHandle_t. This is used to store the
    handle to the queue that is accessed by all three tasks. 
*/
QueueHandle_t xQueue;

int main(void) 
{
    /* The queue is created to hold a maximum of 5 values, each of which is
       large enough to hold a variable of type int32_t. */

    xQueue = xQueueCreate(5,                   // The maximum number of items that the queue being created can hold at any one time.
                          sizeof(int32_t));    // The size in bytes of each data item that can be stored in the queue.

    if (xQueue != NULL) {
        /* Create two instances of the task that will send to the queue. The
           task parameter is used to pass the value that the task will write
           to the queue, so one task will continuously write 100 to the queue
           while the other task will continuously write 200 to the queue. Both
           tasks are created at priority 1. */

        xTaskCreate(vSenderTask, "Sender1", 1000, (void *) 100, 1, NULL);
        xTaskCreate(vSenderTask, "Sender2", 1000, (void *) 200, 1, NULL);

        /* Create the task that will read from the queue. The task is created
           with priority 2, so above the priority of the sender tasks. */

        xTaskCreate(vReceiverTask, "Receiver", 1000, NULL, 2, NULL);

        /* Start the scheduler so the created tasks start executing. */
        vTaskStartScheduler();
    } 
    else
    {
        /* The queue could not be created. */

        /* If NULL is returned, then the queue cannot be created because there
           is insufficient heap memory available for FreeRTOS to allocate the queue
           data structures and storage area. */
    }
    /* If all is well then main() will never reach here as the scheduler will
       now be running the tasks. If main() does reach here then it is likely
       that there was insufficient FreeRTOS heap memory available for the idle
       task to be created. */
    for (;;);
}
#endif /* SENDER_RECIEVER */



#ifdef MULTIPLE_SOURCES

/* Define an enumerated type used to identify the source of the data. */
typedef enum
{ 
    eSender1,
    eSender2
} DataSource_t;

/* Define the structure type that will be passed on the queue. */
typedef struct {
    uint8_t ucValue;
    DataSource_t eDataSource;
} Data_t;

/* Declare two variables of type Data_t that will be passed on the queue. */
static const Data_t xStructsToSend[2] = {
    {100, eSender1}, /* Used by Sender1. */
    {200, eSender2}  /* Used by Sender2. */
};


/* 
    the sending tasks have the higher priority, so the queue will normally be full.
    This is because, as soon as the receiving task removes an item from the queue,
    it is pre-empted by one of the sending tasks which then immediately re-fills the queue.
    The sending task then re-enters the Blocked state to wait for space to become available on the queue again.
*/
static void vSenderTask(void *pvParameters) 
{
    BaseType_t xStatus;
    const TickType_t xTicksToWait = pdMS_TO_TICKS(100);
    /* As per most tasks, this task is implemented within an infinite loop. */
    for (;;) {
        /* Send to the queue.
        The second parameter is the address of the structure being sent. The
        address is passed in as the task parameter so pvParameters is used
        directly.
        The third parameter is the Block time - the time the task should be
        kept in the Blocked state to wait for space to become available on
        the queue if the queue is already full. A block time is specified
        because the sending tasks have a higher priority than the receiving
        task so the queue is expected to become full. The receiving task
        will remove items from the queue when both sending tasks are in the
        Blocked state. */
        xStatus = xQueueSendToBack(xQueue, pvParameters, xTicksToWait);
        if (xStatus != pdPASS) {
            /* The send operation could not complete, even after waiting for
            100ms. This must be an error as the receiving task should make
            space in the queue as soon as both sending tasks are in the
            Blocked state. */
            vPrintString("Could not send to the queue.\r\n");
        }
    }
}

/* The receiving task has the lowest priority so it only runs only when both sending tasks are in the Blocked state. */
static void vReceiverTask(void *pvParameters) 
{
    /* Declare the structure that will hold the values received from the queue. */
    Data_t xReceivedStructure;
    BaseType_t xStatus;

    for (;;) 
    {
        /* Because it has the lowest priority this task will only run when the
           sending tasks are in the Blocked state. The sending tasks will only
           enter the Blocked state when the queue is full so this task always
           expects the number of items in the queue to be equal to the queue
           length, which is 3 in this case. */

        if (uxQueueMessagesWaiting(xQueue) != 3) {
            vPrintString("Queue should have been full!\r\n");
        }
        /* 
            Receive from the queue.

            The second parameter is the buffer into which the received data will
            be placed. In this case the buffer is simply the address of a
            variable that has the required size to hold the received structure.

            The last parameter is the block time - the maximum amount of time
            that the task will remain in the Blocked state to wait for data to
            be available if the queue is already empty. In this case a block
            time is not necessary because this task will only run when the
            queue is full. 
        */
        xStatus = xQueueReceive(xQueue, &xReceivedStructure, 0);

        if (xStatus == pdPASS) 
        {
            /* Data was successfully received from the queue, print out the
            received value and the source of the value. */
            if (xReceivedStructure.eDataSource == eSender1) {
                vPrintStringAndNumber("From Sender 1 = ", xReceivedStructure.ucValue);
            } else {
                vPrintStringAndNumber("From Sender 2 = ", xReceivedStructure.ucValue);
            }
        } else {
            /* Nothing was received from the queue. This must be an error as
            this task should only run when the queue is full. */
            vPrintString("Could not receive from the queue.\r\n");
        }
    }
}

int main(void) 
{
    /* The queue is created to hold a maximum of 3 structures of type Data_t. */
    xQueue = xQueueCreate(3, sizeof(Data_t));

    if (xQueue != NULL) {
        /* Create two instances of the task that will write to the queue. The
           parameter is used to pass the structure that the task will write to
           the queue, so one task will continuously send xStructsToSend[ 0 ]
           to the queue while the other task will continuously send
           xStructsToSend[ 1 ]. Both tasks are created at priority 2, which is
           above the priority of the receiver. */
        xTaskCreate(vSenderTask, "Sender1", 1000, &(xStructsToSend[0]), 2, NULL);
        xTaskCreate(vSenderTask, "Sender2", 1000, &(xStructsToSend[1]), 2, NULL);

        /* Create the task that will read from the queue. The task is created
           with priority 1, so below the priority of the sender tasks. */
        xTaskCreate(vReceiverTask, "Receiver", 1000, NULL, 1, NULL);
        /* Start the scheduler so the created tasks start executing. */
        vTaskStartScheduler();
    } else {
        /* The queue could not be created. */
    }
    /* If all is well then main() will never reach here as the scheduler will
    now be running the tasks. If main() does reach here then it is likely
    that there was insufficient heap memory available for the idle task to
    be created. Chapter 3 provides more information on heap memory
    management. */
    for (;;) ;
}
#endif /* MULTIPLE_SOURCES */



#ifdef VARIABLE_SIZE

/* Declare a variable of type QueueHandle_t to hold the handle of the queue being created. */
QueueHandle_t xPointerQueue;
/* Create a queue that can hold a maximum of 5 pointers, in this case character pointers. */
xPointerQueue = xQueueCreate(5, sizeof(char *));

/* A task that obtains a buffer, writes a string to the buffer, then
sends the address of the buffer to the queue created in Listing 5.13. */
void vStringSendingTask(void *pvParameters) 
{
    char *pcStringToSend;
    const size_t xMaxStringLength = 50;
    BaseType_t xStringNumber = 0;

    for (;;) 
    {
        /* Obtain a buffer that is at least xMaxStringLength characters big.
           The implementation of prvGetBuffer() is not shown – it might obtain
           the buffer from a pool of pre-allocated buffers, or just allocate
           the buffer dynamically. */
        pcStringToSend = (char *) prvGetBuffer(xMaxStringLength);

        /* Write a string into the buffer. */
        snprintf(pcStringToSend, xMaxStringLength, "String number %d\r\n",xStringNumber);

        /* Increment the counter so the string is different on each iteration of this task. */
        xStringNumber++;

        /* Send the address of the buffer to the queue that was created */
        xQueueSend(xPointerQueue, &pcStringToSend, portMAX_DELAY);
    }
}

void vStringReceivingTask(void *pvParameters) 
{
    char *pcReceivedString;
    for (;;) 
    {
        /* Receive the address of a buffer. */
        xQueueReceive(xPointerQueue,     /* The handle of the queue. */
                      &pcReceivedString, /* Store the buffer's address in pcReceivedString. */
                      portMAX_DELAY);

        /* The buffer holds a string, print it out. */
        vPrintString(pcReceivedString);

        /* The buffer is not required any more - release it so it can be freed, or re-used. */
        prvReleaseBuffer(pcReceivedString);
    }
}
#endif /* VARIABLE_SIZE */



#ifdef TCP/IP

/* A subset of the enumerated types used in the TCP/IP stack to identify events. */
typedef enum {
    eNetworkDownEvent = 0,  /* The network interface has been lost, or needs (re)connecting. */
    eNetworkRxEvent,        /* A packet has been received from the network. */
    eTCPAcceptEvent,        /* FreeRTOS_accept() called to accept or wait for a new client. */

    /* Other event types appear here but are not shown in this listing. */
} eIPEvent_t;

/* The structure that describes events, and is sent on a queue to the TCP/IP task. */
typedef struct IP_TASK_COMMANDS 
{
    eIPEvent_t eEventType; /* An enumerated type that identifies the event.*/
    void *pvData; /* A generic pointer that can hold a value, or point to a buffer. */
} IPStackEvent_t;

void vSendRxDataToTheTCPTask(NetworkBufferDescriptor_t *pxRxedData) 
{
    IPStackEvent_t xEventStruct;
    /* Complete the IPStackEvent_t structure. The received data is stored in pxRxedData. */
    xEventStruct.eEventType = eNetworkRxEvent;
    xEventStruct.pvData = (void *) pxRxedData;
    /* Send the IPStackEvent_t structure to the TCP/IP task. */
    xSendEventStructToIPTask(&xEventStruct);
}

void vSendAcceptRequestToTheTCPTask(Socket_t xSocket) 
{
    IPStackEvent_t xEventStruct;
    /* Complete the IPStackEvent_t structure. */
    xEventStruct.eEventType = eTCPAcceptEvent;
    xEventStruct.pvData = (void *) xSocket;
    /* Send the IPStackEvent_t structure to the TCP/IP task. */
    xSendEventStructToIPTask(&xEventStruct);
}

void vSendNetworkDownEventToTheTCPTask(Socket_t xSocket) 
{
    IPStackEvent_t xEventStruct;
    /* Complete the IPStackEvent_t structure. */
    xEventStruct.eEventType = eNetworkDownEvent;
    xEventStruct.pvData = NULL; /* Not used, but set to NULL for completeness. */
    /* Send the IPStackEvent_t structure to the TCP/IP task. */
    xSendEventStructToIPTask(&xEventStruct);
}

int main(void)
{
    IPStackEvent_t xReceivedEvent;

    /*  Block on the network event queue until either an event is received, or
        xNextIPSleep ticks pass without an event being received. eEventType is
        set to eNoEvent in case the call to xQueueReceive() returns because it
        timed out, rather than because an event was received. 
    */
    xReceivedEvent.eEventType = eNoEvent;
    xQueueReceive(xNetworkEventQueue, &xReceivedEvent, xNextIPSleep);

    /* Which event was received, if any? */
    switch (xReceivedEvent.eEventType) 
    {
        case eNetworkDownEvent:
            /* Attempt to (re)establish a connection. */
            prvProcessNetworkDownEvent();
            break;
        case eNetworkRxEvent:
            /* The network interface has received a new packet. A pointer to the
               received data is stored in the pvData member of the received
               IPStackEvent_t structure. Process the received data. */
            prvHandleEthernetPacket((NetworkBufferDescriptor_t *) (xReceivedEvent.pvData));
            break;
        case eTCPAcceptEvent:
            /* The FreeRTOS_accept() API function was called. The handle of the
               socket that is accepting a connection is stored in the pvData
               member of the received IPStackEvent_t structure. */
            xSocket = (FreeRTOS_Socket_t *) (xReceivedEvent.pvData);
            xTCPCheckNewClient(xSocket);
            break;
        /* Other event types are processed in the same way, but are not shown here. */
    }
}

#endif /* TCP/IP */




/*  A design that uses a queue set to receive data from multiple sources is less
   neat, and less efficient, than a design that achieves the same functionality
   using a single queue that receives structures. For that reason, it is
    recommended to only use queue sets if design constraints make their use
   absolutely necessary
*/

#ifdef QUEUE_SET

/* Declare two variables of type QueueHandle_t. Both queues are added to the same queue set. */
static QueueHandle_t xQueue1 = NULL;
static QueueHandle_t xQueue2 = NULL;

/* Declare a variable of type QueueSetHandle_t. This is the queue set to which the two queues are added. */
static QueueSetHandle_t xQueueSet = NULL;

int main(void) 
{
    /* Create the two queues, both of which send character pointers. The
       priority of the receiving task is above the priority of the sending
       tasks, so the queues will never have more than one item in them at any one time*/
    xQueue1 = xQueueCreate(1, sizeof(char *));
    xQueue2 = xQueueCreate(1, sizeof(char *));

    /* Create the queue set. Two queues will be added to the set, each of
       which can contain 1 item, so the maximum number of queue handles the
       queue set will ever have to hold at one time is 2 (2 queues multiplied
       by 1 item per queue). */
    xQueueSet = xQueueCreateSet(1 * 2);

    /* Add the two queues to the set. */
    xQueueAddToSet(xQueue1, xQueueSet);
    xQueueAddToSet(xQueue2, xQueueSet);

    /* Create the tasks that send to the queues. */
    xTaskCreate(vSenderTask1, "Sender1", 1000, NULL, 1, NULL);
    xTaskCreate(vSenderTask2, "Sender2", 1000, NULL, 1, NULL);

    /* Create the task that reads from the queue set to determine which of the two queues contain data. */
    xTaskCreate(vReceiverTask, "Receiver", 1000, NULL, 2, NULL);

    /* Start the scheduler so the created tasks start executing. */
    vTaskStartScheduler();

    for (;;) ;

    return 0;
}

/*
    The first sending task uses xQueue1 to send a character pointer
    to the receiving task every 100 milliseconds. 
*/
void vSenderTask1(void *pvParameters) 
{
    const TickType_t xBlockTime = pdMS_TO_TICKS(100);

    const char *const pcMessage = "Message from vSenderTask1\r\n";

    for (;;) 
    {
        /* Block for 100ms. */
        vTaskDelay(xBlockTime);

        /* Send this task's string to xQueue1. It is not necessary to use a
           block time, even though the queue can only hold one item. This is
           because the priority of the task that reads from the queue is
           higher than the priority of this task; as soon as this task writes
           to the queue it will be pre-empted by the task that reads from the
           queue, so the queue will already be empty again by the time the
           call to xQueueSend() returns. The block time is set to 0. */
        xQueueSend(xQueue1, &pcMessage, 0);
    }
}
/*-----------------------------------------------------------*/

/* The second sending task uses xQueue2 to send a character pointer to the receiving task every 200 milliseconds. */

void vSenderTask2(void *pvParameters) 
{
    const TickType_t xBlockTime = pdMS_TO_TICKS(200);

    const char *const pcMessage = "Message from vSenderTask2\r\n";

    for (;;)
    {
        /* Block for 200ms. */
        vTaskDelay(xBlockTime);

        /* Send this task's string to xQueue2. It is not necessary to use a
           block time, even though the queue can only hold one item. This is
           because the priority of the task that reads from the queue is
           higher than the priority of this task; as soon as this task writes
           to the queue it will be pre-empted by the task that reads from the
           queue, so the queue will already be empty again by the time the
           call to xQueueSend() returns. The block time is set to 0. */
        xQueueSend(xQueue2, &pcMessage, 0);
    }
}

void vReceiverTask(void *pvParameters) 
{
    QueueHandle_t xQueueThatContainsData;
    char *pcReceivedString;

    for (;;)
    {
        /* Block on the queue set to wait for one of the queues in the set to
           contain data. Cast the QueueSetMemberHandle_t value returned from
           xQueueSelectFromSet() to a QueueHandle_t, as it is known all the
           members of the set are queues (the queue set does not contain any
           semaphores). */
        xQueueThatContainsData = (QueueHandle_t) xQueueSelectFromSet(xQueueSet, portMAX_DELAY);

        /* An indefinite block time was used when reading from the queue set,
           so xQueueSelectFromSet() will not have returned unless one of the
           queues in the set contained data, and xQueueThatContainsData cannot
           be NULL. Read from the queue. It is not necessary to specify a
           block time because it is known the queue contains data. The block
           time is set to 0. */
        xQueueReceive(xQueueThatContainsData, &pcReceivedString, 0);

        /* Print the string received from the queue. */
        vPrintString(pcReceivedString);
    }
}
#endif /* QUEUE_SET */



#ifdef QUEUE_SET_SEM

/* The handle of the queue from which character pointers are received. */
QueueHandle_t xCharPointerQueue;

/* The handle of the queue from which uint32_t values are received. */
QueueHandle_t xUint32tQueue;

/* The handle of the binary semaphore. */
SemaphoreHandle_t xBinarySemaphore;

/* The queue set to which the two queues and the binary semaphore belong. */
QueueSetHandle_t xQueueSet;

void vAMoreRealisticReceiverTask(void *pvParameters) 
{
    QueueSetMemberHandle_t xHandle;
    char *pcReceivedString;
    uint32_t ulRecievedValue;

    const TickType_t xDelay100ms = pdMS_TO_TICKS(100);

    for (;;) 
    {
        /* Block on the queue set for a maximum of 100ms to wait for one of the
           members of the set to contain data. */
        xHandle = xQueueSelectFromSet(xQueueSet, xDelay100ms);

        /* Test the value returned from xQueueSelectFromSet(). 
           If the returned value is NULL then the call to xQueueSelectFromSet() timed out.
           If the returned value is not NULL then the returned value will be the
           handle of one of the set's members. The QueueSetMemberHandle_t
           value can be cast to either a QueueHandle_t or a SemaphoreHandle_t.
           Whether an explicit cast is required depends on the compiler. */
        if (xHandle == NULL) {
            /* The call to xQueueSelectFromSet() timed out. */
        } else if (xHandle == (QueueSetMemberHandle_t) xCharPointerQueue) {
            /* The call to xQueueSelectFromSet() returned the handle of the
               queue that receives character pointers. Read from the queue.
               The queue is known to contain data, so a block time of 0 is
               used. */
            xQueueReceive(xCharPointerQueue, &pcReceivedString, 0);
            /* The received character pointer can be processed here... */
        } else if (xHandle == (QueueSetMemberHandle_t) xUint32tQueue) {
            /* The call to xQueueSelectFromSet() returned the handle of the
               queue that receives uint32_t types. Read from the queue. The
               queue is known to contain data, so a block time of 0 is used. */
            xQueueReceive(xUint32tQueue, &ulRecievedValue, 0);
            /* The received value can be processed here... */
        } else if (xHandle == (QueueSetMemberHandle_t) xBinarySemaphore) {
            /* The call to xQueueSelectFromSet() returned the handle of the
               binary semaphore. Take the semaphore now. The semaphore is
               known to be available so a block time of 0 is used. */
            xSemaphoreTake(xBinarySemaphore, 0);
            /* Whatever processing is necessary when the semaphore is taken can be performed here... */
        }
    }
}

#endif /* QUEUE_SET_SEM */

#define MAIL_BOX 

#ifdef MAIL_BOX

/* A mailbox can hold a fixed size data item. The size of the data item is set
   when the mailbox (queue) is created. In this example the mailbox is created
   to hold an Example_t structure. Example_t includes a time stamp to allow the
   data held in the mailbox to note the time at which the mailbox was last
   updated. The time stamp used in this example is for demonstration purposes
   only - a mailbox can hold any data the application writer wants, and the
   data does not need to include a time stamp. */
typedef struct xExampleStructure 
{
    TickType_t xTimeStamp;
    uint32_t ulValue;
} Example_t;

/* A mailbox is a queue, so its handle is stored in a variable of type QueueHandle_t. */
QueueHandle_t xMailbox;

void vAFunction(void) 
{
    /* Create the queue that is going to be used as a mailbox. The queue has
       a length of 1 to allow it to be used with the xQueueOverwrite() API
       function, which is described below. */
    xMailbox = xQueueCreate(1, sizeof(Example_t));
}

void vUpdateMailbox( uint32_t ulNewValue )
{
    Example_t xData;

    /* Write the new data into the Example_t structure.*/
    xData.ulValue = ulNewValue;

    /* Use the RTOS tick count as the time stamp stored in the Example_t structure. */
    xData.xTimeStamp = xTaskGetTickCount();
    /* Send the structure to the mailbox - overwriting any data that is already in the mailbox. */
    xQueueOverwrite( xMailbox, &xData );
    /* if the queue is already full, then xQueueOverwrite() overwrites data that is already in the queue. 
        xQueueOverwrite() must only be used with queues that have a length of one. */
}

BaseType_t vReadMailbox(Example_t *pxData) 
{
    TickType_t xPreviousTimeStamp;
    BaseType_t xDataUpdated;

    /* This function updates an Example_t structure with the latest value
       received from the mailbox. Record the time stamp already contained in
       *pxData before it gets overwritten by the new data. */
    xPreviousTimeStamp = pxData->xTimeStamp;

    /* Update the Example_t structure pointed to by pxData with the data
    contained in the mailbox. If xQueueReceive() was used here then the
    mailbox would be left empty, and the data could not then be read by
    any other tasks. Using xQueuePeek() instead of xQueueReceive() ensures
    the data remains in the mailbox.

    A block time is specified, so the calling task will be placed in the
    Blocked state to wait for the mailbox to contain data should the mailbox
    be empty. An infinite block time is used, so it is not necessary to
    check the value returned from xQueuePeek(), as xQueuePeek() will only
    return when data is available. */
    xQueuePeek(xMailbox, pxData, portMAX_DELAY);
    /* xQueuePeek() receives (reads) an item from a queue without removing the item from the queue */
    /* Return pdTRUE if the value read from the mailbox has been updated since
    this function was last called. Otherwise return pdFALSE. */
    if (pxData->xTimeStamp > xPreviousTimeStamp) {
        xDataUpdated = pdTRUE;
    } else {
        xDataUpdated = pdFALSE;
    }
    return xDataUpdated;
}
#endif /* MAIL_BOX */