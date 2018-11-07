//#include <cmsis.h>
#include <stm32f1xx.h>
#include <system_stm32f1xx.h>

#include "gpio.h"
#include <cstdio>
#include <cstring>

#include "encoder.h"
#include "motor.h"
#include "laser.h"
#include "config.h"
#include "controller.h"
#include "protocol.h"
#include "stepper.h"
#include "uart.h"

#define NVIC_PRIORITYGROUP_4         ((uint32_t)0x00000003) /*!< 4 bits for pre-emption priority
                                                                 0 bits for subpriority */
#define SYS_TICK_INT_PRIORITY            ((uint32_t)0) 


static volatile uint32_t _st_ticks = 0;
//SysTick Interrupt
extern "C" void SysTick_Handler(void)
{
	++_st_ticks;
}

void delay(uint32_t ms) {
	uint32_t t = _st_ticks + ms;
	while (_st_ticks < t) {
		__NOP();
	}
}
uint32_t get_ms_timer() { return _st_ticks; }




static UART com_uart(USART1);

class UART_Protocol : public Protocol {
public:
	void process() {
		Protocol::process();
	}
	virtual uint16_t send_data(const void* data,uint16_t size) {
		return com_uart.transmit(static_cast<const uint8_t*>(data),size);
	}
	void process_rx() {
		while (!com_uart.rx_empty()) {
			on_rx_byte(com_uart.read());
		}
	}
};

static UART_Protocol uart_protocol;



extern "C" void SystemInit() {
	
}


#define SET_SYSCLOCK(SRC) MODIFY_REG(RCC->CFGR,RCC_CFGR_SW,SRC)
extern "C" int main() {
	// hsi
	SET_BIT(RCC->CR, RCC_CR_HSION);
  	while((RCC->CR & RCC_CR_HSIRDY)==0) {__NOP();}
  	SET_SYSCLOCK(RCC_CFGR_SW_HSI);


  	SET_BIT(RCC->CR, RCC_CR_HSEON);
  	while((RCC->CR & RCC_CR_HSERDY)==0) {__NOP();}
  	SET_SYSCLOCK(RCC_CFGR_SW_HSE);

  	// hpre
  	MODIFY_REG(RCC->CFGR,RCC_CFGR_HPRE,RCC_CFGR_HPRE_DIV1);
  	MODIFY_REG(RCC->CFGR,RCC_CFGR_ADCPRE,RCC_CFGR_ADCPRE_DIV8);
  	MODIFY_REG(RCC->CFGR, RCC_CFGR_PPRE1, RCC_CFGR_PPRE1_DIV2);
  	MODIFY_REG(RCC->CFGR, RCC_CFGR_PPRE2, RCC_CFGR_PPRE2_DIV1);
	// 1.5 prescaler
  	MODIFY_REG(RCC->CFGR, RCC_CFGR_USBPRE, 0);

	
	// prefetch enable
	SET_BIT(FLASH->ACR,FLASH_ACR_PRFTBE);
	MODIFY_REG(FLASH->ACR,FLASH_ACR_LATENCY,2);
	
	MODIFY_REG(RCC->CFGR, (RCC_CFGR_PLLSRC | RCC_CFGR_PLLMULL),(RCC_CFGR_PLLSRC | RCC_CFGR_PLLMULL9 ));
  	MODIFY_REG(RCC->CFGR,RCC_CFGR_PLLXTPRE, RCC_CFGR_PLLXTPRE_HSE);
  
  	SET_BIT(RCC->CR,RCC_CR_PLLON);
  	while((RCC->CR & RCC_CR_PLLRDY)==0) {__NOP();}
  	SET_SYSCLOCK(RCC_CFGR_SW_PLL);


	// configure rcc
	SET_BIT(RCC->APB2ENR, RCC_APB2ENR_AFIOEN);
    // Delay after an RCC peripheral clock enabling 
    volatile uint32_t tmpreg = READ_BIT(RCC->APB2ENR, RCC_APB2ENR_AFIOEN);
    (void)tmpreg;

    MODIFY_REG(AFIO->MAPR, AFIO_MAPR_SWJ_CFG, AFIO_MAPR_SWJ_CFG_JTAGDISABLE);

    SET_BIT(RCC->APB2ENR, RCC_APB2ENR_IOPDEN);
    tmpreg = READ_BIT(RCC->APB2ENR, RCC_APB2ENR_IOPDEN);
    (void)tmpreg;
  	
	// enable gpiob
	SET_BIT(RCC->APB2ENR, RCC_APB2ENR_IOPBEN);
	tmpreg = READ_BIT(RCC->APB2ENR, RCC_APB2ENR_IOPBEN);
	(void)tmpreg;

	// enable gpioa
	SET_BIT(RCC->APB2ENR, RCC_APB2ENR_IOPAEN);
	tmpreg = READ_BIT(RCC->APB2ENR, RCC_APB2ENR_IOPAEN);
	(void)tmpreg;

	// enable gpioc
	SET_BIT(RCC->APB2ENR, RCC_APB2ENR_IOPCEN);
	tmpreg = READ_BIT(RCC->APB2ENR, RCC_APB2ENR_IOPCEN);
	(void)tmpreg;

	// enable DMA1
	SET_BIT(RCC->AHBENR, RCC_AHBENR_DMA1EN);
	tmpreg = READ_BIT(RCC->AHBENR, RCC_AHBENR_DMA1EN);
	(void)tmpreg;



  	NVIC_SetPriorityGrouping(NVIC_PRIORITYGROUP_4);
	
	// init SysTick
	//SystemCoreClockUpdate();
	SysTick_Config(FREQ_CPU/1000);


	NVIC_SetPriority(SysTick_IRQn, 7);



	Pin(GPIOA,9).configure_af_pp();
	Pin(GPIOA,10).configure_input_f();

	SET_BIT(RCC->APB2ENR, RCC_APB2ENR_USART1EN);
	tmpreg = READ_BIT(RCC->APB2ENR, RCC_APB2ENR_USART1EN);
	(void)tmpreg;

	NVIC_SetPriority(USART1_IRQn, 0);
	NVIC_EnableIRQ(USART1_IRQn);
	
	com_uart.configure_8n1(115200);
	com_uart.start_rx_it();


	Encoder::init();
	
	
	Laser::init();
	Motor::init();
	Stepper::init();

	Encoder::set_zero();

	//DBG("started\n");

	
	while (true) {
		
		Controller::process();
		if (com_uart.tx_empty()) {
			uart_protocol.process();
		}
		uart_protocol.process_rx();
	}

	return 0;
}

extern "C" void USART1_IRQHandler(void)
{
	// __IO uint32_t sr = USART1->SR;
	// if (sr & USART_SR_RXNE) {
	// 	__IO uint32_t tmpreg = USART1->DR;
	// 	//uart_protocol.on_rx_byte(tmpreg);
	// 	//sr = USART1->SR;
	// 	USART1->DR = tmpreg;
	// } 
	com_uart.on_irq();
}

extern "C" void HardFault_Handler() {
	while(true){__NOP();}
}
extern "C" void MemManage_Handler() {
	while(true){__NOP();}
}
extern "C" void BusFault_Handler() {
	while(true){__NOP();}
}
extern "C" void UsageFault_Handler() {
	while(true){__NOP();}
}
extern "C" void WWDG_IRQHandler() {
	while(true){__NOP();}
}