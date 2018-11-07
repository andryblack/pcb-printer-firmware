#ifndef _SLIP_H_INCLUDED_
#define _SLIP_H_INCLUDED_

#include <cstdint>

static const uint8_t SLIP_SOF = 0xc0;
static const uint8_t SLIP_DB = 0xdb;
static const uint8_t SLIP_DC = 0xdc;
static const uint8_t SLIP_DD = 0xdd;

class SlipEncoder {
private:
	uint8_t*	m_dst;
	uint16_t 	m_bytes;
	uint16_t 	m_max_bytes;
	void write_byte(uint8_t b);
public:
	explicit SlipEncoder(uint8_t* dst,uint16_t max);
	void begin();
	void write(const void* data,uint16_t size);
	uint16_t end();
};

class SlipDecoder {
private:
	enum State {
		S_UNKNOWN,
		S_X_DB,
		S_ACTIVE
	} m_state;
	uint8_t*	m_dst;
	uint16_t	m_bytes;
	uint16_t 	m_max_bytes;
	void on_end();
	void put_byte(uint8_t b);
public:
	explicit SlipDecoder(uint8_t* dst,uint16_t max);
	void reset();
	void on_data(const void* data,uint16_t size);
	void on_data(uint8_t d);
	virtual void on_packet(const void* data,uint16_t size) = 0;
};

#endif /*_SLIP_H_INCLUDED_*/