
#include "app_azure_rtos.h"
#include "app_threadx.h"
#include "app_filex.h"
#include "app_netxduo.h"

#if (USE_STATIC_ALLOCATION == 1)

/* --------------------------------------------------------- */
/* ThreadX Application Pool                                  */
/* --------------------------------------------------------- */
__ALIGN_BEGIN static UCHAR tx_byte_pool_buffer[TX_APP_MEM_POOL_SIZE] __ALIGN_END;
static TX_BYTE_POOL tx_app_byte_pool;

/* --------------------------------------------------------- */
/* USBX Application Pool                                  */
/* --------------------------------------------------------- */
__attribute__((section(".UsbxPoolSection")))
__ALIGN_BEGIN static UCHAR ux_byte_pool_buffer[UX_APP_MEM_POOL_SIZE] __ALIGN_END;
static TX_BYTE_POOL ux_app_byte_pool;

__ALIGN_BEGIN static UCHAR  usbpd_byte_pool_buffer[USBPD_DEVICE_APP_MEM_POOL_SIZE] __ALIGN_END;
static TX_BYTE_POOL usbpd_app_byte_pool;

/* --------------------------------------------------------- */
/* FileX Pool                                                */
/* --------------------------------------------------------- */
__ALIGN_BEGIN static UCHAR fx_byte_pool_buffer[FX_APP_MEM_POOL_SIZE] __ALIGN_END;
static TX_BYTE_POOL fx_app_byte_pool;

/* --------------------------------------------------------- */
/* NetX Duo Pool                                             */
/* --------------------------------------------------------- */
__attribute__((section(".NetXPoolSection")))
__ALIGN_BEGIN static UCHAR nx_byte_pool_buffer[NX_APP_MEM_POOL_SIZE] __ALIGN_END;
static TX_BYTE_POOL nx_app_byte_pool;

#endif


/**
  * @brief  Define the initial system.
  * @param  first_unused_memory : Pointer to the first unused memory
  * @retval None
  */
VOID tx_application_define(VOID *first_unused_memory)
{

#if (USE_STATIC_ALLOCATION == 1)

	UINT status = TX_SUCCESS;
    (void)first_unused_memory;

    /* ----------------------------------------------------- */
    /* Application ThreadX Pool                              */
    /* ----------------------------------------------------- */
	status = tx_byte_pool_create(&tx_app_byte_pool, "Tx App memory pool", tx_byte_pool_buffer, TX_APP_MEM_POOL_SIZE);

	if(status != TX_SUCCESS){
		Error_Handler();
	}

	/* ----------------------------------------------------- */
	/* Application Thread                                    */
	/* ----------------------------------------------------- */
	status = App_ThreadX_Init((VOID *)&tx_app_byte_pool);

	if (status != TX_SUCCESS)
	{
		Error_Handler();
	}

    /* ----------------------------------------------------- */
    /* Application USBX Pool                                 */
    /* ----------------------------------------------------- */
	status = tx_byte_pool_create(&ux_app_byte_pool, "Ux App memory pool", ux_byte_pool_buffer, UX_APP_MEM_POOL_SIZE);

	if(status != TX_SUCCESS){
		Error_Handler();
	}

    /* ----------------------------------------------------- */
    /* Application USBX Thread                               */
    /* ----------------------------------------------------- */
	status = MX_USBX_Init((VOID *)&ux_app_byte_pool);

	if (status != TX_SUCCESS)
	{
		Error_Handler();
	}

    /* ----------------------------------------------------- */
    /* Application USBPD Pool                                */
    /* ----------------------------------------------------- */
	status = tx_byte_pool_create(&usbpd_app_byte_pool, "USBPD App memory pool", usbpd_byte_pool_buffer, USBPD_DEVICE_APP_MEM_POOL_SIZE);

	if (status != TX_SUCCESS)
	{
		Error_Handler();
	}

    /* ----------------------------------------------------- */
    /* Application USBPD Thread                              */
    /* ----------------------------------------------------- */
	status = MX_USBPD_Init((VOID *)&usbpd_app_byte_pool);

	if (status != USBPD_OK)
	{
		Error_Handler();
	}

    /* ----------------------------------------------------- */
    /* FileX                                                 */
    /* ----------------------------------------------------- */
    // FileX arkada thread oluşturmuyor pool gereksiz ama şimdilik silmiyorum.
    status = tx_byte_pool_create(&fx_app_byte_pool, "FileX Memory Pool", fx_byte_pool_buffer, FX_APP_MEM_POOL_SIZE);

    if (status != TX_SUCCESS)
    {
        Error_Handler();
    }

    status = MX_FileX_Init((VOID *)&fx_app_byte_pool);

    if (status != FX_SUCCESS)
    {
        Error_Handler();
    }


    /* ----------------------------------------------------- */
    /* NetX Duo                                              */
    /* ----------------------------------------------------- */
    // Nex Duo arkada thread oluşturuyor. Benim app_threadx.c kodumda yazdığım threadlerle birlikte bu da çalışıyor.
    status = tx_byte_pool_create(&nx_app_byte_pool, "NetX Memory Pool", nx_byte_pool_buffer, NX_APP_MEM_POOL_SIZE);

    if (status != TX_SUCCESS)
    {
        Error_Handler();
    }

    status = MX_NetXDuo_Init((VOID *)&nx_app_byte_pool);

    if (status != NX_SUCCESS)
    {
        Error_Handler();
    }

#endif

}
