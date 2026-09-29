#ifndef CK_USBD_INTERFACE_H
#define CK_USBD_INTERFACE_H

#include <stdint.h>

#define CK_USB_BUFFER_SIZE    8192 * 4

void CK_USBD_Init(void);

int CK_USBD_Send(void);
int CK_USBD_BufferSend(uint8_t *buffer, uint32_t length);
int CK_USBD_Read(uint8_t *data);
int CK_USBD_Available(void);
int CK_USBD_TxAvailable(void);

int CK_USBD_Write(uint8_t data);
int CK_USBD_Print(const char *str);
int CK_USBD_Println(const char *str);

int CK_USBD_IntPrint(int32_t num);
int CK_USBD_IntPrintln(int32_t num);

int CK_USBD_FloatPrint(float num);
int CK_USBD_FloatPrintln(float num);

#endif
