#include "stepper.h"
#include "config.h"

int32_t Stepper::pos = 0;

static const int32_t max_speed = 700;
static const int32_t min_speed = 50;
static const int32_t acceleration = 2;
static const int32_t speed_down_steps = 100;

uint32_t get_ms_timer();

static volatile enum State {
		S_NONE,
		S_MOVE,
} state = S_NONE;

static int32_t target = 0;
static int32_t speed = min_speed;
static int32_t dir = 0;
static bool dir_config = false;
static uint32_t last_ms = 0;

static int32_t _abs(int32_t dif) {
	return dif < 0 ? -dif : dif;
}

static uint32_t speed_to_arr(int32_t speed) {
	int32_t res = STEPPER_FREQ / speed;
	if (res < 4) {
		return 4;
	}
	return res - 1;
}

void Stepper::init() {

	pin_stepper_step::configure_af_pp();
	pin_stepper_dir::configure_output_pp();
	pin_stepper_enable::configure_output_pp();

	pin_stepper_enable::set();
	timer_stepper::enable_clock();
	timer_stepper::get()->CR1 |= TIM_CR1_ARPE | TIM_CR1_URS;
	timer_stepper::configure(timer_stepper::clk/STEPPER_FREQ-1,speed_to_arr(min_speed));
	timer_stepper::configure_pwm(STEPPER_PWM_CHANNEL);
	timer_stepper::set_pwm(STEPPER_PWM_CHANNEL,2);

	timer_stepper::get()->DIER = TIM_DIER_CC2IE;
	timer_stepper::get()->CCR2 = 3;

	NVIC_SetPriority (STEPPER_TIMER_IRQn,STEPPER_TIMER_IRQ_PRI);		
	NVIC_EnableIRQ(STEPPER_TIMER_IRQn);	


}

void Stepper::set_zero() {
	pos = 0;
}

void Stepper::stop() {
	timer_stepper::stop();
	state = S_NONE;
	dir = 0;
}

bool Stepper::is_stopped() {
	return state == S_NONE;
}

void Stepper::move(int32_t p) {
	target = p;
	speed = min_speed;
	if (target > pos) {
		dir = 1;
		pin_stepper_dir::write(dir_config);
	} else if (target < pos) {
		dir = -1;
		pin_stepper_dir::write(!dir_config);
	} else {
		stop();
		dir = 0;
		return;
	}
	pin_stepper_enable::clear();
	timer_stepper::get()->ARR = speed_to_arr(speed);
	timer_stepper::get()->CNT = 0;
	timer_stepper::get()->EGR = TIM_EGR_UG;
	last_ms = get_ms_timer();
	state = S_MOVE;
	timer_stepper::start();
}

void Stepper::process() {
	if (state == S_MOVE) {
		uint32_t now = get_ms_timer();
		if ((now-last_ms)>=10) {
			last_ms = now;
			if (_abs(target-Stepper::pos) < speed_down_steps) {
				speed -= acceleration * 4;
				if (speed < min_speed) {
					speed = min_speed;
				}
				timer_stepper::get()->ARR = speed_to_arr(speed);
			} else if (speed < max_speed) {
				speed += acceleration;
				if (speed > max_speed) {
					speed = max_speed;
				}
				timer_stepper::get()->ARR = speed_to_arr(speed);
			}
		}
	}
}

void process_move() {
	Stepper::pos = Stepper::pos + dir;
	if (state == S_MOVE) {
		if (Stepper::pos == target) {
			timer_stepper::stop();
			state = S_NONE;
		} 
	}
}

extern "C" void STEPPER_TIMER_IRQ() {
	TIM_TypeDef* timer = timer_stepper::get();
	__IO uint32_t sr = timer->SR;
	timer->SR = sr & ~(TIM_SR_CC2IF);
	if (sr&TIM_SR_CC2IF) {
		process_move();
	} 
}