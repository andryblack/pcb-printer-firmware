#ifndef _CRC8_H_INCLUDED_
#define _CRC8_H_INCLUDED_

#include <cstdint>

struct CRC8 {

	static uint8_t calc(const void* data,uint16_t size);
	static bool check(const void* data,uint16_t size,uint8_t checksum);

	static uint8_t begin();
	static uint8_t update(uint8_t d,const void* data,uint16_t size);
	static uint8_t end(uint8_t d);
};

#endif /*_CRC8_H_INCLUDED_*/