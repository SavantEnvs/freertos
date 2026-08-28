#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "FreeRTOS.h"
#include "FreeRTOS_IP.h"
#include "task.h"

BaseType_t xApplicationGetRandomNumber( uint32_t * pulNumber )
{
    *pulNumber = 1337;
    return pdTRUE;
}


uint32_t ulApplicationGetNextSequenceNumber( uint32_t ulSourceAddress,
                                             uint16_t usSourcePort,
                                             uint32_t ulDestinationAddress,
                                             uint16_t usDestinationPort )
{
    uint32_t pulNumber = 0;
    xApplicationGetRandomNumber( &pulNumber );
    return pulNumber;
}

/*  CALLED BY FREERTOS when conducting a DNS query
 *  Function that returns pdTRUE if the pcName matches the LLMNR node name */
BaseType_t xApplicationDNSQueryHook( const char * pcName )
{
    return pdTRUE;
}

BaseType_t xApplicationDNSQueryHook_Multi( NetworkEndPoint_t * pxEndPoint,
                                           const char * pcName ) {
    return pdTRUE;
}

const char *pcApplicationHostnameHook( void ) {
    return "localhost";
}

/* void vApplicationGetIdleTaskMemory( StaticTask_t ** ppxIdleTaskTCBBuffer, */
/*                                     StackType_t ** ppxIdleTaskStackBuffer, */
/*                                     configSTACK_DEPTH_TYPE * puxIdleTaskStackSize ); */

void vApplicationGetIdleTaskMemory( StaticTask_t ** ppxIdleTaskTCBBuffer,
                                    StackType_t ** ppxIdleTaskStackBuffer,
                                    configSTACK_DEPTH_TYPE * puxIdleTaskStackSize )
{
    static StaticTask_t xIdleTaskTCB;
    static StackType_t uxIdleTaskStack[ configMINIMAL_STACK_SIZE ];

    *ppxIdleTaskTCBBuffer = &( xIdleTaskTCB );
    *ppxIdleTaskStackBuffer = &( uxIdleTaskStack[ 0 ] );
    *puxIdleTaskStackSize = configMINIMAL_STACK_SIZE;
}


void vTaskDelay(const TickType_t xTicksToDelay) {}

void vAssertCalled(const char *pos, int linenum) {
    if (linenum == 567 && strstr(pos, "Posix/port.c") != NULL) {
        exit(0);
    }

    printf("Assertion failed at %s:%d\n", pos, linenum);
    abort();
}


size_t xPortGetFreeHeapSize(void) {
    return -1;
}


size_t xPortGetMinimumEverFreeHeapSize(void) {
    return 1;
}


int ipFOREVER() {
    static int counts = configMAYHEM_IP_CYCLES;
    printf("ipFOREVER is called\n");
    counts--;
    return counts != 0;
}


void new_network_event(int event) {
    printf("we got a network event: %d\n", event);
}
