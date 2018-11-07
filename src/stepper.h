#ifndef _STEPPER_H_INCLUDED_
#define _STEPPER_H_INCLUDED_

#include <cstdint>

struct Stepper {
	static int32_t pos;
	static void init();
	static void set_zero();
	static void stop();
	static void move(int32_t p);
	static bool is_stopped();
	static void process();
};

#endif /*_STEPPER_H_INCLUDED_*/