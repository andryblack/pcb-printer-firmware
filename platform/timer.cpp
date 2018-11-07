#include "timer.h"

void timer_configure_pwm(TIM_TypeDef* tim,uint32_t ch) {
	CLEAR_BIT(tim->CCER,TIM_CCER_CC1E << (4*(ch-1)));	
	if (ch < 3) {
		MODIFY_REG(tim->CCMR1,
			(TIM_CCMR1_OC1PE | TIM_CCMR1_OC1M | TIM_CCMR1_CC1S) << (8*(ch-1)),
			(TIM_CCMR1_OC1PE | (TIM_CCMR1_OC1M_2|TIM_CCMR1_OC1M_1/*|TIM_CCMR1_OC1M_0*/)) << (8*(ch-1)));
	} else {
		MODIFY_REG(tim->CCMR2,
			(TIM_CCMR2_OC3PE | TIM_CCMR2_OC3M) << (8*(ch-3)),
			(TIM_CCMR2_OC3PE | (TIM_CCMR2_OC3M_2|TIM_CCMR2_OC3M_1/*|TIM_CCMR2_OC3M_0*/)) << (8*(ch-3)));
	}
	SET_BIT(tim->CCER,TIM_CCER_CC1E << (4*(ch-1)));	
}

Timer::Timer(TIM_TypeDef* tim) : m_tim(tim) {

}

void Timer::configure(uint32_t prescaler,uint32_t cnt) {
	//m_tim->CCMR1 = TIM_CCMR1_OC1PE           // Preload CCR (buffered)
    //          | (TIM_CCMR1_OC1M_2 | TIM_CCMR1_OC1M_1 | TIM_CCMR1_OC1M_0);
    
	m_tim->PSC = prescaler;
	m_tim->ARR = cnt;
	m_tim->CNT = cnt;
}

void Timer::configure_pwm(uint8_t ch) {
	timer_configure_pwm(m_tim,ch);
}

void Timer::set_pwm(uint8_t ch,uint32_t val) {
	__IO uint32_t* CCR = &m_tim->CCR1;
	//m_tim->CCMR1 | 
	CCR[ch-1] = val;
}

void Timer::start() {
	SET_BIT(m_tim->CR1,TIM_CR1_CEN);
}

void Timer::stop() {
	CLEAR_BIT(m_tim->CR1,TIM_CR1_CEN);
}