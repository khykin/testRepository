/* Blink Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/

#define DEBUG 1

/* inclued file are located in //User/51chevytruck/esp/esp-idf/componenets */
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"			// FreeRTOS Task Code
#include "freertos/queue.h"			// FreeRTOS Queue Code
#include "freertos/semphr.h"		// FreeRTOS Semaphore Code
#include "driver/gpio.h"
#include "sdkconfig.h"

/* Can use project configuration menu (idf.py menuconfig) to choose the GPIO to blink,
   or you can edit the following line and set a number here.
*/
//#define BLINK_GPIO 2
//#define BLINK_GPIO1 4
//#define BLINK_GPIO2 5


//static const char LOG_TAG[] = "DumpInfo";


// this is used when converting to a .cpp file
//extern "C" {
//	void app_main(void);
//}

/* Can run 'make menuconfig' to choose the GPIO to blink,
   or you can edit the following line and set a number here.
*/
#define BLINK_GPIO CONFIG_BLINK_GPIO
#define BLINK_GPIO2 (CONFIG_BLINK_GPIO + 1)

xQueueHandle Global_Queue_Handle = 0;	// handle for the communication queue

void enableFlushAfterPrintf()
{
	/* Disable buffering on stdin and stdout */
	setvbuf(stdin, NULL, _IONBF, 0);
	setvbuf(stdout, NULL, _IONBF, 0);
}

void blink_task(void *pvParameter)
{
    gpio_num_t io_num = (gpio_num_t) BLINK_GPIO;	// set the I/O pin number

    printf("starting %i\n", io_num);
    /* Configure the IOMUX register for pad BLINK_GPIO (some pads are
       muxed to GPIO on reset already, but some default to other
       functions and need to be switched to GPIO. Consult the
       Technical Reference for a list of pads and their default
       functions.)
    */
    gpio_pad_select_gpio(io_num);
    /* Set the GPIO as a push/pull output */
    gpio_set_direction((gpio_num_t) io_num, (gpio_mode_t) GPIO_MODE_OUTPUT);
    /* variable for state */
    uint32_t ii = 0;
    while(1) {
    	//ii ^= 1; // toggle the flag
    	if(ii == 0)	ii = 1;	// toggle the value of the flag
    	else		ii = 0;
    	// notify led status
		#ifdef DEBUG
		printf("Sent Blink %s %i\n", ii ? "On " : "Off", io_num);
		#endif
    	if( !xQueueSend(Global_Queue_Handle, &ii, 1000))
    	{
    		printf("Failed to send to Queue off\n");
    	}

        gpio_set_level(io_num, ii);	// Blink on/off based on state
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}

void blink_task2(void *pvParameter)
{
    gpio_num_t io_num = (gpio_num_t) BLINK_GPIO2;	// set the I/O pin number

    printf("starting %i\n", io_num);
	/* Configure the IOMUX register for pad BLINK_GPIO (some pads are
       muxed to GPIO on reset already, but some default to other
       functions and need to be switched to GPIO. Consult the
       Technical Reference for a list of pads and their default
       functions.)
    */
    gpio_pad_select_gpio(io_num);
    /* Set the GPIO as a push/pull output */
    gpio_set_direction((gpio_num_t) io_num, (gpio_mode_t) GPIO_MODE_OUTPUT);

    uint32_t rx_ii = 0;
    while(1) {
    	// get the sent state, wait for up to 10 seconds
    	if(!xQueueReceive(Global_Queue_Handle, &rx_ii, 10000))
    	{
    		printf("Failed to receive value");
    	}
    	else
    	{
			printf("Received Blink %s %i\n", rx_ii ? "On " : "Off", rx_ii);
	        gpio_set_level(io_num, rx_ii);	// Blink on/off based on command
    	}
    }
}

void app_main(void)
{
	// Create the global queue
	Global_Queue_Handle = xQueueCreate(3, sizeof(int));

	xTaskCreate(&blink_task, "blink_task", configMINIMAL_STACK_SIZE * 5, NULL, 4, NULL);
	xTaskCreate(&blink_task2, "blink_task2", configMINIMAL_STACK_SIZE * 5, NULL, 5, NULL);
}
