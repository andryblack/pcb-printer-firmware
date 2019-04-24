#include "stepper.h"
#include "config.h"

int32_t Stepper::pos = 0;

static uint32_t max_speed = 700;
static uint32_t min_speed = 50;
static uint32_t acceleration = 700;
static uint32_t	decceleration = 1200;
static uint32_t speed_down_steps = 200;

static const uint32_t STEPPER_CNT = 256;

uint32_t get_ms_timer();

static volatile enum State {
		S_NONE,
		S_MOVE,
} state = S_NONE;

static int32_t target = 0;
static uint32_t speed = min_speed;
static int32_t dir = 0;
static bool dir_config = false;

static inline uint32_t _abs(int32_t dif) {
	return dif < 0 ? uint32_t(-dif) : uint32_t(dif);
}

static volatile uint32_t time_per_step = 0;
static volatile uint32_t move_time = 0;
static uint32_t last_move_time = 0;

static inline void set_speed( uint32_t speed ) {
	time_per_step = 1000000 / speed;
	timer_stepper::configure( timer_stepper::clk/speed/STEPPER_CNT - 1, STEPPER_CNT - 1 );
}

void Stepper::set_param(ParamID param,uint32_t value) {
	switch (param) {
		case PARAM_STEPPER_MAX_SPEED: 	max_speed = value; break;
		case PARAM_STEPPER_START_SPEED:	min_speed = value; break;
		case PARAM_STEPPER_ACCEL:		acceleration = value; break;
		case PARAM_STEPPER_DECCEL:		decceleration = value; break;
		case PARAM_STEPPER_STOP_STEPS:	speed_down_steps = value; break;
	}
}

void Stepper::init() {

	pin_stepper_step::configure_af_pp();
	pin_stepper_dir::configure_output_pp();
	pin_stepper_enable::configure_output_pp();

	pin_stepper_enable::set();
	timer_stepper::enable_clock();
	timer_stepper::get()->CR1 |= TIM_CR1_ARPE | TIM_CR1_URS;
	set_speed(min_speed);
	timer_stepper::configure_pwm(STEPPER_PWM_CHANNEL);
	timer_stepper::set_pwm(STEPPER_PWM_CHANNEL,2);

	timer_stepper::get()->DIER = TIM_DIER_CC2IE;
	timer_stepper::get()->CCR2 = STEPPER_CNT/4;

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
	set_speed( speed );
	timer_stepper::get()->CNT = 0;
	timer_stepper::get()->EGR = TIM_EGR_UG;
	move_time = last_move_time = 0;
	state = S_MOVE;
	timer_stepper::start();
}


void Stepper::process() {
	if (state == S_MOVE) {
		uint32_t now = move_time;
		uint32_t dt = now - last_move_time;
		if (_abs(target-Stepper::pos) < speed_down_steps) {
			uint32_t deccel = decceleration * dt / 1000000; // x/1000 * us  
			if (deccel) {
				last_move_time = now;
				if (speed > deccel) {
					speed -= deccel;
				} else {
					speed = 0;
				}
				if (speed < min_speed) {
					speed = min_speed;
				}
				set_speed(speed);
			}
		} else if (speed < max_speed) {
			uint32_t accel = acceleration * dt / 1000000;
			if (accel) {
				last_move_time = now;
				speed += acceleration * dt / 1000000;
				if (speed > max_speed) {
					speed = max_speed;
				}
				set_speed(speed);
			}
		}
	}
}

void process_move() {
	Stepper::pos += dir;
	if (state == S_MOVE) {
		if (Stepper::pos == target) {
			Stepper::stop();
		} 
	}
}

extern "C" void STEPPER_TIMER_IRQ() {
	TIM_TypeDef* timer = timer_stepper::get();
	__IO uint32_t sr = timer->SR;
	timer->SR = sr & ~(TIM_SR_CC2IF);
	if (sr&TIM_SR_CC2IF) {
		move_time += time_per_step;
		process_move();
	} 
}