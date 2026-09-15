
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "app_threadx.h"
#include "ux_device_cdc_acm.h"

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
static VOID USB_TX_Thread2(ULONG arg);

static VOID MATH_Thread(ULONG arg);

extern UX_SLAVE_CLASS_CDC_ACM *cdc_acm;

volatile uint32_t usb_task_counter = 0;
volatile ULONG usb_actual_length = 0;
volatile UINT usb_write_status = 0;

//****************** LED 1 TASK *************
static TX_THREAD led1_thread;
static UCHAR led1_stack[1024];
static VOID LED1_Thread(ULONG arg);
volatile uint32_t led1_task_counter = 0;

//****************** LED 2 TASK *************
static TX_THREAD led2_thread;
static UCHAR led2_stack[1024];
static VOID LED2_Thread(ULONG arg);
volatile uint32_t led2_task_counter = 0;

//****************** MATH TASK *************
static TX_THREAD math_thread;
static UCHAR math_stack[2048];
volatile uint32_t math_task_counter = 0;
volatile float math_result = 0.0f;

//****************** AI TASK *************
static TX_THREAD ai_thread;
static UCHAR ai_stack[4096];
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

/**********DEBUG*****************/
volatile uint32_t usb_ready_seen = 0;
volatile uint32_t usb_write_ok = 0;
volatile uint32_t usb_write_err = 0;

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

    ret = tx_thread_create(&usb_tx_thread, "USB TX", USB_TX_Thread2, 0, usb_tx_stack, sizeof(usb_tx_stack), 15, 15, 1, TX_AUTO_START);
    if (ret != TX_SUCCESS) return ret;

    ret = tx_thread_create(&ethernet_thread, "ETHERNET", Ethernet_Thread, 0, ethernet_stack, sizeof(ethernet_stack), 15, 15, 1, TX_AUTO_START);
    if (ret != TX_SUCCESS) return ret;

    ret = tx_thread_create(&led1_thread, "LED1", LED1_Thread, 0, led1_stack, sizeof(led1_stack), 20, 20, 1, TX_AUTO_START);
    if (ret != TX_SUCCESS) return ret;

    ret = tx_thread_create(&led2_thread, "LED2", LED2_Thread, 0, led2_stack, sizeof(led2_stack), 20, 20, 1, TX_AUTO_START);
    if (ret != TX_SUCCESS) return ret;

    ret = tx_thread_create(&math_thread, "MATH", MATH_Thread, 0, math_stack, sizeof(math_stack), 20, 20, 1, TX_AUTO_START);
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


//// TX ONLY TEST
static VOID Ethernet_Thread(ULONG thread_input)
{
    UINT status;

    (void)thread_input;

    status = NetXDuo_DHCP_Wait();
    if (status != NX_SUCCESS)
        return;

    status = NetXDuo_TCP_Server_Start(TCP_PORT);
    if (status != NX_SUCCESS)
        return;

    while (1)
    {
        printf("Waiting TCP client...\r\n");

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
                printf("TCP send error: 0x%02X\r\n", status);
                break;
            }
        }

        NetXDuo_TCP_Disconnect();
    }
}

static VOID LCD_Text_Thread(ULONG arg){

	UX_PARAMETER_NOT_USED(arg);

	char text_buffer[256];

	while(1){

		if(lcd_result_ready == 0){
			tx_thread_sleep(1);
			continue;
		}

	    UTIL_LCD_Clear(0x00000000);
	    UTIL_LCD_SetFont(&Font16);
	    UTIL_LCD_SetTextColor(UTIL_LCD_COLOR_LIGHTGREEN);

	    if(pp_output.nb_detect){
			for(int i = 0; i < pp_output.nb_detect; i++){

				snprintf(text_buffer, sizeof(text_buffer),
						"Face %d: C=%.2f X=%.2f Y=%.2f W=%.2f H=%.2f",
						i+1,
						pp_output.pOutBuff[i].conf,
						pp_output.pOutBuff[i].x_center * SCREEN_WIDTH,
						pp_output.pOutBuff[i].y_center * SCREEN_HEIGHT,
						pp_output.pOutBuff[i].width,
						pp_output.pOutBuff[i].height
				);

				UTIL_LCD_DisplayStringAt(10, 10, (uint8_t *)text_buffer, LEFT_MODE);

			}
	    }
	    else{
	    	snprintf(text_buffer, sizeof(text_buffer), "No face detected");
	    	UTIL_LCD_DisplayStringAt(10, 10, (uint8_t *)text_buffer, LEFT_MODE);
	    }

	    /* CPU cache -> PSRAM, so LTDC sees updated pixels */
	    SCB_CleanDCache_by_Addr((uint32_t *)lcd_fg_buffer[0], LCD_FG_FRAMEBUFFER_SIZE);

	    lcd_result_ready = 0;
	}


}

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

        /* Kameradan alınan görüntüyü modelden geçir */
        AI_Run();

        /* Ham NN çıktısını gerçek yüz detection sonucuna çevir */
        app_postprocess_run((void **)nn_out, number_output, &pp_output, &pp_params);

        ai_face_count = pp_output.nb_detect;

        ai_task_counter++;

        ai_result_ready = 1;
        lcd_result_ready = 1;

        /* Sonraki kamera snapshot'ını başlat */
        CameraPipeline_IspUpdate();

        CameraPipeline_NNPipe_Start((uint8_t *)nn_in, DCMIPP_MODE_SNAPSHOT);

        HAL_GPIO_TogglePin(LED_RED_GPIO_Port, LED_RED_Pin);

    }
}

// USB Print Detected Face function
//static VOID USB_TX_Thread1(ULONG arg)
//{
//    ULONG actual_length;
//    UINT status;
//
//    UX_PARAMETER_NOT_USED(arg);
//
//    while (1)
//    {
//        if (cdc_acm == UX_NULL)
//        {
//            tx_thread_sleep(1);
//            continue;
//        }
//
//        if (ai_result_ready)
//        {
//            usb_ready_seen++;
//
//            int len = snprintf((char *)usb_msg, sizeof(usb_msg), "AI=%lu Faces=%ld\r\n", (unsigned long)ai_task_counter, (long)ai_face_count);
//
//            uint32_t clean_len = ((uint32_t)len + 31U) & ~31U;
//            SCB_CleanDCache_by_Addr((uint32_t *)usb_msg, clean_len);
//
//            status = ux_device_class_cdc_acm_write(cdc_acm, usb_msg, len, &actual_length);
//
//            if (status == UX_SUCCESS)
//            {
//                usb_write_ok++;
//                ai_result_ready = 0;
//            }
//            else
//            {
//                usb_write_err++;
//            }
//        }
//        else
//        {
//            tx_thread_sleep(1);
//        }
//    }
//}

// USB Throughput test function
static VOID USB_TX_Thread2(ULONG arg)
{
    ULONG actual_length;
    UINT status;

    UX_PARAMETER_NOT_USED(arg);

    while (1)
    {
        if (cdc_acm == UX_NULL)
        {
            tx_thread_sleep(1);
            continue;
        }

        status = ux_device_class_cdc_acm_write(cdc_acm, usb_tx_buffer, USB_TX_BUFFER_SIZE, &actual_length);

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

static VOID MATH_Thread(ULONG arg)
{
    float x = 1.2345f;

    UX_PARAMETER_NOT_USED(arg);

    while (1)
    {
        /* Example math workload */
        x = x * 1.00001f + 0.0001f;
        x = x * x;
        x = x / 1.0001f;

        if (x > 100.0f)
            x = 1.2345f;

        math_result = x;
        math_task_counter++;

        for(int i = 0; i < 1000; i++){
        	math_result++;
        }

        HAL_GPIO_TogglePin(GPIOE, GPIO_PIN_14);
        tx_thread_relinquish();
    }
}

static VOID LED1_Thread(ULONG arg)
{
    UX_PARAMETER_NOT_USED(arg);

    while (1)
    {
        led1_task_counter++;
        //HAL_GPIO_TogglePin(LED_GREEN_GPIO_Port, LED_GREEN_Pin);
        tx_thread_sleep(25);

    }
}

static VOID LED2_Thread(ULONG arg)
{
    UX_PARAMETER_NOT_USED(arg);

    while (1)
    {
        led2_task_counter++;
        //HAL_GPIO_TogglePin(LED_RED_GPIO_Port, LED_RED_Pin);
        tx_thread_sleep(50);
    }
}
