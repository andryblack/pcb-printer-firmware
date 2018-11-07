#include "gpio.h"


Pin::Pin(GPIO_TypeDef* gpio,uint8_t pin) : m_gpio(gpio),m_pin(pin) {

}

void Pin::configure_output_pp() {
	gpio_configure_output_pp(m_gpio,m_pin);
}

void Pin::configure_input_f() {
	gpio_configure_input_f(m_gpio,m_pin);
}

void Pin::configure_input_pu() {
	gpio_configure_input_pu(m_gpio,m_pin);
}

void Pin::configure_input_it(bool rising,bool falling) {
	uint32_t offset = (m_pin & 0x3) * 4;
	uint32_t index = 
					(m_gpio == GPIOA) ? 0 : 
					((m_gpio == GPIOB) ? 1 :
					((m_gpio == GPIOC) ? 2 :
					((m_gpio == GPIOD) ? 3 : 4)));
	MODIFY_REG(AFIO->EXTICR[m_pin>>2],0x0F << offset, index);
	SET_BIT(EXTI->IMR, 1<<m_pin);
	MODIFY_REG(EXTI->RTSR,1<<m_pin,rising ? (1<<m_pin) : 0);
	MODIFY_REG(EXTI->FTSR,1<<m_pin,falling ? (1<<m_pin) : 0);
}

void Pin::configure_af_pp() {
	gpio_configure_af_pp(m_gpio,m_pin);
}

void Pin::toggle() {
	m_gpio->ODR ^= (1<<m_pin);
}

void Pin::set() {
	m_gpio->ODR |= (1<<m_pin);
}
void Pin::clear() {
	m_gpio->ODR &= ~(1<<m_pin);
}

bool Pin::read() const {
	return m_gpio->IDR & (1<<m_pin);
}