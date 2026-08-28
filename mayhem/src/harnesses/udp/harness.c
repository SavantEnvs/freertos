#include "FreeRTOS_IP.h"
#include "FreeRTOS_UDP_IP.h"
#include "mayhem.h"
#include <stdio.h>

int mayhem_process_input(char *buf, size_t len) {
    if (len < sizeof(UDPPacket_t)) {
        printf("ignoring short packet\n");
        return -1;
    }
    IPPacket_t *ip = ( IPPacket_t * )buf;

    if (ip->xEthernetHeader.usFrameType != ipIPv4_FRAME_TYPE &&
        ip->xEthernetHeader.usFrameType != ipIPv6_FRAME_TYPE) {
        printf("discarding non-ip packet\n");
        return 1;
    }
    NetworkBufferDescriptor_t net = {};
    init_network_buffer(&net, buf, len);
    BaseType_t wait_arp;
    int r = xProcessReceivedUDPPacket(&net, 4242, &wait_arp);
    printf("processed %d-long packet as a udp packet: %d\n", len, r);
    release_network_buffer(&net);
}
