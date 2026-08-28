#include "FreeRTOS_IP.h"
#include "FreeRTOS_IP_Private.h"
#include "NetworkBufferManagement.h"
#include "mayhem.h"

#include <stdlib.h>
#include <stdio.h>
#include <err.h>
#include <unistd.h>

NetworkBufferDescriptor_t *net;
NetworkInterface_t *iface;

int mayhem_init() {
    printf("calling init\n");
    xNetworkBuffersInitialise();
    iface = create_mayhem_interface();
    if (iface == NULL) {
        errx(1, "failed to create interface");
    }
    if (FreeRTOS_IPInit_Multi() == pdFALSE) {
        errx(1, "failed to initialize network stack");
    }
    printf("network stack is initialized and ready\n");
    return 0;
}

int mayhem_process_input(char *buf, size_t len) {
    printf("feeding %lu bytes of mayhem\n", len);
    net = pxGetNetworkBufferWithDescriptor(len, 0);
    net->pxInterface = iface;
    net->pxEndPoint = iface->pxEndPoint;
    memcpy(net->pucEthernetBuffer, buf, len);
    net->xDataLength = len;
    IPStackEvent_t ev = { eNetworkRxEvent, (void *) net };
    if (xQueueSendToBack(xNetworkEventQueue, &ev, 0) == pdFALSE) {
        errx(1, "failed to send event to ip stack");
    }
    vTaskStartScheduler();
    vTaskEndScheduler();
    return 0;
}
