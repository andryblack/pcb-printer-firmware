#ifndef _STEPPER_H_INCLUDED_
#define _STEPPER_H_INCLUDED_

#include <cstdint>
#include "protocol.h"

struct Stepper {
	static int32_t pos;
	static void init();
	static void set_zero();
	static void stop();
	static void move(int32_t p);
	static bool is_stopped();
	static void process();
	static void set_param(ParamID param,uint32_t value);
};

#endif /*_STEPPER_H_INCLUDED_*/