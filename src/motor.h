#ifndef _MOTOR_H_INCLUDED_
#define _MOTOR_H_INCLUDED_

#include <cstdint>

struct Motor {

	static int32_t target_speed;
	static bool write_speed;
	static uint32_t write_speed_pos;
	
	static enum State {
		S_STOPPED,
		S_MOVE,
		S_STOPPING,
	} state;

	static void init();
	static void stop();
	static void set_dir_r();
	static void set_dir_l();
	static void move();
	static void set_target_speed(uint32_t speed);
	static void process_PID();
	static void process();
	static bool is_move() { return state == S_MOVE; }
	static bool is_stopped() { return state == S_STOPPED; }

	static void start_write_speed();
	static void setup_PID(float P,float I,float D);
};

#endif /*_MOTOR_H_INCLUDED_*/