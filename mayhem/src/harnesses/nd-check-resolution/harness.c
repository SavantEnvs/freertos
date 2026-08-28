/* Include standard libraries */
#include <stdio.h>
#include "FreeRTOS_ND.h"
#include "mayhem.h"

int mayhem_process_input(char *buf, size_t len) {
    if (len < sizeof(IPPacket_IPv6_t)) {
        printf("the packet is too small\n");
        return 1;
    }
    IPPacket_t *ip = ( IPPacket_t * )buf;

    if (ip->xEthernetHeader.usFrameType != ipIPv6_FRAME_TYPE) {
        printf("and ipv6 packet is required\n");
        return 1;
    }

    NetworkBufferDescriptor_t net = {};
    init_network_buffer(&net, buf, len);
    BaseType_t r = xCheckRequiresNDResolution(&net);
    printf("processed ipv6 packet with the following outcome: %d\n", r);
}
