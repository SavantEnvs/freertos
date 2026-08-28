#include "FreeRTOS_IP.h"
#include "FreeRTOS_IP_Private.h"
#include "FreeRTOS_ARP.h"

#include "mayhem.h"

int mayhem_process_input(char *buf, size_t len) {
    if (len < sizeof(ARPPacket_t)) {
        printf("dropping a short package\n");
        return -1;
    }
    NetworkBufferDescriptor_t net = {};
    init_network_buffer(&net, buf, len);
    eFrameProcessingResult_t r = eARPProcessPacket(&net);
    printf("processed %d bytes as an arp packet, outcome %d\n", len, r);
}
