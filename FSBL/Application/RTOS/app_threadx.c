
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "systimer.h"

#include "app_threadx.h"
#include "ux_device_cdc_acm.h"
#include "usbd_interface.h"

#include "app_netxduo.h"
#include "ai_app.h"

//****************** USB TASK *************
// USB 2.0 TX speed is 43 MB. Increasing buffer size decreases task loop time so dont go below 8192*2 which has 2.63k taks loop time
// 8192*4 buffer size makes task loop time 1.3khz with 43Mb throughput
#define USB_TX_BUFFER_SIZE 8192*4

__attribute__((aligned(32)))
static UCHAR usb_tx_buffer[USB_TX_BUFFER_SIZE];

static TX_THREAD usb_tx_thread;
static UCHAR usb_tx_stack[2048];
//static VOID USB_TX_Thread1(ULONG arg);
static VOID USB_TX_Thread(ULONG arg);

extern UX_SLAVE_CLASS_CDC_ACM *cdc_acm;

volatile uint32_t usb_task_counter = 0;
volatile ULONG usb_actual_length = 0;
volatile UINT usb_write_status = 0;

//****************** LED 1 TASK *************
static TX_THREAD led_thread;
static UCHAR led_stack[1024];
static VOID LED_Thread(ULONG arg);
volatile uint32_t led_task_counter = 0;

//****************** AI TASK *************
static TX_THREAD ai_thread;
static UCHAR ai_stack[8192];
static VOID Camera_Thread(ULONG arg);
static VOID AI_Thread(ULONG arg);


//****************** AI TASK *************
static TX_THREAD lcd_text_thread;
static UCHAR lcd_text_stack[2048];
static VOID LCD_Text_Thread(ULONG arg);

#include "app_postprocess.h"
#include "camera_app.h"
#include "lcd_app.h"
#include "stm32_lcd.h"

volatile int32_t cameraFrameReceived = 0;
volatile uint32_t ai_task_counter = 0;
volatile int32_t ai_face_count = 0;
volatile uint32_t ai_result_ready = 0;
volatile uint32_t lcd_result_ready = 0;

/***********ETHERNET Task********/
#define ETHERNET_THREAD_STACK_SIZE 4096

static TX_THREAD ethernet_thread;
static UCHAR ethernet_stack[ETHERNET_THREAD_STACK_SIZE];
static VOID Ethernet_Thread(ULONG thread_input);
#define TCP_PORT 5000
#define TCP_BUFFER_SIZE 1536
UCHAR tx_data[1400];

UINT App_ThreadX_Init(VOID *memory_ptr)
{
    UINT ret;

    UX_PARAMETER_NOT_USED(memory_ptr);

    for (uint32_t i = 0; i < USB_TX_BUFFER_SIZE; i++)
    {
        usb_tx_buffer[i] = (UCHAR)i;
    }

    // CPU wrote the data so it is in cache. Clean cache -> ram so usb dma can transfer the correct data
    SCB_CleanDCache_by_Addr((uint32_t *)usb_tx_buffer, USB_TX_BUFFER_SIZE);

    ret = tx_thread_create(&usb_tx_thread, "USB TX", USB_TX_Thread, 0, usb_tx_stack, sizeof(usb_tx_stack), 15, 15, 1, TX_AUTO_START);
    if (ret != TX_SUCCESS) return ret;

    ret = tx_thread_create(&ethernet_thread, "ETHERNET", Ethernet_Thread, 0, ethernet_stack, sizeof(ethernet_stack), 15, 15, 1, TX_AUTO_START);
    if (ret != TX_SUCCESS) return ret;

    ret = tx_thread_create(&led_thread, "LED", LED_Thread, 0, led_stack, sizeof(led_stack), 20, 20, 1, TX_AUTO_START);
    if (ret != TX_SUCCESS) return ret;

    ret = tx_thread_create(&ai_thread, "AI", AI_Thread, 0, ai_stack, sizeof(ai_stack), 10, 10, 1, TX_AUTO_START);
    if (ret != TX_SUCCESS) return ret;

    ret = tx_thread_create(&lcd_text_thread, "LCD TEXT", LCD_Text_Thread, 0, lcd_text_stack, sizeof(lcd_text_stack), 15, 15, 1, TX_AUTO_START);
    if (ret != TX_SUCCESS) return ret;

    return TX_SUCCESS;
}

void MX_ThreadX_Init(void)
{
    tx_kernel_enter();
}

// This one works with fsbl loading.
// This one does not work with self debug since it does not have AI update codes
//static VOID Camera_Thread(ULONG arg)
//{
//    UX_PARAMETER_NOT_USED(arg);
//
//    while (1)
//    {
//        if (cameraFrameReceived == 0)
//        {
//            tx_thread_sleep(1);
//            continue;
//        }
//
//        CK_USBD_Println("Camera Thread entered");
//
//        cameraFrameReceived = 0;
//
//        CameraPipeline_IspUpdate();
//
//        CameraPipeline_NNPipe_Start((uint8_t *)nn_in, DCMIPP_MODE_SNAPSHOT);
//
//        HAL_GPIO_TogglePin(LED_RED_GPIO_Port, LED_RED_Pin);
//    }
//}

static VOID AI_Thread(ULONG arg)
{
    UX_PARAMETER_NOT_USED(arg);

    while (1)
    {

        if (cameraFrameReceived == 0)
        {
            tx_thread_sleep(1);
            continue;
        }

        cameraFrameReceived = 0;

        AI_Run(); // Kameradan alınan görüntüyü modelden geçir

        app_postprocess_run((void **)nn_out, number_output, &pp_output, &pp_params);	// Ham NN çıktısını gerçek yüz detection sonucuna çevir

        if (pp_output.nb_detect > 0){

        	ai_face_count = pp_output.nb_detect;

        	Face_Crop(&pp_output.pOutBuff[0]);
        }

        CK_USBD_Print("ai_face_count: ");
        CK_USBD_IntPrintln(ai_face_count);

        ai_task_counter++;

        ai_result_ready = 1;
        lcd_result_ready = 1;

        // Sonraki kamera snapshot'ını başlat
        CameraPipeline_IspUpdate();

        CameraPipeline_NNPipe_Start((uint8_t *)nn_in, DCMIPP_MODE_SNAPSHOT);

        HAL_GPIO_TogglePin(LED_RED_GPIO_Port, LED_RED_Pin);

    }
}

static VOID LCD_Text_Thread(ULONG arg)
{
    UX_PARAMETER_NOT_USED(arg);

    char text_buffer[64];

    while (1)
    {
        if (lcd_result_ready == 0)
        {
            tx_thread_sleep(1);
            continue;
        }

        UTIL_LCD_Clear(0x00000000);
        UTIL_LCD_SetFont(&Font16);
        UTIL_LCD_SetTextColor(UTIL_LCD_COLOR_LIGHTGREEN);

        if (pp_output.nb_detect)
        {
            for (int i = 0; i < pp_output.nb_detect; i++)
            {
                float xc = pp_output.pOutBuff[i].x_center * SCREEN_WIDTH;
                float yc = pp_output.pOutBuff[i].y_center * SCREEN_HEIGHT;
                float w = pp_output.pOutBuff[i].width * SCREEN_WIDTH;
                float h = pp_output.pOutBuff[i].height * SCREEN_HEIGHT;

                int32_t x = (int32_t)(xc - (w / 2.0f));
                int32_t y = (int32_t)(yc - (h / 2.0f));

                if (x < 0) x = 0;
                if (y < 0) y = 0;
                if ((x + (int32_t)w) > SCREEN_WIDTH) w = SCREEN_WIDTH - x;
                if ((y + (int32_t)h) > SCREEN_HEIGHT) h = SCREEN_HEIGHT - y;

                UTIL_LCD_DrawRect(x, y, (uint32_t)w, (uint32_t)h, UTIL_LCD_COLOR_LIGHTGREEN);

                snprintf(text_buffer, sizeof(text_buffer), "Face %d %.2f", i + 1, pp_output.pOutBuff[i].conf);
                UTIL_LCD_DisplayStringAt(x, (y > 20) ? y - 20 : y, (uint8_t *)text_buffer, LEFT_MODE);
            }
        }
        else
        {
            UTIL_LCD_DisplayStringAt(10, 10, (uint8_t *)"No face detected", LEFT_MODE);
        }

        SCB_CleanDCache_by_Addr((uint32_t *)lcd_fg_buffer[0], LCD_FG_FRAMEBUFFER_SIZE);

        lcd_result_ready = 0;
    }
}


//// TX ONLY TEST
static VOID Ethernet_Thread(ULONG thread_input)
{
    UINT status;

    (void)thread_input;

    status = NetXDuo_TCP_Server_Start(TCP_PORT);
    if (status != NX_SUCCESS)
        return;

    while (1)
    {
    	CK_USBD_Println("Waiting TCP client...\r\n");

        status = NetXDuo_TCP_Accept();

        if (status != NX_SUCCESS)
            continue;

        for (int i = 0; i < 1400; i++)
        {
            tx_data[i] = (UCHAR)i;
        }

        while (1)
        {

            status = NetXDuo_TCP_Send(tx_data, 1400);

            if (status != NX_SUCCESS)
            {
            	CK_USBD_Println("TCP send error...\r\n");
                break;
            }
        }

        NetXDuo_TCP_Disconnect();
    }
}

// USB Throughput test function
static VOID USB_TX_Thread(ULONG arg)
{
    UINT status;

    UX_PARAMETER_NOT_USED(arg);

    while (1)
    {
        if (cdc_acm == UX_NULL)
        {
            tx_thread_sleep(1);
            continue;
        }

        status = CK_USBD_BufferSend(usb_tx_buffer, USB_TX_BUFFER_SIZE);

        if (status == UX_SUCCESS)
        {
            usb_task_counter++;
            HAL_GPIO_TogglePin(GPIOE, GPIO_PIN_7);
        }
        else
        {
            tx_thread_sleep(1);
        }
    }
}

//static VOID USB_TX_Thread(ULONG arg)
//{
//    UX_PARAMETER_NOT_USED(arg);
//
//    while (1)
//    {
//        if (CK_USBD_TxAvailable()) CK_USBD_Send();
//        tx_thread_sleep(1);
//    }
//}

static VOID LED_Thread(ULONG arg)
{
    UX_PARAMETER_NOT_USED(arg);

    while (1)
    {
        led_task_counter++;
        HAL_GPIO_TogglePin(LED_GREEN_GPIO_Port, LED_GREEN_Pin);
        tx_thread_sleep(25);

    }
}
