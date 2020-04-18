#include "motor.h"
#include "gpio.h"
#include "timer.h"
#include "config.h"
#include "laser.h"
#include "mem_overlay.h"

// 180p
// 180*4/25.4 = 28.3 evts/mm
// 28346 events / s on 1m/s

static const uint32_t pwm_resolution = 1024;
static const uint32_t pwm_freq = 20000;
static const int32_t min_pwm = 300;
static const int32_t max_pwm = 1000;


static float K_prp = 0.0525f;
static float K_int = 0.003f;//00015f;
static float K_dif = 0.005f;//0001f;

static float I_value = 0;
static int32_t D_value = 0;
static int32_t pwm = 0;

static const uint32_t pid_update_div = 10;
static uint32_t pid_update_cntr = 0;
static volatile bool need_pid_update = false;

static const uint32_t pid_update_dt = 1000000 / (pwm_freq / pid_update_div); // 500us

int32_t Motor::target_speed = 0;
Motor::State Motor::state = S_STOPPED;

static int32_t speed = 0;

#if defined(USE_FAST_MOTOR_CHOPPING)
static uint8_t motor_pwm_channel = 0;
#else
static const uint8_t motor_pwm_channel = MOTOR_PWM_CHANNEL;
#endif

bool Motor::write_speed = false;
uint32_t Motor::write_speed_pos = 0;

void Motor::set_target_speed( uint32_t speed ) {
	target_speed = speed;
}
void Motor::setup_PID(float P,float I,float D) {
	K_prp = P;
	K_int = I;
	K_dif = D;
}
static void reset_pid() {
	I_value = 0;
	D_value = 0;
	pid_update_cntr = 0;
}

void Motor::init() {
	
	#if defined(USE_FAST_MOTOR_CHOPPING)
		pin_motor_a::configure_af_pp();
		pin_motor_b::configure_af_pp();
	#else
		pin_motor_enable::configure_af_pp();

		pin_motor_a::configure_output_pp();
		pin_motor_b::configure_output_pp();
	#endif

	//TIM_TypeDef* tim_pwm = timer_motor::get();

	
	timer_motor::enable_clock();
	SET_BIT(timer_motor::get()->BDTR,TIM_BDTR_MOE|TIM_BDTR_AOE);
	//timer_motor::get()->SMCR = 0;//|= TIM_SMCR_ETPS_0;
	timer_motor::configure((timer_motor::clk/pwm_freq/pwm_resolution)-1,pwm_resolution-1);

	#if defined(USE_FAST_MOTOR_CHOPPING)
		timer_motor::configure_pwm(MOTOR_A_PWM_CHANNEL);
		timer_motor::set_pwm(MOTOR_A_PWM_CHANNEL,0);
		timer_motor::configure_pwm(MOTOR_B_PWM_CHANNEL);
		timer_motor::set_pwm(MOTOR_B_PWM_CHANNEL,0);
	#else
		timer_motor::configure_pwm(MOTOR_PWM_CHANNEL);
		timer_motor::set_pwm(MOTOR_PWM_CHANNEL,0);
	#endif
	
	timer_motor::get()->DIER = TIM_DIER_UIE;

	NVIC_SetPriority (MOTOR_TIMER_IRQn,MOTOR_TIMER_IRQ_PRI);		
	NVIC_EnableIRQ(MOTOR_TIMER_IRQn);	
}

void Motor::stop() {
	if (state != S_MOVE) {
		return;
	}
	#if defined(USE_FAST_MOTOR_CHOPPING)
		timer_motor::set_pwm(MOTOR_A_PWM_CHANNEL,0);	
		timer_motor::set_pwm(MOTOR_B_PWM_CHANNEL,0);	
	#else
		timer_motor::set_pwm(MOTOR_PWM_CHANNEL,0);	
		pin_motor_a::clear();
		pin_motor_b::clear();
	#endif
	
	state = S_STOPPING;
	timer_motor::stop();
	reset_pid();
	write_speed = 0;
	pwm = 0;
	CLEAR_BIT(TIM1->BDTR,TIM_BDTR_MOE);
}

void Motor::set_dir_r() {
	#if defined(USE_FAST_MOTOR_CHOPPING)
		timer_motor::set_pwm(MOTOR_A_PWM_CHANNEL,0);	
		timer_motor::set_pwm(MOTOR_B_PWM_CHANNEL,0);	
		motor_pwm_channel = MOTOR_B_PWM_CHANNEL;
	#else
		pin_motor_a::clear();
		pin_motor_b::set();
	#endif
}

void Motor::set_dir_l() {
	#if defined(USE_FAST_MOTOR_CHOPPING)
		timer_motor::set_pwm(MOTOR_A_PWM_CHANNEL,0);
		timer_motor::set_pwm(MOTOR_B_PWM_CHANNEL,0);
		motor_pwm_channel = MOTOR_A_PWM_CHANNEL;
	#else
		pin_motor_a::set();
		pin_motor_b::clear();
	#endif
}

static uint32_t move_failed_cntr = 0;
void Motor::move() {
	state = S_MOVE;
	reset_pid();
	move_failed_cntr = 0;
	pwm = min_pwm;
	timer_motor::set_pwm(motor_pwm_channel,pwm);
	timer_motor::get()->EGR = TIM_EGR_UG;
	timer_motor::start();
	SET_BIT(TIM1->BDTR,TIM_BDTR_MOE);
}


void Motor::process() {
	if (state == S_STOPPING) {
		if (Laser::dt >= 50000) {
			state = S_STOPPED;
		}
	} else if (state == S_MOVE) {
		if (need_pid_update) {
			need_pid_update = false;
			speed = LASER_FREQ / Laser::dt;
			process_PID( );
			if (Laser::dt > 50000) {
				++move_failed_cntr;
				if (move_failed_cntr > 5000) {
					stop();
					return;
				}
			} else {
				move_failed_cntr = 0;
			}
			if (write_speed && write_speed_pos<max_speed_samples) {
				SpeedSample* sample = &mem_overlay.speed[write_speed_pos];
				sample->dt = Laser::dt;
				sample->pwm = pwm;
				++write_speed_pos;
			}
		}
		
	}
}

void Motor::process_PID() { // us
	if (state != S_MOVE) 
		return;
	float error = target_speed - speed; // pt/s
	float ival = I_value; 
	ival += float(error);
	float dval = (float(error - D_value));
	int32_t vm=	((error*K_prp)					//
				+(ival*K_int)
				+(dval*K_dif)
				);
	I_value = ival;
	D_value = error;
	pwm = vm;
	if (pwm < 0) {
		pwm = 0;
	}
	pwm += min_pwm;

	// pwm += vm;
	// if (pwm < min_pwm) {
	// 	pwm = min_pwm;
	// }
	if (pwm > max_pwm) {
		pwm = max_pwm;
	}

	timer_motor::set_pwm(motor_pwm_channel,pwm);
}

void Motor::start_write_speed() {
	write_speed = true;
	write_speed_pos = 0;
}

extern "C" void MOTOR_TIMER_IRQ() {
	timer_motor::get()->SR &= ~TIM_SR_UIF;
	pid_update_cntr++;
	if (pid_update_cntr >= pid_update_div) {
		pid_update_cntr = 0;
		need_pid_update = true;
	}
}
