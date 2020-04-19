#ifndef _PROTOCOL_H_INCLUDED_
#define _PROTOCOL_H_INCLUDED_

#include "slip.h"

struct header_t {
	uint8_t seq;
	uint8_t cmd;
	uint16_t len;
} __attribute__((packed));

enum Commands : uint8_t {
	CMD_PING,
	CMD_MOVE_X,
	CMD_STOP,
	CMD_READ_SPEED,
	CMD_ZERO_X,
	CMD_ZERO_Y,
	CMD_SETUP_MOTOR,
	CMD_PRINT,
	CMD_MOVE_Y,
	CMD_SETUP_LASER,
	CMD_SET_STEPPER_PARAM,
	CMD_FLASH,
};
enum Codes : uint16_t {
	CODE_OK,
	CODE_INVALID_DATA,
	CODE_OVERFLOW,
};
struct move_x_t {
	int32_t pos;
	uint16_t speed;
	uint16_t flags;
} __attribute__((packed));
static const uint16_t FLAG_WRITE_SPEED = (1<<0);
static const uint16_t FLAG_WAIT_MOVE = (1<<1);

struct setup_motor_t {
	float P;
	float I;
	float D;
	uint32_t min_pwm;
	uint32_t max_pwm;
} __attribute__((packed));

struct print_t {
	uint16_t start;
	uint16_t len;
	int16_t move_y;
	uint16_t speed;
	uint8_t data[0];
} __attribute__((packed));

struct move_y_t {
	int32_t pos;
	uint16_t flags;
} __attribute__((packed));

struct ping_resp_t {
	int16_t pos_x;
	int32_t pos_y;
} __attribute__((packed));

enum LaserMode : uint16_t {
	LASER_MODE_PRINT,
	LASER_MODE_PWM
};

struct setup_laser_t {
	LaserMode mode:16;
	uint16_t param;
} __attribute__((packed));

enum ParamID : uint8_t {
	PARAM_STEPPER_MAX_SPEED,
	PARAM_STEPPER_START_SPEED,
	PARAM_STEPPER_ACCEL,
	PARAM_STEPPER_DECCEL,
	PARAM_STEPPER_STOP_STEPS,
};

struct set_param_t {
	ParamID  param;
	uint32_t value;
} __attribute__((packed));

class Protocol {
public:
	static const uint16_t MAX_RX_PACKET = 1024*3; 
	static const uint16_t MAX_TX_PACKET = 1024;
private:
	uint8_t m_rx_packet[MAX_RX_PACKET];
	uint16_t m_rx_packet_size;
	uint8_t m_tx_packet[MAX_TX_PACKET];
	uint16_t m_tx_packet_size;
	uint16_t m_tx_packet_pos;
	class SlipDecoderImpl : public SlipDecoder {
		Protocol* m_self;
	public:
		SlipDecoderImpl(Protocol* self,uint8_t* data,uint16_t max_size);
		void on_packet(const void* data,uint16_t size);
	} m_slip_decoder;
	void on_rx_packet(uint16_t size);
	uint8_t m_rx_seq;
	SlipEncoder m_slip_encoder;
	enum State {
		S_WAIT_CMD,
		S_PROCESS_CMD,
		S_SEND_RESP,
		S_WAIT_SLOT,
		S_WAIT_MOVE
	} m_state;
	uint32_t m_speed_samples_sended;
protected:
	virtual uint16_t send_data(const void* data,uint16_t size) = 0;
public:
	Protocol();
	void on_rx_data(const void* data,uint16_t size);
	void on_rx_byte(uint8_t d);
	void process();
	void process_cmd(uint8_t cmd,const void* data,uint16_t data_size);
	void send_resp(uint8_t cmd,const void* data,uint16_t data_size,bool immediate=true);
	void send_resp(uint16_t cmd,uint16_t code,bool immediate=true);
};

#endif /*_PROTOCOL_H_INCLUDED_*/