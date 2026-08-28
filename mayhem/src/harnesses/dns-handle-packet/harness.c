#include "FreeRTOS_IP.h"
#include "FreeRTOS_DNS.h"
#include "FreeRTOS_DNS_Parser.h"
#include "mayhem.h"
#include <stdio.h>

int mayhem_process_input(char *buf, size_t len) {
    if (len < sizeof(DNSMessage_t)) {
        printf("dropping a short package\n");
        return -1;
    }
    NetworkBufferDescriptor_t net = {};
    init_network_buffer(&net, buf, len);
    BaseType_t r = xDNSHandlePacket(&net);
    printf("handled dns packet, outcome: %ld\n", (long) r);
}
