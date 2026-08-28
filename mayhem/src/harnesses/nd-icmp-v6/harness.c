/* Include standard libraries */
#include <stdio.h>
#include "FreeRTOS_ICMP.h"
#include "FreeRTOS_IPv6_Private.h"
#include "mayhem.h"

int mayhem_process_input(char *buf, size_t len) {
    if (len < sizeof(IPPacket_IPv6_t)) {
        printf("the packet is too small\n");
        return 1;
    }
    printf("%d+%d+%d=%d\n",
           sizeof(EthernetHeader_t),
           sizeof(IPHeader_IPv6_t),
           sizeof(ICMPHeader_IPv6_t),
           sizeof(ICMPPacket_IPv6_t));
    printf(
        "len=%d, ipv6=%d, icmpv6=%d\n", len,
        sizeof(IPPacket_IPv6_t), sizeof(ICMPPacket_IPv6_t)
        );
    IPPacket_t *ip = ( IPPacket_t * )buf;

    if (ip->xEthernetHeader.usFrameType != ipIPv4_FRAME_TYPE &&
        ip->xEthernetHeader.usFrameType != ipIPv6_FRAME_TYPE) {
        printf("discarding non-ip packet");
        return 1;
    }

    NetworkBufferDescriptor_t net = {};
    init_network_buffer(&net, buf, len);
    eFrameProcessingResult_t r = prvProcessICMPMessage_IPv6(&net);
    printf("processed ICMPv6 packet with the following outcome: %d\n", r);
}
