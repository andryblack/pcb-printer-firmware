#ifndef _CONTROLLER_H_INCLUDED_
#define _CONTROLLER_H_INCLUDED_

#include <cstdint>

struct Controller {
	
	static void move_r( uint32_t speed, int32_t target );
	static void move_l( uint32_t speed, int32_t target );
	static void move( uint32_t speed, int32_t target );
	static void stop();
	static bool move_complete();
	static void process();
private:
	volatile static enum State {
		S_NONE,
		S_MOVE_Y,
		S_MOVE_TO_START,
		S_PRINT,
		S_END_PRINT
	} state;
	static void start_print();
	static void setup_laser();
	static void stop_laser();
};

#endif /*_CONTROLLER_H_INCLUDED_*/