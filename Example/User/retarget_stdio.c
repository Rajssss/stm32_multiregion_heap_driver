/*
 * retarget_stdio.c
 *
 *  Created on: Feb 12, 2022
 *      Author: rajssss
 */
#include "stm32f7xx_hal.h"
#include "retarget_stdio.h"
#include <stdio.h>
#include <errno.h>
#include <sys/stat.h>

#define STDIN_FILENO 0
#define STDOUT_FILENO 1
#define STDERR_FILENO 2


void RetargetInit(USART_TypeDef *UARTx)
{
	gHuart.Instance = UARTx;
	gHuart.Init.BaudRate = 115200;
	gHuart.Init.HwFlowCtl = UART_HWCONTROL_NONE;
	gHuart.Init.Mode = UART_MODE_TX_RX;
	gHuart.Init.Parity = UART_PARITY_NONE;
	gHuart.Init.StopBits = UART_STOPBITS_1;
	gHuart.Init.WordLength = UART_WORDLENGTH_8B;
	gHuart.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
	HAL_UART_Init(&gHuart);

	/* Disable I/O buffering for STDOUT stream, so that
	 * chars are sent out as soon as they are printed. */
	setvbuf(stdout, NULL, _IONBF, 0);
}


void HAL_UART_MspInit(UART_HandleTypeDef *huart)
{
  GPIO_InitTypeDef  GPIO_InitStruct;

  /*##-1- Enable peripherals and GPIO Clocks #################################*/
  /* Enable GPIO TX/RX clock */
  USARTx_TX_GPIO_CLK_ENABLE();
  USARTx_RX_GPIO_CLK_ENABLE();


  /* Enable USARTx clock */
  USARTx_CLK_ENABLE();

  /*##-2- Configure peripheral GPIO ##########################################*/
  /* UART TX GPIO pin configuration  */
  GPIO_InitStruct.Pin       = USARTx_TX_PIN;
  GPIO_InitStruct.Mode      = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull      = GPIO_PULLUP;
  GPIO_InitStruct.Speed     = GPIO_SPEED_HIGH;
  GPIO_InitStruct.Alternate = USARTx_TX_AF;

  HAL_GPIO_Init(USARTx_TX_GPIO_PORT, &GPIO_InitStruct);

  /* UART RX GPIO pin configuration  */
  GPIO_InitStruct.Pin = USARTx_RX_PIN;
  GPIO_InitStruct.Alternate = USARTx_RX_AF;

  HAL_GPIO_Init(USARTx_RX_GPIO_PORT, &GPIO_InitStruct);
}


int _write(int fd, char* ptr, int len)
{
	HAL_StatusTypeDef hstatus;

	if (fd == STDOUT_FILENO || fd == STDERR_FILENO)
	{
		hstatus = HAL_UART_Transmit(&gHuart, (uint8_t *) ptr, len, HAL_MAX_DELAY);
		if (hstatus == HAL_OK)
			return len;
		else
			return EIO;
	}
	errno = EBADF;
	return -1;
}


int _read(int fd, char* ptr, int len) {
	HAL_StatusTypeDef hstatus;

	if (fd == STDIN_FILENO) {
		hstatus = HAL_UART_Receive(&gHuart, (uint8_t *) ptr, 1, HAL_MAX_DELAY);
		if (hstatus == HAL_OK)
			return 1;
		else
			return EIO;
	}
	errno = EBADF;
	return -1;
}

