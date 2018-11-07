#include "slip.h"
#include "config.h"

SlipEncoder::SlipEncoder(uint8_t* dst,uint16_t max) :
 m_dst(dst),
 m_bytes(0),
 m_max_bytes(max) {

}

void SlipEncoder::write_byte(uint8_t b) {
	if (m_bytes >= m_max_bytes) {
		// overflow
	} else {
		m_dst[m_bytes] = b;
		++m_bytes;
	}
}

void SlipEncoder::begin() {
	m_bytes = 0;
	write_byte(SLIP_SOF);
}
void SlipEncoder::write(const void* data,uint16_t size) {
	const uint8_t* d = static_cast<const uint8_t*>(data);
	for (uint16_t i=0;i<size;++i) { 
		uint8_t b = d[i];
		if (b == SLIP_SOF) {
			write_byte(SLIP_DB);
			write_byte(SLIP_DC);
		} else if (b == SLIP_DB) {
			write_byte(SLIP_DB);
			write_byte(SLIP_DD);
		} else {
			write_byte(b);
		}
	}
}

uint16_t SlipEncoder::end() {
	write_byte(SLIP_SOF);
	return m_bytes;
}

SlipDecoder::SlipDecoder(uint8_t* dst,uint16_t max) :
 m_state(S_UNKNOWN),
 m_dst(dst),
 m_bytes(0),
 m_max_bytes(max) {

}

void SlipDecoder::reset() {
	m_bytes = 0;
	m_state = S_UNKNOWN;
}

void SlipDecoder::put_byte(uint8_t b) {
	if (m_bytes >= m_max_bytes) {
		// overflow
		reset();
	} else {
		m_dst[m_bytes] = b;
		++m_bytes;
	}
}

void SlipDecoder::on_end() {
	uint16_t len = m_bytes;
	reset();
	on_packet(m_dst,len);
}
void SlipDecoder::on_data(uint8_t b) {
	switch (m_state) {
	case S_UNKNOWN:
		if (b == SLIP_SOF) {
			reset();
			m_state = S_ACTIVE;
		}
		break;
	case S_X_DB:
		if (b==SLIP_DC) {
			put_byte(SLIP_SOF);
			m_state = S_ACTIVE;
		} else if (b==SLIP_DD) {
			put_byte(SLIP_DB);
			m_state = S_ACTIVE;
		} else {
			DBG("slip invalid DB\n");
			// error
			reset();
		}
		break;
	case S_ACTIVE:
		if (b == SLIP_DB) {
			m_state = S_X_DB;
			break;
		} else if (b == SLIP_SOF) {
			if (m_bytes == 0) {
				//DBG("slip invalid SOF\n");
				reset();
				m_state = S_ACTIVE;
			} else {
				on_end();
			}
		} else {
			put_byte(b);
		}
		break;
	}
}
void SlipDecoder::on_data(const void* data,uint16_t size) {
	const uint8_t* d = static_cast<const uint8_t*>(data);
	for (uint16_t i=0;i<size;++i) {
		uint8_t b = d[i];
		on_data(b);
	}
}