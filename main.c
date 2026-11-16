/*
 * main.c
 *
 *  Created on: Feb 15, 2025
 *      Author: Patrick NDAYIKUNDA
 */

#include<stdint.h>
//#include "uart.h"
#include "uart_dma.h"
#include <stdio.h>
#include <string.h>

extern uint8_t g_rx_complete;
extern uint8_t g_tx_complete;
extern uint8_t g_uart_complete;
extern char uart_data[BUFFER_SIZE];
char message_buffer[150] = {'\0'};

int main(){

	//Initialize debug UART

	uart2_rx_tx_init();
	dma1_init();
	dma1_stream5_uart_rx_config();
	sprintf(message_buffer,"Initialization complete \n\r");
	dma1_stream6_uart_tx_config((uint32_t)message_buffer,strlen(message_buffer));

	// Wait for the message transfer
	while(!g_tx_complete){}

	while(1){
		// Check the message availability by reading g_rt_complete from the interrupt handler
		if(g_rx_complete){

			sprintf(message_buffer, "message received: %s \n\r",uart_data);
			g_rx_complete = 0;
			g_tx_complete = 0;
			g_uart_complete = 0;

			dma1_stream6_uart_tx_config((uint32_t)message_buffer,strlen(message_buffer));
			while(!g_tx_complete){}

		}

	}

}
