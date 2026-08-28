#include "FreeRTOS_IP.h"
#include "FreeRTOS_TCP_IP.h"
#include "mayhem.h"
#include <stdio.h>

int mayhem_process_input(char *buf, size_t len) {
    if (len < sizeof(TCPPacket_t)) {
        printf("ignoring short packet\n");
        return -1;
    }
    vListInitialise(&xBoundTCPSocketsList);
    NetworkBufferDescriptor_t net = {};
    init_network_buffer(&net, buf, len);
    int r = xProcessReceivedTCPPacket(&net);
    printf("processed %d-long packet as a tcp packet: %d\n", len, r);
}
