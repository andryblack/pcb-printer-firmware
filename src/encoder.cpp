#include "encoder.h"
#include "timer.h"
#include "gpio.h"

#include "config.h"
#include "motor.h"

static int16_t dir = 0;

static void empy_event() {

}

Encoder::event_t Encoder::target_event = &empy_event;

static const uint32_t CH4_CONFIG = /*TIM_CCMR2_OC4FE |*/ TIM_CCMR2_OC4M_0 /*| TIM_CCMR2_OC4M_1 | TIM_CCMR2_OC4M_2*/;	// match
static const uint32_t CH3_RESET = TIM_CCMR2_OC3M_2;
static const uint32_t CH3_CONFIG = /*TIM_CCMR2_OC3FE |*/ TIM_CCMR2_OC3M_0  /*| TIM_CCMR2_OC3M_1 | TIM_CCMR2_OC3M_2*/; // match  

static void update_encoder() {
	TIM_TypeDef* timer = timer_enc::get();
	int16_t cnt = timer->CNT;
	timer->CCMR2 = CH4_CONFIG | CH3_RESET;
	timer->CCR3 = uint16_t(cnt + dir);
	timer->CCMR2 = CH4_CONFIG | CH3_CONFIG;
	//pin_led::toggle();
	//timer->CCR4 = cnt - 1;
	//timer->SR &= ~(TIM_SR_CC3IF | TIM_SR_CC4IF);
}

void Encoder::init() {
	pin_enc_a::configure_input_f();
	pin_enc_b::configure_input_f();

	timer_enc::enable_clock();
	TIM_TypeDef* timer = timer_enc::get();
	timer->ARR = 0xffff;
	timer->PSC = 0;

	timer->SMCR = 
		TIM_SMCR_SMS_1 | TIM_SMCR_SMS_0;  // enc1 | enc2


	timer->CR2 &= ~TIM_CR2_MMS;
	timer->CR2 |= TIM_CR2_MMS_2|TIM_CR2_MMS_1; // OC3REF -> TRGO

	timer->CCMR1=
		TIM_CCMR1_CC1S_0|TIM_CCMR1_CC2S_0|      // pins
		/*TIM_CCMR1_IC1F_3|TIM_CCMR1_IC1F_2|*/TIM_CCMR1_IC1F_1|TIM_CCMR1_IC1F_0|
		/*TIM_CCMR1_IC2F_3|TIM_CCMR1_IC2F_2|*/TIM_CCMR1_IC2F_1|TIM_CCMR1_IC2F_0;		// filters

	timer->CCMR2 = CH4_CONFIG | CH3_CONFIG;
		
	timer->DIER=
		TIM_DIER_CC3IE|TIM_DIER_CC4IE;			// compare interrupt


	NVIC_SetPriority (ENC_TIMER_IRQn,ENC_TIMER_IRQ_PRI);		
	NVIC_EnableIRQ(ENC_TIMER_IRQn);	

	update_encoder();

	// timer->CCER|=								
	// 	TIM_CCER_CC3E | TIM_CCER_CC4E;							// capture 3,4 channels

	//timer_enc::start();
	
}

void Encoder::setup_target(int32_t pos, event_t evt) {
	target_event = empy_event;
	uint16_t rpos = uint32_t(pos);
	timer_enc::get()->CCR4 = rpos;
	target_event = evt;
}

void Encoder::setup_r() {
	dir = 1;
	update_encoder();
}

void Encoder::setup_l() {
	dir = -1;
	update_encoder();
}


void Encoder::set_zero() {
	TIM_TypeDef* timer = timer_enc::get();
	timer_enc::stop();
	timer->CNT = 0;
	update_encoder();
	//timer->SR = 0;
	timer_enc::start();
}

int32_t Encoder::get_pos() {
	int16_t res = timer_enc::get()->CNT;
	return res;
}


extern "C" void ENC_TIMER_IRQ() {
	TIM_TypeDef* timer = timer_enc::get();
	__IO uint32_t sr = timer->SR;
	timer->SR = sr & ~(TIM_SR_CC3IF|TIM_SR_CC4IF);
	if (sr&TIM_SR_CC3IF) {
		//pin_test::toggle();
		update_encoder();
	} 
	if (sr&TIM_SR_CC4IF) {
		Encoder::target_event();
	}
}
