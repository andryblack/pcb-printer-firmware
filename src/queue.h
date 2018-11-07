#ifndef _QUEUE_H_INCLUDED_
#define _QUEUE_H_INCLUDED_

#include "mem_overlay.h"

struct Queue {
	static uint32_t w_pos;
	static uint32_t size;
	static uint32_t r_pos;
	static bool has_slot() { return size < max_print_queue; }
	static print_t* write();
	static print_t* read();
	static void push();
	static void pop();
	static void reset();
};

#endif /*_QUEUE_H_INCLUDED_*/