# STM32 UART Communication Using DMA
# Overview
This project demonstrates UART communication using Direct Memory Access (DMA) on an STM32 microcontroller, implemented using bare-metal C programming.
The implementation explores how to configure UART2 and DMA1 registers directly to perform data transmission and reception without continuously involving the CPU.
The project uses DMA interrupts to detect transfer completion and USART interrupts to monitor UART transmission status.

# Features
-->Bare-metal UART2 configuration.
-->UART baud rate configuration (115200 baud).
-->UART transmission using DMA.
-->UART reception using DMA.
-->DMA transfer-complete interrupts.
-->USART transmission-complete interrupt.
-->Register-level peripheral configuration.

# Project Structure
STM32F4-UART-DMA-Communication/
main.c
uart_dma.c
uart_dma.h
README.md

# Hardware and Development Environment
-->Microcontroller: STM32 (STM32F4 series)
-->Programming Language: C
-->Communication Protocol: UART
-->DMA Peripheral: DMA1
-->UART Peripheral: USART2
-->Baud Rate: 115200
-->Development Approach: Bare-metal register-level programming

# How It Works
1. GPIO pins are configured for USART2 alternate-function mode.
2. USART2 is configured for UART transmission and reception.
3. DMA1 Stream 5 is configured for UART reception.
4. DMA1 Stream 6 is configured for UART transmission.
5. DMA interrupts indicate transfer completion.
6. Received data is processed and transmitted back through UART.

# Learning Reference
This project was developed as part of my embedded systems learning journey, using concepts and practical guidance from:
Book: Bare-Metal Embedded C Programming: Develop High-Performance Embedded Systems with C for Arm Microcontrollers
# Author: Israel Gbati
The book served as a learning reference for understanding bare-metal programming, peripheral register configuration, UART communication, and DMA operation.

# Learning Objectives
1. Understanding STM32 peripheral registers.
2. Configuring UART without relying on high-level HAL drivers.
3. Understanding DMA streams and channels.
4. Implementing interrupt-driven peripheral communication.

# Author
Patrick NDAYIKUNDA
