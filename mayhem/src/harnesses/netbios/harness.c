#include "FreeRTOS_IP.h"
#include "FreeRTOS_DNS.h"
#include <stdio.h>
#include "mayhem.h"

int mayhem_process_input(char *buf, size_t len) {
    if (len <= sizeof(UDPPacket_t)) {
        printf("ignoring a too short packet\n");
        return -1;
    }
    IPPacket_t *ip = ( IPPacket_t * )buf;

    if (ip->xEthernetHeader.usFrameType != ipIPv4_FRAME_TYPE &&
        ip->xEthernetHeader.usFrameType != ipIPv6_FRAME_TYPE) {
        printf("discarding non-ip packet");
        return 1;
    }

    NetworkBufferDescriptor_t net = {};
    init_network_buffer(&net, buf, len);
    int r = ulNBNSHandlePacket(&net);
    printf("parsed a netbios packet: %d\n", r);
}
