#include "app_netxduo.h"
#include "main.h"
#include <stdio.h>

NX_PACKET_POOL NxAppPool;
NX_IP NetXDuoEthIpInstance;

static NX_DHCP DHCPClient;
static TX_SEMAPHORE DHCPSemaphore;
static NX_TCP_SOCKET TcpSocket;

static ULONG IpAddress;
static ULONG NetMask;
static UINT TcpPort;

static VOID NetXDuo_IP_Change_Callback(NX_IP *ip_instance, VOID *ptr);

UINT MX_NetXDuo_Init(VOID *memory_ptr)
{
    UINT status;
    CHAR *memory;
    TX_BYTE_POOL *byte_pool = (TX_BYTE_POOL *)memory_ptr;

    nx_system_initialize();

    status = tx_byte_allocate(byte_pool, (VOID **)&memory, NETX_PACKET_POOL_SIZE, TX_NO_WAIT);
    if (status != TX_SUCCESS) return status;

    status = nx_packet_pool_create(&NxAppPool, "NetX Packet Pool", NETX_PACKET_PAYLOAD_SIZE, memory, NETX_PACKET_POOL_SIZE);
    if (status != NX_SUCCESS) return status;

    status = tx_byte_allocate(byte_pool, (VOID **)&memory, NETX_IP_THREAD_STACK_SIZE, TX_NO_WAIT);
    if (status != TX_SUCCESS) return status;

    status = nx_ip_create(&NetXDuoEthIpInstance, "NetX IP", 0, 0, &NxAppPool, nx_stm32_eth_driver, memory, NETX_IP_THREAD_STACK_SIZE, NETX_IP_THREAD_PRIORITY);
    if (status != NX_SUCCESS) return status;

    status = tx_byte_allocate(byte_pool, (VOID **)&memory, NETX_ARP_CACHE_SIZE, TX_NO_WAIT);
    if (status != TX_SUCCESS) return status;

    status = nx_arp_enable(&NetXDuoEthIpInstance, memory, NETX_ARP_CACHE_SIZE);
    if (status != NX_SUCCESS) return status;

    status = nx_icmp_enable(&NetXDuoEthIpInstance);
    if (status != NX_SUCCESS) return status;

    status = nx_tcp_enable(&NetXDuoEthIpInstance);
    if (status != NX_SUCCESS) return status;

    status = nx_udp_enable(&NetXDuoEthIpInstance);
    if (status != NX_SUCCESS) return status;

    status = tx_semaphore_create(&DHCPSemaphore, "DHCP Semaphore", 0);
    if (status != TX_SUCCESS) return status;

    status = nx_ip_address_change_notify(&NetXDuoEthIpInstance, NetXDuo_IP_Change_Callback, NX_NULL);
    if (status != NX_SUCCESS) return status;

    status = nx_dhcp_create(&DHCPClient, &NetXDuoEthIpInstance, "DHCP Client");
    if (status != NX_SUCCESS) return status;

    printf("NetX initialized\r\n");

    return NX_SUCCESS;
}

UINT NetXDuo_DHCP_Wait(void)
{
    UINT status;

    printf("Looking for DHCP server...\r\n");

    status = nx_dhcp_start(&DHCPClient);
    if (status != NX_SUCCESS) return status;

    status = tx_semaphore_get(&DHCPSemaphore, TX_WAIT_FOREVER);
    if (status != TX_SUCCESS) return status;

    status = nx_ip_address_get(&NetXDuoEthIpInstance, &IpAddress, &NetMask);
    if (status != NX_SUCCESS) return status;

    printf("STM32 IP: %lu.%lu.%lu.%lu\r\n", (IpAddress >> 24) & 0xFF, (IpAddress >> 16) & 0xFF, (IpAddress >> 8) & 0xFF, IpAddress & 0xFF);

    return NX_SUCCESS;
}

UINT NetXDuo_TCP_Server_Start(UINT port)
{
    UINT status;

    TcpPort = port;

    status = nx_tcp_socket_create(&NetXDuoEthIpInstance, &TcpSocket, "TCP Server", NX_IP_NORMAL, NX_FRAGMENT_OKAY, NX_IP_TIME_TO_LIVE, NETX_TCP_WINDOW_SIZE, NX_NULL, NX_NULL);
    if (status != NX_SUCCESS) return status;

    status = nx_tcp_server_socket_listen(&NetXDuoEthIpInstance, TcpPort, &TcpSocket, 1, NX_NULL);
    if (status != NX_SUCCESS) return status;

    printf("TCP server listening on port %u\r\n", TcpPort);

    return NX_SUCCESS;
}

UINT NetXDuo_TCP_Receive_Packet(NX_PACKET **packet)
{
    return nx_tcp_socket_receive(&TcpSocket, packet, TX_WAIT_FOREVER);
}

UINT NetXDuo_TCP_Accept(void)
{
    UINT status;

    status = nx_tcp_server_socket_accept(&TcpSocket, TX_WAIT_FOREVER);

    if (status == NX_SUCCESS) printf("TCP client connected\r\n");

    return status;
}

UINT NetXDuo_TCP_Receive(UCHAR *buffer, ULONG buffer_size, ULONG *received)
{
    UINT status;
    NX_PACKET *packet;

    *received = 0;

    status = nx_tcp_socket_receive(&TcpSocket, &packet, TX_WAIT_FOREVER);
    if (status != NX_SUCCESS) return status;

    status = nx_packet_data_extract_offset(packet, 0, buffer, buffer_size, received);

    nx_packet_release(packet);

    return status;
}

UINT NetXDuo_TCP_Get_TX_Buffer(NX_PACKET **packet, UCHAR **tx_data, ULONG *capacity)
{
    UINT status;

    // Gives ethernet packet in NetX packet format with header etc to put data in it
    status = nx_packet_allocate(&NxAppPool, packet, NX_TCP_PACKET, TX_WAIT_FOREVER);

    if (status != NX_SUCCESS)
        return status;

    // Assign NetX packets payload address to the tx data
    *tx_data = (*packet)->nx_packet_append_ptr;

    *capacity = (ULONG)((*packet)->nx_packet_data_end - (*packet)->nx_packet_append_ptr);

    return NX_SUCCESS;
}

UINT NetXDuo_TCP_Send_ZeroCopy(NX_PACKET *packet, ULONG length)
{
    UINT status;

    if (length > (ULONG)(packet->nx_packet_data_end - packet->nx_packet_append_ptr))
    {
        nx_packet_release(packet);
        return NX_SIZE_ERROR;
    }

    packet->nx_packet_append_ptr += length;
    packet->nx_packet_length += length;

    status = nx_tcp_socket_send(&TcpSocket, packet, TX_WAIT_FOREVER);

    if (status != NX_SUCCESS)
    {
        nx_packet_release(packet);
    }

    return status;
}

UINT NetXDuo_TCP_Send(UCHAR *data, ULONG length)
{
    UINT status;
    NX_PACKET *packet;

    // Gives ethernet packet in NetX packet format with header etc to put data in it
    status = nx_packet_allocate(&NxAppPool, &packet, NX_TCP_PACKET, TX_WAIT_FOREVER);
    if (status != NX_SUCCESS) return status;

    // Put data into the packet
    status = nx_packet_data_append(packet, data, length, &NxAppPool, TX_WAIT_FOREVER);

    if (status != NX_SUCCESS)
    {
        nx_packet_release(packet);
        return status;
    }

    // Send the data with also adding tcp headers to the packet
    status = nx_tcp_socket_send(&TcpSocket, packet, TX_WAIT_FOREVER);

    if (status != NX_SUCCESS)
    {
        nx_packet_release(packet);
        return status;
    }

    return NX_SUCCESS;
}

VOID NetXDuo_TCP_Disconnect(void)
{
    nx_tcp_socket_disconnect(&TcpSocket, NX_NO_WAIT);
    nx_tcp_server_socket_unaccept(&TcpSocket);
    nx_tcp_server_socket_relisten(&NetXDuoEthIpInstance, TcpPort, &TcpSocket);

    printf("TCP client disconnected\r\n");
}

static VOID NetXDuo_IP_Change_Callback(NX_IP *ip_instance, VOID *ptr)
{
    ULONG address;
    ULONG mask;

    (void)ip_instance;
    (void)ptr;

    if (nx_ip_address_get(&NetXDuoEthIpInstance, &address, &mask) != NX_SUCCESS) return;

    if (address != 0) tx_semaphore_put(&DHCPSemaphore);
}
