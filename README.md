# STM32F4discoveryでミュージック・ビジュアライザーを作る
## Index
* システムとシーケンス
* HAL_Delay()関数の修正
* FFTとIIEフィルタの実装
* TIPS STM32F4でstd::coutのようにusartを使う  
* References

## システムとシーケンス  
<img src="images/system.png" width="700">
<img src="images/sequence.png" width="700">

## HAL_Delay()関数の修正  
0Hz~5000Hz程度までの音を可視化するため, 10000Hzで音のサンプリングを行いたい.  
通常通り周辺回路のTimerを設定してもよいが, HALライブラリではSystemクロックを用いたHal_Delay()関数が用意されているのでそれを用いて簡単にすませたい.  
しかしHAL_Delay()関数の最小時間は1msであるため修正を加えなければいけない.  
HAL_Delay()用のクロックの設計はHAL_Init()で行われている. さらにHAL_Init()の中を追っていくとstm32f4xx_hal.cの中の以下の関数に行き着くので修正を加える.  

```cpp
__weak HAL_StatusTypeDef HAL_InitTick(uint32_t TickPriority)
{
  /*Configure the SysTick to have interrupt in 1ms time basis*/
  //HAL_SYSTICK_Config(HAL_RCC_GetHCLKFreq()/1000U); ←もともとこうだった
  HAL_SYSTICK_Config(HAL_RCC_GetHCLKFreq()/10000U); // こう修正した

  /*Configure the SysTick IRQ priority */
  HAL_NVIC_SetPriority(SysTick_IRQn, TickPriority ,0U);

  /* Return function status */
  return HAL_OK;
}

```
これによってHAL_Delay(1)で100usの待ちをいれることができる.

## FFTとIIRフィルタの実装  

## TIPS STM32F4でstd::coutのようにusartを使う  
 - stm32f4のHALライブラリーを使用する.  
 - my_usart_stream.h  

```cpp
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
```

## References
* C言語によるFFTやIIRの実装について詳しく書かれている本  
<a href="http://floor13.sakura.ne.jp/book03/book03.html">[1] 青木 直史, C言語で始める音のプログラミング</a>

