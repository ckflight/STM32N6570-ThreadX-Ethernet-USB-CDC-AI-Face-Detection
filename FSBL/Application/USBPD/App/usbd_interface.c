#include "ux_api.h"
#include "ux_device_class_cdc_acm.h"
#include "stm32n6xx.h"
#include "usbd_interface.h"

#include <stdio.h>
#include <string.h>

extern UX_SLAVE_CLASS_CDC_ACM *cdc_acm;


/* -------------------------------------------------------------------------- */
/* Buffers                                                                    */
/* -------------------------------------------------------------------------- */

static uint8_t tx_buffer[CK_USB_BUFFER_SIZE] __attribute__((aligned(32)));
static uint8_t rx_buffer[CK_USB_BUFFER_SIZE];

static volatile uint32_t tx_length = 0;
static volatile uint32_t rx_write = 0;
static volatile uint32_t rx_read = 0;


/* -------------------------------------------------------------------------- */
/* Init                                                                       */
/* -------------------------------------------------------------------------- */

void CK_USBD_Init(void)
{
    tx_length = 0;
    rx_write = 0;
    rx_read = 0;
}


/* -------------------------------------------------------------------------- */
/* RX                                                                         */
/* -------------------------------------------------------------------------- */

int CK_USBD_Available(void)
{
    return (rx_write != rx_read);
}

int CK_USBD_Read(uint8_t *data)
{
    if (data == NULL)
        return 0;

    if (rx_write == rx_read)
        return 0;

    *data = rx_buffer[rx_read];

    rx_read++;

    if (rx_read >= CK_USB_BUFFER_SIZE)
        rx_read = 0;

    return 1;
}

void CK_USBD_RxCallback(uint8_t *data, uint32_t length)
{
    for (uint32_t i = 0; i < length; i++)
    {
        uint32_t next = rx_write + 1;

        if (next >= CK_USB_BUFFER_SIZE)
            next = 0;

        if (next == rx_read)
            break;

        rx_buffer[rx_write] = data[i];
        rx_write = next;
    }
}


/* -------------------------------------------------------------------------- */
/* TX                                                                         */
/* -------------------------------------------------------------------------- */

//int CK_USBD_Write(uint8_t data)
//{
//    if (tx_length >= CK_USB_BUFFER_SIZE)
//        return 0;
//
//    tx_buffer[tx_length++] = data;
//
//    return 1;
//}

volatile uint32_t debug_tx_length = 0;

int CK_USBD_Write(uint8_t data)
{
    if (tx_length >= CK_USB_BUFFER_SIZE)
        return 0;

    tx_buffer[tx_length++] = data;
    debug_tx_length = tx_length;

    return 1;
}

int CK_USBD_TxAvailable(void)
{
    return (tx_length > 0);
}

int CK_USBD_Send(void)
{
    ULONG actual_length;
    UINT status;

    if (cdc_acm == UX_NULL)
        return 0;

    if (tx_length == 0)
        return 1;

    status = ux_device_class_cdc_acm_write(cdc_acm, tx_buffer, tx_length, &actual_length);

    if (status == UX_SUCCESS)
    {
        tx_length = 0;
        return 1;
    }

    return 0;
}

int CK_USBD_BufferSend(uint8_t *buffer, uint32_t length)
{
    ULONG actual_length;
    UINT status;

    if (cdc_acm == UX_NULL)
        return -1;

    status = ux_device_class_cdc_acm_write(cdc_acm, buffer, length, &actual_length);

    return (status == UX_SUCCESS) ? 0 : -1;
}


/* -------------------------------------------------------------------------- */
/* String                                                                     */
/* -------------------------------------------------------------------------- */

int CK_USBD_Print(const char *str)
{
    if (str == NULL)
        return 0;

    while (*str)
    {
        if (!CK_USBD_Write((uint8_t)*str))
            return 0;

        str++;
    }

    return 1;
}

int CK_USBD_Println(const char *str)
{
    if (!CK_USBD_Print(str))
        return 0;

    if (!CK_USBD_Write('\r'))
        return 0;

    if (!CK_USBD_Write('\n'))
        return 0;

    return 1;
}


/* -------------------------------------------------------------------------- */
/* Integer                                                                    */
/* -------------------------------------------------------------------------- */

int CK_USBD_IntPrint(int32_t num)
{
    char buffer[16];

    snprintf(buffer, sizeof(buffer), "%ld", (long)num);

    return CK_USBD_Print(buffer);
}

int CK_USBD_IntPrintln(int32_t num)
{
    char buffer[16];

    snprintf(buffer, sizeof(buffer), "%ld", (long)num);

    return CK_USBD_Println(buffer);
}


/* -------------------------------------------------------------------------- */
/* Float                                                                      */
/* -------------------------------------------------------------------------- */

int CK_USBD_FloatPrint(float num)
{
    char buffer[32];

    snprintf(buffer, sizeof(buffer), "%.2f", (double)num);

    return CK_USBD_Print(buffer);
}

int CK_USBD_FloatPrintln(float num)
{
    char buffer[32];

    snprintf(buffer, sizeof(buffer), "%.2f", (double)num);

    return CK_USBD_Println(buffer);
}
