#include "queue.h"

uint32_t Queue::w_pos = 0;
uint32_t Queue::r_pos = 0;
uint32_t Queue::size = 0;

print_t* Queue::write() {
	print_t* res = &mem_overlay.print_queue[w_pos];
	return res;
}
print_t* Queue::read() {
	print_t* res = &mem_overlay.print_queue[r_pos];
	return res;
}

void Queue::push() {
	++size;
	w_pos = (w_pos + 1) % max_print_queue;
}

void Queue::pop() {
	if (size) {
		--size;
		r_pos = (r_pos + 1) % max_print_queue;
	}
}

void Queue::reset() {
	w_pos = 0;
	r_pos = 0;
	size = 0;
}