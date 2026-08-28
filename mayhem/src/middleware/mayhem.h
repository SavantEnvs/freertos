#include "FreeRTOS_IP.h"

int init_network_buffer(NetworkBufferDescriptor_t *net, uint8_t *buf, size_t len);
void release_network_buffer(NetworkBufferDescriptor_t *net);
NetworkInterface_t *create_mayhem_interface(void);
