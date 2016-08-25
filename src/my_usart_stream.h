#ifndef MY_USART_STREAM_H
#define MY_USART_STREAM_H

#include "stm32f4xx.h"
#include <string>
#include <sstream>

/*
 * @brief std::cout for STM32 HAL
 */
class usartStream{
private:
	UART_HandleTypeDef husart;
public:
	usartStream(UART_HandleTypeDef huartx) { this->husart = huartx; }

	template<class T>
	usartStream operator<<(T val);
};

template<class T>
usartStream usartStream::operator <<(T val){
	std::stringstream ss;
	ss << val;
	HAL_UART_Transmit(&husart, (uint8_t*)ss.str().c_str(), ss.str().size(), 30);

	usartStream tmp(husart);
	return tmp;
}

#endif // MY_USART_STREAM_H
