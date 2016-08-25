#ifndef MY_USART_PRINTF_H
#define MY_USART_PRINTF_H

#include <cstdio>
#include <cstdarg>
#include <cstdlib>

/*
 * @brief printf for STM32 HAL
 */
void myUsartPrintf(UART_HandleTypeDef *huart, const char *format, ...){
	va_list list;
	va_start(list, format);
	int len = vsnprintf(0, 0, format, list);
	char *s;
	s = (char *)malloc(len + 1);
	vsprintf(s, format, list);
	HAL_UART_Transmit(huart, (uint8_t*)s, len, 30);
	free(s);
	va_end(list);
}

#endif // MY_USART_PRINTF_H
