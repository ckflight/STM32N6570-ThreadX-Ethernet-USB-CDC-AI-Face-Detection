#ifndef __APP_NETXDUO_H__
#define __APP_NETXDUO_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "nx_api.h"
#include "nxd_dhcp_client.h"
#include "nx_stm32_eth_driver.h"

#define NETX_PACKET_PAYLOAD_SIZE 	1536
#define NETX_PACKET_POOL_SIZE 		((NETX_PACKET_PAYLOAD_SIZE + sizeof(NX_PACKET)) * 20)
#define NETX_IP_THREAD_STACK_SIZE 	2048
#define NETX_IP_THREAD_PRIORITY 	5
#define NETX_ARP_CACHE_SIZE 		1024
#define NETX_TCP_WINDOW_SIZE 		8192

UINT MX_NetXDuo_Init(VOID *memory_ptr);
UINT NetXDuo_DHCP_Wait(void);
UINT NetXDuo_TCP_Server_Start(UINT port);
UINT NetXDuo_TCP_Accept(void);
UINT NetXDuo_TCP_Receive_Packet(NX_PACKET **packet);
UINT NetXDuo_TCP_Receive(UCHAR *buffer, ULONG buffer_size, ULONG *received);

UINT NetXDuo_TCP_Get_TX_Buffer(NX_PACKET **packet, UCHAR **tx_data, ULONG *capacity);
UINT NetXDuo_TCP_Send_ZeroCopy(NX_PACKET *packet, ULONG length);

UINT NetXDuo_TCP_Send(UCHAR *data, ULONG length);
VOID NetXDuo_TCP_Disconnect(void);

extern NX_IP NetXDuoEthIpInstance;
extern NX_PACKET_POOL NxAppPool;

#ifdef __cplusplus
}
#endif

#endif
