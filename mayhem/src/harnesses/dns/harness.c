#include "FreeRTOS_IP.h"
#include "FreeRTOS_DNS_Parser.h"
#include <stdio.h>

int mayhem_process_input(char *buf, size_t len) {
    if (len < sizeof(DNSMessage_t)) {
        printf("dropping a short package\n");
        return -1;
    }
    struct freertos_addrinfo *addrs = NULL;
    uint32_t addr = DNS_ParseDNSReply(buf, len, &addrs, 0, 8080);
    printf("parsed dns reply, the address is: %d\n", addr);
}
