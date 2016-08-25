#include "stm32f4xx.h"

#include <vector>

//#include "my_usart_printf.h"
#include "my_usart_stream.h"
#include "my_digital_filter.h"
#include "my_fft.h"

#define FFT_SAMPLE_RATE 10000.0 // Hz
#define FFT_N 128

static void initGpio();
static void initUsart();
static void initAdc();
static void initPwm();

static void SystemClock_Config(void);
static void Error_Handler(void);

UART_HandleTypeDef huart2;
ADC_HandleTypeDef hadc1;
TIM_HandleTypeDef htim4;

/*
 * @brief main
 */
int main(void)
{

	/*
	 * Start the HAL Library
	 * a) configures the Flash prefetch, instruction and data caches(configurable in stm32f4xx_hal_conf.h)
	 * b) Configure the SysTick to generate an interrupt every 1ms (to use HAL_Dealy()),Systick is clocked by HSI
	 * c) set NVIC group priority to 4
	 * d) calls the HAL_MspInit() callback function (defined in stm32f4xx_hal_msp.c) to do the global low-level hardwere initialization
	 */
	if(HAL_Init()!= HAL_OK){
		/* Start Conversation Error */
		Error_Handler();
	}

	/*
	 * Configure the system clock (please call 2 APIs)
	 * a) HAL_RCC_OscConfig() : if the system must not run at high frequency, user can skip this function
	 * b) HAL_RCC_ClockConfig() : configures AHB and APB prescalers
	 */
	SystemClock_Config();

	/*
	 * Peripheral initialization
	 * a) start by writing HAL_PPP_MspInit function
	 * b) edit stm32f4xx_it.c to call the required interrupt function
	 */
	//initGpio();
	initUsart();
	initAdc();
	initPwm();

	/*
	 * Then, start application
	 */

	usartStream us(huart2);

	std::vector<float> Input_real(FFT_N);
	std::vector<float> Input_imag(FFT_N);
	std::vector<float> Output(FFT_N/2);

	//IIR_LPFの作成
	float fc, Q, a[3], b[3];
	fc = 5000.0 / FFT_SAMPLE_RATE; // 遮断周波数
	Q = 1.0 / sqrt(2.0); // クオリティファクタ
	IIR_LPF(fc, Q, a, b); // 係数a,bの計算

	while(1){

		// sample data
		for (int i = 0; i < FFT_N; i++){
			Input_real[i] = HAL_ADC_GetValue(&hadc1) - 2047.0;;
			Input_imag[i] = 0.0;
			HAL_Delay(1);
		}

		// Disp input data
		/*
		for (int i = 0; i < FFT_N; i++)
			us << "[FFT_INPUT]" << i << "," << (int)Input_real[i] << "\r\n";
		*/

		// IIR Low Pass Filtering

		for (int n = 0; n < FFT_N; n++){
			for (int m = 0; m <= 2; m++)
				if (n - m >= 0)
					Input_real[n] += b[m] * Input_real[n - m];
			for (int m = 1; m <= 2; m++)
				if (n - m >= 0)
					Input_real[n] += -a[m] * Input_real[n - m];
		}


		// FFT
		tFFT(Input_real, Input_imag, FFT_N);

		// Calc Power Spectrum
		for (int i = 0; i < FFT_N/2; i++)
			Output[i] = Input_real[i]*Input_real[i] + Input_imag[i] * Input_imag[i];

		// Disp output data
		for (int i = 0; i < FFT_N/2; i++)
			us << "[FFT_OUTPUT]" << i << "," << (int)Output[i] << "\r\n";
		us << "\r\n";

		/* wati */
		//HAL_GPIO_TogglePin(GPIOD, GPIO_PIN_13);
		HAL_Delay(10000);
	}
}

/*
 * @brief init PD12, PD13 as GPIO
 */
static void initGpio(){
	// Enable the Clock
	__GPIOD_CLK_ENABLE();

	// Configure GPIO
	GPIO_InitTypeDef  GPIO_InitStruct;
	GPIO_InitStruct.Pin = GPIO_PIN_12|GPIO_PIN_13; //LED4,LED3
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_PULLUP;
	GPIO_InitStruct.Speed = GPIO_SPEED_FAST;
	HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);
	HAL_GPIO_WritePin(GPIOD, GPIO_PIN_13, GPIO_PIN_RESET);

}

/*
 * @brief init PA2, PA3 as USART2
 */
static void initUsart(){
	// Enable the Clock
	__GPIOA_CLK_ENABLE();
	__USART2_CLK_ENABLE();

	// Configure GPIO for USART
	GPIO_InitTypeDef  GPIO_InitStruct;
	GPIO_InitStruct.Pin = GPIO_PIN_2 | GPIO_PIN_3;
	GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_LOW;
	GPIO_InitStruct.Alternate = GPIO_AF7_USART1;
	HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

	// Configure USART
	huart2.Instance = USART2;
	huart2.Init.BaudRate = 460800;
	huart2.Init.WordLength = UART_WORDLENGTH_8B;
	huart2.Init.StopBits = UART_STOPBITS_1;
	huart2.Init.Parity = UART_PARITY_NONE;
	huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
	huart2.Init.Mode = UART_MODE_TX_RX;
	HAL_UART_Init(&huart2);

}

/*
 * @brief init PC1 as ADC1
 */
static void initAdc(){
	// Enable the Clock
	__GPIOC_CLK_ENABLE();
	__ADC1_CLK_ENABLE();

	// Configure GPIO for ADC
	GPIO_InitTypeDef GPIO_InitStruct;
	GPIO_InitStruct.Pin = GPIO_PIN_1;
	GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

	HAL_NVIC_SetPriority(ADC_IRQn, 0, 0);
	HAL_NVIC_EnableIRQ(ADC_IRQn);

	// Configure ADC
	hadc1.Instance = ADC1;
	hadc1.Init.ClockPrescaler = ADC_CLOCKPRESCALER_PCLK_DIV2;
	hadc1.Init.Resolution = ADC_RESOLUTION_12B;
	hadc1.Init.ScanConvMode = DISABLE;
	hadc1.Init.ContinuousConvMode = ENABLE;
	hadc1.Init.DiscontinuousConvMode = DISABLE;
	hadc1.Init.NbrOfDiscConversion = 0;
	hadc1.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_NONE;
	hadc1.Init.ExternalTrigConv = ADC_EXTERNALTRIGCONV_T1_CC1;
	hadc1.Init.DataAlign = ADC_DATAALIGN_RIGHT;
	hadc1.Init.NbrOfConversion = 1;
	hadc1.Init.DMAContinuousRequests = ENABLE;
	hadc1.Init.EOCSelection = DISABLE;
	HAL_ADC_Init(&hadc1);

	// Configure channels 1 for ADC
	ADC_ChannelConfTypeDef adcChannel;
	adcChannel.Channel = ADC_CHANNEL_11;
	adcChannel.Rank = 1;
	adcChannel.SamplingTime = ADC_SAMPLETIME_480CYCLES;
	adcChannel.Offset = 0;
	if (HAL_ADC_ConfigChannel(&hadc1, &adcChannel) != HAL_OK)
		asm("bkpt 255");

	// start ADC
	HAL_ADC_Start(&hadc1);

}

/*
 * @brief init PD12, PD13, PD14, PD15 as PWM(TIM4)
 */
static void initPwm(){
	// Enable the Clock
	__GPIOD_CLK_ENABLE();
	__TIM4_CLK_ENABLE();

	// Configure GPIO for PWM
	GPIO_InitTypeDef GPIO_InitStruct;
	GPIO_InitStruct.Pin = GPIO_PIN_12|GPIO_PIN_13|GPIO_PIN_14|GPIO_PIN_15;
	GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_HIGH;
	GPIO_InitStruct.Alternate = GPIO_AF2_TIM4;
	HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);

	// Configure TIM4 for PWM
	htim4.Instance = TIM4;
	htim4.Init.Prescaler = 42000 - 1; // (84MHz / 42000 = 2KHz)
	htim4.Init.CounterMode = TIM_COUNTERMODE_UP;
	htim4.Init.Period = 1000 - 1; // Period = 2000 -> 1 second
	htim4.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
	HAL_TIM_PWM_Init(&htim4);

	// Configure channels 1-4 for TIM4,
	TIM_OC_InitTypeDef sConfigOC;
	sConfigOC.OCMode = TIM_OCMODE_PWM1;
	sConfigOC.OCIdleState = TIM_OCIDLESTATE_SET;
	sConfigOC.Pulse = 500;
	sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
	sConfigOC.OCFastMode = TIM_OCFAST_ENABLE;

	HAL_TIM_PWM_ConfigChannel(&htim4, &sConfigOC, TIM_CHANNEL_1);
	HAL_TIM_PWM_ConfigChannel(&htim4, &sConfigOC, TIM_CHANNEL_2);
	HAL_TIM_PWM_ConfigChannel(&htim4, &sConfigOC, TIM_CHANNEL_3);
	HAL_TIM_PWM_ConfigChannel(&htim4, &sConfigOC, TIM_CHANNEL_4);

	// start PWM
	HAL_TIM_PWM_Start(&htim4, TIM_CHANNEL_1);
	HAL_TIM_PWM_Start(&htim4, TIM_CHANNEL_2);
	HAL_TIM_PWM_Start(&htim4, TIM_CHANNEL_3);
	HAL_TIM_PWM_Start(&htim4, TIM_CHANNEL_4);

}

/**
  * @brief  This function is executed in case of error occurrence.
  * @param  None
  * @retval None
  */
static void Error_Handler(void){
	while(1){
		//HAL_GPIO_TogglePin(GPIOD, GPIO_PIN_12);
		HAL_Delay(10000);
	}
}

/**
  * @brief  System Clock Configuration
  *         The system Clock is configured as follow :
  *            System Clock source            = PLL (HSE)
  *            SYSCLK(Hz)                     = 168000000
  *            HCLK(Hz)                       = 168000000
  *            AHB Prescaler                  = 1
  *            APB1 Prescaler                 = 4
  *            APB2 Prescaler                 = 2
  *            HSE Frequency(Hz)              = 8000000
  *            PLL_M                          = 8
  *            PLL_N                          = 336
  *            PLL_P                          = 2
  *            PLL_Q                          = 7
  *            VDD(V)                         = 3.3
  *            Main regulator output voltage  = Scale1 mode
  *            Flash Latency(WS)              = 5
  * @param  None
  * @retval None
  */
static void SystemClock_Config(void){
	RCC_ClkInitTypeDef RCC_ClkInitStruct;
	RCC_OscInitTypeDef RCC_OscInitStruct;

	/* Enable Power Control clock */
	__HAL_RCC_PWR_CLK_ENABLE();

	/* The voltage scaling allows optimizing the power consumption when the device is
	 * clocked below the maximum system frequency, to update the voltage scaling value
	 * regarding system frequency refer to product datasheet.
	 */
	__HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

	/* Enable HSE Oscillator and activate PLL with HSE as source */
	RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
	RCC_OscInitStruct.HSEState = RCC_HSE_ON;
	RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
	RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
	RCC_OscInitStruct.PLL.PLLM = 8;
	RCC_OscInitStruct.PLL.PLLN = 336;
	RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
	RCC_OscInitStruct.PLL.PLLQ = 7;
	HAL_RCC_OscConfig(&RCC_OscInitStruct);

	/* Select PLL as system clock source and configure the HCLK, PCLK1 and PCLK2 clocks dividers */
	RCC_ClkInitStruct.ClockType = (RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2);
	RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
	RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
	RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
	RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;
	HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5);

	/* STM32F405x/407x/415x/417x Revision Z devices: prefetch is supported  */
	if (HAL_GetREVID() == 0x1001){
		/* Enable the Flash prefetch */
		__HAL_FLASH_PREFETCH_BUFFER_ENABLE();
	}
}
