#ifndef _TIMER_H_INCLUDED_
#define _TIMER_H_INCLUDED_


#include <stm32f1xx.h>
#include "hw_config.h"

struct TimerTIM1 {
	static void enable_clock() {
		SET_BIT(RCC->APB2ENR, RCC_APB2ENR_TIM1EN);
		volatile uint32_t tmpreg = READ_BIT(RCC->APB2ENR, RCC_APB2ENR_TIM1EN);
		(void)tmpreg;
		SET_BIT(TIM1->BDTR,TIM_BDTR_MOE|TIM_BDTR_AOE);
	}
	static TIM_TypeDef* get() { return TIM1; }
	static const uint32_t clk = FREQ_APB2_TIMER;
};

struct TimerTIM2 {
	static void enable_clock() {
		SET_BIT(RCC->APB1ENR, RCC_APB1ENR_TIM2EN);
		volatile uint32_t tmpreg = READ_BIT(RCC->APB1ENR, RCC_APB1ENR_TIM2EN);
		(void)tmpreg;
	}
	static TIM_TypeDef* get() { return TIM2; }
	static const uint32_t clk = FREQ_APB1_TIMER;
};

struct TimerTIM3 {
	static void enable_clock() {
		SET_BIT(RCC->APB1ENR, RCC_APB1ENR_TIM3EN);
		volatile uint32_t tmpreg = READ_BIT(RCC->APB1ENR, RCC_APB1ENR_TIM3EN);
		(void)tmpreg;
	}
	static TIM_TypeDef* get() { return TIM3; }
	static const uint32_t clk = FREQ_APB1_TIMER;
};

struct TimerTIM4 {
	static void enable_clock() {
		SET_BIT(RCC->APB1ENR, RCC_APB1ENR_TIM4EN);
		volatile uint32_t tmpreg = READ_BIT(RCC->APB1ENR, RCC_APB1ENR_TIM4EN);
		(void)tmpreg;
	}
	static TIM_TypeDef* get() { return TIM4; }
	static const uint32_t clk = FREQ_APB1_TIMER;
};

template <class Slave,class Master>
struct TimerMasterSlave;

template <>
struct TimerMasterSlave<TimerTIM2,TimerTIM1> {
	static const uint32_t TS = 0;
};
template <>
struct TimerMasterSlave<TimerTIM3,TimerTIM1> {
	static const uint32_t TS = 0;
};
template <>
struct TimerMasterSlave<TimerTIM4,TimerTIM1> {
	static const uint32_t TS = 0;
};

template <>
struct TimerMasterSlave<TimerTIM3,TimerTIM2> {
	static const uint32_t TS = TIM_SMCR_TS_0;
};
template <>
struct TimerMasterSlave<TimerTIM4,TimerTIM2> {
	static const uint32_t TS = TIM_SMCR_TS_0;
};

template <>
struct TimerMasterSlave<TimerTIM2,TimerTIM3> {
	static const uint32_t TS = TIM_SMCR_TS_1;
};
template <>
struct TimerMasterSlave<TimerTIM4,TimerTIM3> {
	static const uint32_t TS = TIM_SMCR_TS_1;
};

template <>
struct TimerMasterSlave<TimerTIM2,TimerTIM4> {
	static const uint32_t TS = TIM_SMCR_TS_0 | TIM_SMCR_TS_1;
};
template <>
struct TimerMasterSlave<TimerTIM3,TimerTIM4> {
	static const uint32_t TS = TIM_SMCR_TS_0 | TIM_SMCR_TS_1;
};


void timer_configure_pwm(TIM_TypeDef* tim,uint32_t ch);

template <class Timer>
struct StaticTimer : Timer {
	using Timer::get;
	static void configure(uint32_t prescaler,uint32_t cnt) {
		get()->PSC = prescaler;
		get()->ARR = cnt;
	}
	static void configure_pwm(uint32_t ch) {
		timer_configure_pwm(get(),ch);
	}
	static void set_pwm(uint32_t ch,uint32_t val) {
		__IO uint32_t* CCR = &(get()->CCR1);
		CCR[ch-1] = val;
	}
	static void start() {
		SET_BIT(get()->CR1,TIM_CR1_CEN);
	}
	static void stop() {
		CLEAR_BIT(get()->CR1,TIM_CR1_CEN);
	}
};

template <class Slave,class Master>
struct TimerMasterSlave< StaticTimer<Slave>,
					 StaticTimer<Master> > {
	static const uint32_t TS = TimerMasterSlave<Slave,Master>::TS;
};


class Timer {
private:
	TIM_TypeDef* m_tim;
public:
	explicit Timer(TIM_TypeDef* tim);
	template <class T>
	explicit Timer() : m_tim(T::get()) {}
	void configure(uint32_t prescaler,uint32_t reload);
	void configure_pwm(uint8_t ch);
	void set_pwm(uint8_t ch,uint32_t val);
	void start();
	void stop();
};




#endif /*_TIMER_H_INCLUDED_*/