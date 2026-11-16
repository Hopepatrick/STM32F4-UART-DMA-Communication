/*
 * uart_dma.c
 *
 *  Created on: Feb 14, 2025
 *      Author: Patrick NDAYIKUNDA
 */

#include "uart_dma.h"

#define UART2EN					(1<<17)
#define GPIOAEN					(1<<0)

#define CR1_TE					(1<<3)
#define CR1_RE					(1<<2)
#define CR1_UE					(1<<13)
#define SR_TXE					(1<<7)

#define CR3_DMAT				(1<<7)
#define CR3_DMAR				(1<<6)
#define SR_TC					(1<<6)
#define CR1_TCIE				(1<<6)

#define UART_BAUDRATE			115200
#define CLK						16000000

#define DMA1EN					(1<<21)
#define DMA_SCR_EN				(1<<0)
#define DMA_SCR_MINC			(1<<10)
#define DMA_SCR_PINC			(1<<9)
#define DMA_SCR_CIRC			(1<<8)
#define DMA_SCR_TCIE			(1<<4)
#define DMA_SFCR_DMDIS			(1<<2)

#define HIFCR_CDMEIF5			(1<<8)
#define HIFCR_CTEIF5			(1<<9)
#define HIFCR_CTCIF5			(1<<11)

#define HIFCR_CDMEIF6			(1<<18)
#define HIFCR_CTEIF6			(1<<19)
#define HIFCR_CTCIF6			(1<<21)

static uint16_t calculate_uart_baudrate(uint32_t peripheral_clock, uint32_t baudrate);

static void uart_set_baudrate(uint32_t peripheral_clock, uint32_t baudrate);

char uart_data[BUFFER_SIZE];

uint8_t g_rx_complete;
uint8_t g_tx_complete;
uint8_t g_uart_complete;

void uart2_rx_tx_init(void){

	//===================== CONFIG UART GPIO PIN=================

	// Enable clock access
	RCC->AHB1ENR |= GPIOAEN;

	// Set PA2 and PA3 to alternate function mode
	GPIOA->MODER &= ~(3U << 4);
	GPIOA->MODER |=  (2U << 4);

	GPIOA->MODER &= ~(3U << 6);
	GPIOA->MODER |=  (2U << 6);

	// Set PA2 alternate function to AF7(UART_TX)
	GPIOA->AFR[0] |= (1<<8);
	GPIOA->AFR[0] |= (1<<9);
	GPIOA->AFR[0] |= (1<<10);
	GPIOA->AFR[0] &= ~(1<<11);

	// Set PA3 alternate function to AF7(UART_RX)
	GPIOA->AFR[0] |= (1<<12);
	GPIOA->AFR[0] |= (1<<13);
	GPIOA->AFR[0] |= (1<<14);
	GPIOA->AFR[0] &= ~(1<<15);

	//======================== AFTER GPIO, NOW UART MODULE CONFIG =============

	// Enable clock access to UART2
	RCC->APB1ENR |= UART2EN;

	// Set baudrate
	uart_set_baudrate(CLK, UART_BAUDRATE);

	// Select to use DMA for TX and RX
	USART2->CR3 = CR3_DMAT | CR3_DMAR;

	// Set transfer direction
	USART2->CR1 = CR1_TE | CR1_RE;

	// Clear TC flag
	USART2->SR &= ~SR_TC;

	// Enable TCIE
	USART2->CR1 |= CR1_TCIE;

	// Enable uart module
	USART2->CR1 |= CR1_UE;

	// Enable USART2 interrupt in the NVIC
	NVIC_EnableIRQ(USART2_IRQn);

}

void dma1_init(void){

	// Enable clock access to DMA
	RCC->AHB1ENR |= DMA1EN;

	// Enable DMA stream6 Interrupt in NVIC
	NVIC_EnableIRQ(DMA1_Stream6_IRQn);

}

//==================== INITIALIZE DM1 STREAM 5 FOR UART RX =================

void dma1_stream5_uart_rx_config(void){

	// Disable DMA stream before configuration
	DMA1_Stream5->CR &= ~DMA_SCR_EN;

	// Wait till DMA is disabled
	while((DMA1_Stream5->CR & DMA_SCR_EN)){}

	// Clear interrupt flags for stream 5
	DMA1->HIFCR = HIFCR_CDMEIF6 | HIFCR_CTEIF5 | HIFCR_CTCIF5;

	// Set peripheral address register PAR
	DMA1_Stream5->PAR = (uint32_t)(&(USART2->DR));

	// Set memory address register M0AR
	DMA1_Stream5->M0AR = (uint32_t)(&uart_data);

	// Set number of data transfer NDTR
	DMA1_Stream5->NDTR = (uint16_t)BUFFER_SIZE;

	// Select Channel 4
	DMA1_Stream5->CR &= ~(1<<25);
	DMA1_Stream5->CR &= ~(1<<26);
	DMA1_Stream5->CR |= (1<<27);

	// Enable memory address increment after each transfer
	DMA1_Stream5->CR |= DMA_SCR_MINC;

	// Enable transfer complete interrupt
	DMA1_Stream5->CR |= DMA_SCR_TCIE;

	// Enable Circular mode
	DMA1_Stream5->CR |= DMA_SCR_CIRC;

	// Set transfer direction from peripheral to memory
	DMA1_Stream5->CR &= ~(1<<6);
	DMA1_Stream5->CR &= ~(1<<7);

	// Enable DMA stream5 after DMA1 configuration
	DMA1_Stream5->CR |= DMA_SCR_EN;

	// Enable DMA Stream5 Interrupt in NVIC
	NVIC_EnableIRQ(DMA1_Stream5_IRQn);

}

//==================== INITIALIZE DM1 STREAM 6 FOR UART TX =================

void dma1_stream6_uart_tx_config(uint32_t data, uint32_t data_length){

	// Disable DMA Stream before configuration
	DMA1_Stream6->CR &= ~DMA_SCR_EN;

	// Wait until DMA Stream 6 is disabled
	while ((DMA1_Stream6->CR & DMA_SCR_EN)){}

	// Clear interrupt flags for stream 6
	DMA1->HIFCR = HIFCR_CDMEIF6 | HIFCR_CTEIF6 | HIFCR_CTCIF6;

	// Set peripheral address register PAR
	DMA1_Stream6->PAR = (uint32_t)(&(USART2->DR));

	// Set memory 0 address register M0AR
	DMA1_Stream6->M0AR = data;

	// Set the number of transfer
	DMA1_Stream6->NDTR = data_length;

	// Select the channel to be used on stream 6, channel 4 has selected
	DMA1_Stream6->CR &= ~(1<<25);
	DMA1_Stream6->CR &= ~(1<<26);
	DMA1_Stream6->CR |=  (1<<27);

	// Enable memory address increment after each transfer
	DMA1_Stream6->CR |= DMA_SCR_MINC;

	// Now enable direction from peripheral to memory
	DMA1_Stream6->CR |= (1<<6);
	DMA1_Stream6->CR &= ~(1<<7);

	// Set transfer complete
	DMA1_Stream6->CR |= DMA_SCR_TCIE;

	// Then after DMA1 Stream 6 configuration, enable it
	DMA1_Stream6->CR |= DMA_SCR_EN;
}

//=========================== NOW CALCULATE THE BAUDRATE VALUE FOR THE UART ============

static uint16_t calculate_uart_baudrate(uint32_t peripheral_clock, uint32_t baudrate){

	uint16_t bdrate_value = ((peripheral_clock + (baudrate / 2)) / baudrate);
	return bdrate_value;
}

//======================== NOW SET CALCULATED BAUDRATE VALUE TO THE BAUDRATE REGISTER OF USART2==============

static void uart_set_baudrate(uint32_t peripheral_clock, uint32_t baudrate){

	uint16_t brr_value = calculate_uart_baudrate(peripheral_clock,baudrate);

	USART2->BRR = brr_value;
}

// Interrupt section

// Interrupt handler for stream 6

void DMA1_Stream6_IRQHandler(void){

	if((DMA1->HISR) & HIFCR_CTCIF6){

		// Here you can do anything after the interrupt occurred
		g_tx_complete = 1;
		// Clear the interrupt by setting HIFCR_CTCIF6 bit of HISR Register
		DMA1->HISR = HIFCR_CTCIF6;
	}
}

// Now interrupt handler function for stream 5 (RX in this case)

void DMA1_Stream5_IRQHandler(void){

	if((DMA1->HISR) & HIFCR_CTCIF5){

		// Here you can do anything after the interrupt occurred
		g_rx_complete = 1;
		// Clear the interrupt by setting HIFCR_CTCIF6 bit of HISR Register
		DMA1->HISR = HIFCR_CTCIF5;
	}
}

// Finally interrupt handler function for USART2

void USART2_IRQHandler(void){

	g_uart_complete = 1;

	// Clear TC interrupt flag
	USART2->SR &= ~SR_TC;
}
