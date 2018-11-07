#ifndef _GPIO_H_INCLUDED_
#define _GPIO_H_INCLUDED_

#include <stm32f1xx.h>


/* Definitions for bit manipulation of CRL and CRH register */
#define  GPIO_CR_MODE_INPUT         ((uint32_t)0x00000000) /*!< 00: Input mode (reset state)  */
#define  GPIO_CR_CNF_ANALOG         ((uint32_t)0x00000000) /*!< 00: Analog mode  */
#define  GPIO_CR_CNF_INPUT_FLOATING ((uint32_t)GPIO_CRL_CNF0_0) /*!< 01: Floating input (reset state)  */
#define  GPIO_CR_CNF_INPUT_PU_PD    ((uint32_t)GPIO_CRL_CNF0_1) /*!< 10: Input with pull-up / pull-down  */
#define  GPIO_CR_CNF_GP_OUTPUT_PP   ((uint32_t)0x00000000) /*!< 00: General purpose output push-pull  */
#define  GPIO_CR_CNF_GP_OUTPUT_OD   ((uint32_t)GPIO_CRL_CNF0_0) /*!< 01: General purpose output Open-drain  */
#define  GPIO_CR_CNF_AF_OUTPUT_PP   ((uint32_t)GPIO_CRL_CNF0_1) /*!< 10: Alternate function output Push-pull  */
#define  GPIO_CR_CNF_AF_OUTPUT_OD   ((uint32_t)GPIO_CRL_CNF0_0|GPIO_CRL_CNF0_1) /*!< 11: Alternate function output Open-drain  */

#define  GPIO_CR_MODE_OUTPUT_50   (GPIO_CRL_MODE0_0|GPIO_CRL_MODE0_1) 
#define  GPIO_CR_MODE_OUTPUT_10   (GPIO_CRL_MODE0_0) 
#define  GPIO_CR_MODE_OUTPUT_2   (GPIO_CRL_MODE0_1) 

#define BASE_TO_BB_BASE(Base) ((base-PERIPH_BASE)*32+PERIPH_BB_BASE)

class Pin {
private:
	GPIO_TypeDef*	m_gpio;
	uint8_t	m_pin;
public:
	explicit Pin(GPIO_TypeDef* gpio,uint8_t pin);
	void toggle();
	void set();
	void clear();
	bool read() const;
	void configure_output_pp();
	void configure_input_f();
	void configure_input_pu();
	void configure_af_pp();
	void configure_input_it(bool rising,bool falling);
};

static inline void gpio_set(GPIO_TypeDef* gpio,uint8_t pin) {
	gpio->BSRR = (1<<pin);
}
static inline void gpio_clear(GPIO_TypeDef* gpio,uint8_t pin) {
	gpio->BRR = (1<<pin);
}
static inline bool gpio_read(GPIO_TypeDef* gpio,uint8_t pin) {
	return gpio->IDR & (1<<pin);
}
static inline void gpio_configure_af_pp(GPIO_TypeDef* gpio,uint8_t pin) {
	__IO uint32_t *configregister = (pin < 8) ? &gpio->CRL : &gpio->CRH;
	uint32_t registeroffset = (pin < 8) ? (pin << 2) : ((pin - 8) << 2);
	uint32_t config = GPIO_CR_MODE_OUTPUT_50 | GPIO_CR_CNF_AF_OUTPUT_PP;
	MODIFY_REG((*configregister), ((GPIO_CRL_MODE0 | GPIO_CRL_CNF0) << registeroffset ), (config << registeroffset));
}
static inline void gpio_configure_output_pp(GPIO_TypeDef* gpio,uint8_t pin) {
	__IO uint32_t *configregister = (pin < 8) ? &gpio->CRL : &gpio->CRH;
	uint32_t registeroffset = (pin < 8) ? (pin << 2) : ((pin - 8) << 2);
	uint32_t config = GPIO_CR_MODE_OUTPUT_50 | GPIO_CR_CNF_GP_OUTPUT_PP;
	MODIFY_REG((*configregister), ((GPIO_CRL_MODE0 | GPIO_CRL_CNF0) << registeroffset ), (config << registeroffset));
}
static inline void gpio_configure_input_f(GPIO_TypeDef* gpio,uint8_t pin) {
	__IO uint32_t *configregister = (pin < 8) ? &gpio->CRL : &gpio->CRH;
	uint32_t registeroffset = (pin < 8) ? (pin << 2) : ((pin - 8) << 2);
	uint32_t config = GPIO_CR_CNF_INPUT_FLOATING | GPIO_CR_MODE_INPUT;
	MODIFY_REG((*configregister), ((GPIO_CRL_MODE0 | GPIO_CRL_CNF0) << registeroffset ), (config << registeroffset));	
}
static inline void gpio_configure_input_pu(GPIO_TypeDef* gpio,uint8_t pin) {
	__IO uint32_t *configregister = (pin < 8) ? &gpio->CRL : &gpio->CRH;
	uint32_t registeroffset = (pin < 8) ? (pin << 2) : ((pin - 8) << 2);
	uint32_t config = GPIO_CR_CNF_INPUT_PU_PD | GPIO_CR_MODE_INPUT;
	MODIFY_REG((*configregister), ((GPIO_CRL_MODE0 | GPIO_CRL_CNF0) << registeroffset ), (config << registeroffset));
	gpio_set(gpio,pin);	
}

struct PortA { 
	static GPIO_TypeDef* get() { return GPIOA; } 
	static const uint32_t exti_index = 0; 
	static const uint32_t base = GPIOA_BASE;
	static const uint32_t bb_base = BASE_TO_BB_BASE(GPIOA_BASE);
};
struct PortB { 
	static GPIO_TypeDef* get() { return GPIOB; } 
	static const uint32_t exti_index = 1; 
	static const uint32_t base = GPIOB_BASE;
	static const uint32_t bb_base = BASE_TO_BB_BASE(GPIOB_BASE);
};
struct PortC { 
	static GPIO_TypeDef* get() { return GPIOC; } 
	static const uint32_t exti_index = 2;
	static const uint32_t base = GPIOC_BASE;
	static const uint32_t bb_base = BASE_TO_BB_BASE(GPIOC_BASE);
};

template <class Port,uint8_t pin>
struct StaticPin {
	static void configure_af_pp() {
		gpio_configure_af_pp(Port::get(),pin);
	}
	static void configure_output_pp() {
		gpio_configure_output_pp(Port::get(),pin);
	}
	static void configure_input_pu() {
		gpio_configure_input_pu(Port::get(),pin);
	}
	static void configure_input_f() {
		gpio_configure_input_f(Port::get(),pin);
	}

	static void configure_input_it(bool rising,bool falling) {
		const uint32_t offset = (pin & 0x3) * 4;
		const uint32_t index = Port::exti_index;
		MODIFY_REG(AFIO->EXTICR[pin>>2],0x0F << offset, index);
		SET_BIT(EXTI->IMR, 1<<pin);
		MODIFY_REG(EXTI->RTSR,1<<pin,rising ? (1<<pin) : 0);
		MODIFY_REG(EXTI->FTSR,1<<pin,falling ? (1<<pin) : 0);
	}
 
	static void toggle() { Port::get()->ODR ^= (1<<pin); }
	static void set() { Port::get()->BSRR = (1<<pin); }
	static void clear() { Port::get()->BRR = (1<<pin); }
	static bool read() { return Port::get()->IDR & (1<<pin); }
	static void write(bool state) {
		Port::get()->BSRR = (1<<pin) << (state ? 0 : 16);
	}
};

#endif /*_GPIO_H_INCLUDED_*/