#include "laser.h"
#include "config.h"
#include "dma.h"

static uint32_t print_flash_time = 1;

static DMAChannel dma(dma_channel_laser);

int32_t Laser::dt = 60000;

static const uint32_t LASER_PWM_RESOLUTION = 1024;
static const uint32_t LASER_PWM_FREQ = 20000;

static const uint8_t* laser_print_data = 0;
static uint32_t laser_print_size = 0;
static uint32_t laser_print_pos = 0;
static uint8_t laser_print_bit = 0;

void Laser::init() {

	pin_laser::configure_af_pp();

	//dma.configure_m2p(3,DMAChannel::DS8,DMAChannel::DS16,true,false);

	timer_laser::enable_clock();
	timer_laser::configure(timer_laser::clk/LASER_FREQ-1,0xFFFF); // 65535 us = 65ms

	TIM_TypeDef* timer = timer_laser::get();

	timer->CR1 &= ~TIM_CR1_OPM;
	//timer->CR1 |= TIM_CR1_OPM;

	timer->CR1 &= ~TIM_CR1_URS;
	//timer->CR1 |= TIM_CR1_URS;

	timer->SMCR &= ~TIM_SMCR_TS;
	timer->SMCR |= TimerMasterSlave<timer_laser,timer_enc>::TS;  // slave from encoder
	timer->SMCR &= ~TIM_SMCR_SMS;
	timer->SMCR |= /*TIM_SMCR_SMS_1 | */TIM_SMCR_SMS_2; // Reset mode

	timer->EGR |= TIM_EGR_UG;

	
	timer->CCMR1 = /*TIM_CCMR1_OC1FE |*/ TIM_CCMR1_OC1PE | TIM_CCMR1_OC1M_1 | TIM_CCMR1_OC1M_2 /*| // PWM1
					TIM_CCMR1_OC2M_0 | TIM_CCMR1_OC2M_1 | TIM_CCMR1_OC2M_2*/; // PWM2

	timer->CCMR2 =
		TIM_CCMR2_CC3S_0 | TIM_CCMR2_CC3S_1; // CH3 -> capture on trigger

	timer->CCR2 = 65000; 
	

	NVIC_SetPriority (LASER_TIMER_IRQn,LASER_TIMER_IRQ_PRI);		
	NVIC_EnableIRQ(LASER_TIMER_IRQn);	

}

void Laser::setup_print( uint16_t flash_time ) {
	timer_laser::stop();
	timer_laser::configure(timer_laser::clk/LASER_FREQ-1,0xFFFF); // 65535 us = 65ms

	print_flash_time = flash_time;

	TIM_TypeDef* timer = timer_laser::get();

	timer->DIER |= TIM_DIER_TIE | TIM_DIER_CC2IE;

	timer->CCER |=								
		TIM_CCER_CC1E | TIM_CCER_CC3E;

	timer->CCR1 = 0;
	timer->CNT = 0;
	timer->EGR |= TIM_EGR_UG;
	timer_laser::start();
}

void Laser::setup_pwm(uint16_t pwm_value) {
	timer_laser::configure((timer_laser::clk/LASER_PWM_FREQ/LASER_PWM_RESOLUTION)-1,LASER_PWM_RESOLUTION*4-1); // 65535 us = 65ms

	TIM_TypeDef* timer = timer_laser::get();

	timer->CCR1 = pwm_value;
	timer->DIER &= ~( TIM_DIER_TIE | TIM_DIER_CC2IE );

	timer->CCER &= ~( TIM_CCER_CC3E );
	timer->CCER |=								
		TIM_CCER_CC1E;

	timer_laser::start();
}

void Laser::start_print(const uint8_t* data,uint32_t size) {
	laser_print_bit = 0x80;
	laser_print_data = data;
	laser_print_size = size;
	laser_print_pos = 0;
	// prepare_next();
}

void Laser::prepare_next() {
	if (laser_print_data) {
		timer_laser::get()->CCR1 = (laser_print_data[laser_print_pos] & laser_print_bit) ? print_flash_time : 0;
		laser_print_bit = laser_print_bit >> 1;
		if (laser_print_bit == 0) {
			++laser_print_pos;
			laser_print_bit = 0x80;
			if (laser_print_pos >= laser_print_size) {
				laser_print_data = 0;
			}
		}
	} else {
		timer_laser::get()->CCR1 = 0;
	}
}

void Laser::start() {
	TIM_TypeDef* timer = timer_laser::get();
	timer->CCR1 = 0;
	timer->CNT = 0;
	timer->EGR |= TIM_EGR_UG;
	timer->CCER |=								
		TIM_CCER_CC1E | TIM_CCER_CC3E;
	prepare_next();
	timer_laser::start();
}

void Laser::stop() {
	TIM_TypeDef* timer = timer_laser::get();
	timer_laser::stop();
	timer->CCR1 = 0;
	timer->CCER &=								
		~TIM_CCER_CC1E;
}

void Laser::process() {
	TIM_TypeDef* timer = timer_laser::get();
	uint32_t sr = timer->SR;
	if (sr & TIM_SR_CC3IF) {
		uint32_t ddt = timer->CCR3;
		dt = (dt + ddt) >> 1;
	} else if (timer->CNT >= 60000) {
		dt = 60000;
	}
}


extern "C" void LASER_TIMER_IRQ() {
	TIM_TypeDef* timer = timer_laser::get();
	uint32_t sr = timer->SR;
	timer->SR = sr & ~(TIM_SR_TIF | TIM_SR_CC2IF);
	
	if (sr & TIM_SR_TIF) {
		Laser::prepare_next();		
	} else if (sr & TIM_SR_CC2IF) {
		timer->CNT = 60000;
	}

}