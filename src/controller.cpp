#include "controller.h"
#include "encoder.h"
#include "motor.h"
#include "laser.h"
#include "queue.h"
#include "config.h"
#include "stepper.h"

volatile Controller::State Controller::state = Controller::S_NONE;

void Controller::move_r( uint32_t speed, int32_t target ) {
	Motor::set_target_speed(speed);
	Encoder::setup_r();
	Encoder::setup_target(target,&Motor::stop);
	Motor::set_dir_r();
	Motor::move();
}

void Controller::move_l(uint32_t speed, int32_t target ) {
	Motor::set_target_speed(speed);
	Encoder::setup_l();
	Encoder::setup_target(target,&Motor::stop);
	Motor::set_dir_l();
	Motor::move();
}

void Controller::stop() {
	Motor::stop();
}

static int32_t stop_target = 0;
static const uint8_t* print_data = 0;
static uint32_t print_size = 0;

void Controller::setup_laser() {
	if (state == S_MOVE_TO_START) {
		state = S_PRINT;
		// setup laset
		Laser::start_print(print_data,print_size);
		Encoder::setup_target(stop_target,&Controller::stop_laser);
	}
}

void Controller::stop_laser() {
	if (state == S_PRINT) {
		state = S_END_PRINT;
		Motor::stop();
	}
}

void Controller::move( uint32_t speed, int32_t target ) {
	if (Encoder::get_pos()>target) {
		move_l(speed,target);
	} else if (Encoder::get_pos() < target) {
		move_r(speed,target);
	}
}

bool Controller::move_complete() {
	return Motor::is_stopped() && Stepper::is_stopped();
}

void Controller::start_print() {
	print_t* print = Queue::read();
	print_data = print->data;
	print_size = print->len;
	state = S_MOVE_TO_START;
	if (int32_t(print->start) > Encoder::get_pos()) {
		stop_target = int32_t(print->start) - 1 + int32_t(print->len)*8;
		Encoder::setup_r();
		Encoder::setup_target(int32_t(print->start)-1,&Controller::setup_laser);
		Motor::set_dir_r();
	} else {
		stop_target = int32_t(print->start) + 1 - int32_t(print->len)*8;
		Encoder::setup_l();
		Encoder::setup_target(int32_t(print->start)+1,&Controller::setup_laser);
		Motor::set_dir_l();
	}

	Motor::set_target_speed(LASER_FREQ/print->speed);
	Motor::move();
}

void Controller::process() {
	Laser::process();
	Motor::process();
	Stepper::process();
	if (state == S_NONE) {
		if (Queue::size) {
			print_t* print = Queue::read();
			if (print->move_y) {
				Stepper::move(Stepper::pos + print->move_y);
				state = S_MOVE_Y;
			} else if (Motor::is_stopped()) {
				start_print();
			}

		}
	} else if (state == S_MOVE_Y) {
		if (Stepper::is_stopped() && Motor::is_stopped() && Queue::size) {
			start_print();
		}
	} else if (state == S_END_PRINT) {
		Queue::pop();
		state = S_NONE;
	}
}