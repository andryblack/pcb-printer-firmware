#ifndef _ENCODER_H_INCLUDED_
#define _ENCODER_H_INCLUDED_

#include <cstdint>

struct Encoder {
	typedef void (*event_t)();

	static event_t target_event;

	static void init();
	static void set_zero();
	static int32_t get_pos();
	static void setup_r();
	static void setup_l();
	static void setup_target(int32_t pos, event_t evt);
};

#endif /*_ENCODER_H_INCLUDED_*/