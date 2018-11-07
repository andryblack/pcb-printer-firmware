#ifndef _MEM_OVERLAY_H_INCLUDED_
#define _MEM_OVERLAY_H_INCLUDED_

#include <cstdint>
#include "protocol.h"

static const uint32_t max_speed_samples = 1024;
static const uint32_t max_print_mem = Protocol::MAX_RX_PACKET-sizeof(print_t)-sizeof(header_t)-1;
static const uint32_t max_print_queue = 4;

struct SpeedSample {
	uint16_t dt;
	uint16_t pwm;
}__attribute__((packed));


struct PrintCmdData : print_t {
	uint8_t data[max_print_mem];
}__attribute__((packed));

union MemOverlay {
	SpeedSample speed[max_speed_samples];
	PrintCmdData print_queue[max_print_queue];
};
extern MemOverlay mem_overlay;

#endif /*_MEM_OVERLAY_H_INCLUDED_*/