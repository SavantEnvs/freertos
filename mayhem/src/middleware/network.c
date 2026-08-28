/* defines all the interface and endpoint stuff */
#include "FreeRTOS_Routing.h"
#include <stdlib.h>
#include <string.h>


const uint8_t ucDefaultIPAddress_IPv4[ ipIP_ADDRESS_LENGTH_BYTES ] = { 192, 168, 123, 223 };
const uint8_t ucDefaultNetMask_IPv4[ ipIP_ADDRESS_LENGTH_BYTES ] = { 255, 255, 255, 0 };
const uint8_t ucDefaultGatewayAddress_IPv4[ ipIP_ADDRESS_LENGTH_BYTES ] = { 192, 168, 123, 254 };
const uint8_t ucDefaultDNSServerAddress_IPv4[ ipIP_ADDRESS_LENGTH_BYTES ] = { 192, 168, 123, 1 };
const uint8_t ucDefaultMACAddress_IPv4[ ipMAC_ADDRESS_LENGTH_BYTES ] = { 0xab, 0xcd, 0xef, 0x11, 0x22, 0x33 };

/* Default IPv6 address is set to 2001::1 */
const IPv6_Address_t xDefaultIPAddress_IPv6 = { 0x20, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01 };
/* Default IPv6 address is set to 2001:: */
const IPv6_Address_t xDefaultNetPrefix_IPv6 = { 0x20, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
const size_t xDefaultPrefixLength = 64U;
/* Default IPv6 address is set to 2001::fffe */
const IPv6_Address_t xDefaultGatewayAddress_IPv6 = { 0x20, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xfe };
/* Default IPv6 address is set to 2001::ffee */
const IPv6_Address_t xDefaultDNSServerAddress_IPv6 = { 0x20, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xee };

const uint8_t ucDefaultMACAddress_IPv6[ ipMAC_ADDRESS_LENGTH_BYTES ] = { 0x11, 0x22, 0x33, 0xab, 0xcd, 0xef };

void prvProcessEthernetPacket(NetworkBufferDescriptor_t * const pxNetworkBuffer );

BaseType_t mayhem_interface_initialize ( struct xNetworkInterface *i) {
    return pdPASS;
}

BaseType_t mayhem_interface_output(
    struct xNetworkInterface *ni,
    NetworkBufferDescriptor_t * const pxNetworkBuffer,
    BaseType_t xReleaseAfterSend) {
    return pdTRUE;
}

BaseType_t mayhem_interface_get_link_status(struct xNetworkInterface *i) {
    return pdTRUE;
}

void mayhem_interface_manipulate_mac_address(
    struct xNetworkInterface * pxInterface,
    const uint8_t * pucMacAddressBytes) {
    return;
}

NetworkInterface_t *create_mayhem_interface() {
    NetworkInterface_t *ni = calloc(1, sizeof(NetworkInterface_t));
    if (ni == NULL) {
        return ni;
    }
    ni->pcName = "mayhem";
    ni->pvArgument = NULL;
    ni->pfInitialise = mayhem_interface_initialize;
    ni->pfOutput = mayhem_interface_output;
    ni->pfGetPhyLinkStatus = mayhem_interface_get_link_status;
    ni->pfAddAllowedMAC = mayhem_interface_manipulate_mac_address;
    ni->pfRemoveAllowedMAC = mayhem_interface_manipulate_mac_address;
    NetworkEndPoint_t *ep = calloc(1, sizeof(NetworkEndPoint_t));
    if (ep == NULL) {
        return NULL;
    }
    FreeRTOS_FillEndPoint(ni,
                          ep,
                          ucDefaultIPAddress_IPv4,
                          ucDefaultNetMask_IPv4,
                          ucDefaultGatewayAddress_IPv4,
                          ucDefaultDNSServerAddress_IPv4,
                          ucDefaultMACAddress_IPv4 );
    FreeRTOS_FillEndPoint_IPv6(ni,
                               ep,
                               &xDefaultIPAddress_IPv6,
                               &xDefaultNetPrefix_IPv6,
                               xDefaultPrefixLength,
                               &xDefaultGatewayAddress_IPv6,
                               &xDefaultDNSServerAddress_IPv6,
                               ucDefaultMACAddress_IPv6 );
/*     ep->bits.bIsDefault = 1; */
/*     ep->bits.bWantDHCP = 1; */
/* #if (ipconfigUSE_RA == 1) */
/*     ep->bits.bWantRA = 1; */
/* #endif */
/*     ep->bits.bIPv6 = 1; */
/*     ep->bits.bEndPointUp = 1; */
    ni->pxEndPoint = ep;
    return FreeRTOS_AddNetworkInterface(ni);
}

int init_network_buffer(NetworkBufferDescriptor_t *net, uint8_t *buf, size_t len) {
    net->pxInterface = create_mayhem_interface();
    net->pxEndPoint = net->pxInterface != NULL ? net->pxInterface->pxEndPoint : NULL;
    /* The real BufferAllocation schemes reserve ipBUFFER_PADDING bytes ahead of
     * pucEthernetBuffer: the first sizeof(void*) bytes hold a back-pointer to the
     * descriptor and the receive path stores the IP-type marker (0x40/0x60) just
     * below the buffer. Payload<->descriptor conversions
     * (pxUDPPayloadBuffer_to_NetworkBuffer / prvPacketBuffer_to_NetworkBuffer) read
     * that headroom, so a bare exact-size buffer makes them read out of bounds.
     * Replicate the layout here so those conversions resolve inside the allocation. */
    uint8_t *alloc = calloc(1, ipBUFFER_PADDING + len);
    if (alloc == NULL) {
        return -1;
    }
    net->pucEthernetBuffer = alloc + ipBUFFER_PADDING;
    *((NetworkBufferDescriptor_t **) alloc) = net;
    memcpy(net->pucEthernetBuffer, buf, len);
    net->xDataLength = len;
    /* Mark the IP type in the padding, mirroring the receive path. The UDP-payload
     * conversion reads it ipUDP_PAYLOAD_IP_TYPE_OFFSET (48) bytes below the UDP
     * payload; for an IPv4 UDP frame that lands inside the padding, so set it. */
    if (len >= ipSIZE_OF_ETH_HEADER) {
        const EthernetHeader_t *eth = (const EthernetHeader_t *) net->pucEthernetBuffer;
        uint8_t marker = (eth->usFrameType == ipIPv6_FRAME_TYPE)
                             ? (uint8_t) ipTYPE_IPv6
                             : (uint8_t) ipTYPE_IPv4;
        for (size_t i = sizeof(NetworkBufferDescriptor_t *); i < ipBUFFER_PADDING; i++) {
            alloc[i] = marker;
        }
    }
    if (net->pxInterface == NULL && net->pxEndPoint == NULL) {
        return -1;
    }
    return 0;
}

void release_network_buffer(NetworkBufferDescriptor_t *net) {
    free(net->pxInterface);
    free(net->pxEndPoint);
    if (net->pucEthernetBuffer != NULL) {
        free(net->pucEthernetBuffer - ipBUFFER_PADDING);
    }
}
