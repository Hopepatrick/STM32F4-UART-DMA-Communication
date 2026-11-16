/*
 * uart_dma.H
 *
 *  Created on: Feb 14, 2025
 *      Author: Patrick NDAYIKUNDA
 */

#ifndef UART_DMA_H_
#define UART_DMA_H_

#include<stdint.h>
#include "stm32f4xx.h"

#define BUFFER_SIZE	5
void dma1_init(void);
void uart2_rx_tx_init(void);
void dma1_stream5_uart_rx_config(void);
void dma1_stream6_uart_tx_config(uint32_t data, uint32_t data_length);

#endif /* UART_DMA_H_ */
