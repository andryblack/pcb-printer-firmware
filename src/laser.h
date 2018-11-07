#ifndef _LASER_H_INCLUDED_
#define _LASER_H_INCLUDED_

#include <cstdint>

struct Laser {
	static int32_t dt;
	static void init();
	static void prepare_next();
	static void start();
	static void stop();
	static void process();

	static void setup_print(uint16_t flash_time);
	static void setup_pwm(uint16_t pwm);

	static void start_print(const uint8_t* data,uint32_t size);
};

#endif /*_LASER_H_INCLUDED_*/