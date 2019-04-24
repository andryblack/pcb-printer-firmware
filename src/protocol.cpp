#include "protocol.h"
#include "crc8.h"
#include "controller.h"
#include "encoder.h"
#include "mem_overlay.h"
#include "motor.h"
#include "config.h"
#include "queue.h"
#include "stepper.h"
#include "laser.h"

#include <cstring>

static void start_bootloader() {
	static const uint32_t sys_mem_base = 0x1FFFF000;
	// disable all per
	RCC->APB2ENR = 0;
	RCC->AHBENR = 0;
	
#define SET_SYSCLOCK(SRC) MODIFY_REG(RCC->CFGR,RCC_CFGR_SW,SRC)

	__disable_irq();
	// hsi
	SET_BIT(RCC->CR, RCC_CR_HSION);
  	while((RCC->CR & RCC_CR_HSIRDY)==0) {__NOP();}
  	SET_SYSCLOCK(RCC_CFGR_SW_HSI);

  	// disable pll
  	CLEAR_BIT(RCC->CR,	RCC_CR_PLLON);
  	// disable hse
  	CLEAR_BIT(RCC->CR, RCC_CR_HSEON);

  	SysTick->CTRL = 0;
    SysTick->LOAD = 0;
    SysTick->VAL = 0;

    // set stack
  	__set_MSP(*(uint32_t *)sys_mem_base);
  	// jump
  	((void (*)()) (*((uint32_t *)(sys_mem_base + 4))))();
}

Protocol::SlipDecoderImpl::SlipDecoderImpl(Protocol* self,uint8_t* data,uint16_t max_size) : SlipDecoder(data,max_size),
	m_self(self) {}

void Protocol::SlipDecoderImpl::on_packet(const void* data,uint16_t size) {
	m_self->on_rx_packet(size);
}


Protocol::Protocol() : m_slip_decoder(this,m_rx_packet,MAX_RX_PACKET),m_slip_encoder(m_tx_packet,MAX_TX_PACKET) {
	m_state = S_WAIT_CMD;
	m_speed_samples_sended = 0;
}

void Protocol::on_rx_packet(uint16_t size) {
	m_rx_packet_size = size;
}

void Protocol::on_rx_data(const void* data,uint16_t size) {
	m_slip_decoder.on_data(data,size);
}

void Protocol::on_rx_byte(uint8_t d) {
	m_slip_decoder.on_data(d);
}

void Protocol::process() {
	if (m_state == S_WAIT_SLOT) {
		if (Queue::has_slot()) {
			m_state = S_SEND_RESP;
		}
	} else if (m_state == S_WAIT_MOVE) {
		if (Controller::move_complete()) {
			m_state = S_SEND_RESP;
		}
	}
	if (m_state == S_SEND_RESP) {
		if (m_tx_packet_size) {
			uint16_t send_size = send_data(&m_tx_packet[m_tx_packet_pos],m_tx_packet_size);
			m_tx_packet_size -= send_size;
			m_tx_packet_pos += send_size;
			return;
		}
		m_state = S_WAIT_CMD;
	}
	if (m_state == S_WAIT_CMD) {
		if (m_rx_packet_size) {
			m_state = S_PROCESS_CMD;
		} else {
			return;
		}
	}
	if (m_state == S_PROCESS_CMD) {
		uint16_t size = m_rx_packet_size;
		m_rx_packet_size = 0;
		const header_t* header = reinterpret_cast<const header_t*>(m_rx_packet);
		m_rx_seq = header->seq;

		if (size != header->len+sizeof(header_t)+1) {
			DBG("invalid packet size\n");
			m_state = S_WAIT_CMD;
			// invalid packet size
			return;
		}
		uint8_t crc = m_rx_packet[size-1];
		if (!CRC8::check(m_rx_packet,size-1,crc)) {
			DBG("invalid crc\n");
			m_state = S_WAIT_CMD;
			// invalid crc
			return;
		}
		process_cmd(header->cmd,&m_rx_packet[sizeof(header_t)],header->len);
	}
	
}

void Protocol::process_cmd(uint8_t cmd,const void* data,uint16_t data_size) {
	switch (cmd) {
		case CMD_PING: {
			ping_resp_t resp;
			resp.pos_x = Encoder::get_pos();
			resp.pos_y = Stepper::pos;
			send_resp(cmd,&resp,sizeof(resp));
		}break;
		case CMD_MOVE_X: {
			if (data_size < sizeof(move_x_t)) {
				send_resp(cmd,CODE_INVALID_DATA);
				return;
			}
			const move_x_t* move = static_cast<const move_x_t*>(data);
			Controller::move(LASER_FREQ/move->speed,move->pos);
			if (move->flags & FLAG_WRITE_SPEED) {
				m_speed_samples_sended = 0;
				Laser::setup_print(0);
				Motor::start_write_speed();
			}
			
			if (move->flags & FLAG_WAIT_MOVE) {
				send_resp(cmd,CODE_OK,false);
				m_state = S_WAIT_MOVE;
			} else {
				send_resp(cmd,CODE_OK);
			}
		} break;
		case CMD_MOVE_Y: {
			if (data_size < sizeof(move_y_t)) {
				send_resp(cmd,CODE_INVALID_DATA);
				return;
			}
			const move_y_t* move = static_cast<const move_y_t*>(data);
			Stepper::move(move->pos);
			
			if (move->flags & FLAG_WAIT_MOVE) {
				send_resp(cmd,CODE_OK,false);
				m_state = S_WAIT_MOVE;
			} else {
				send_resp(cmd,CODE_OK);
			}
		} break;
		case CMD_STOP: {
			Controller::stop();
			send_resp(cmd,CODE_OK);
		} break;
		case CMD_READ_SPEED: {
			if (!Motor::is_move() && Motor::write_speed_pos==0) {
				Motor::write_speed_pos = 1;
				mem_overlay.speed[0].pwm = 0;
				mem_overlay.speed[0].dt = 0;	
			} 
			uint32_t samples = Motor::write_speed_pos - m_speed_samples_sended;
			static const uint32_t max_samples = (MAX_TX_PACKET-sizeof(header_t)-16) / sizeof(SpeedSample);
			if (samples <= max_samples) {
				send_resp(cmd,&mem_overlay.speed[m_speed_samples_sended],sizeof(SpeedSample)*samples);
				Motor::write_speed_pos = 0;
				m_speed_samples_sended = 0;
			} else {
				send_resp(cmd,&mem_overlay.speed[m_speed_samples_sended],sizeof(SpeedSample)*max_samples);
				m_speed_samples_sended += max_samples;
			}
		} break;
		case CMD_ZERO_X: {
			Encoder::set_zero();
			send_resp(cmd,CODE_OK);
		} break;
		case CMD_ZERO_Y: {
			Stepper::set_zero();
			send_resp(cmd,CODE_OK);
		} break;
		case CMD_SETUP_PID: {
			if (data_size < sizeof(setup_pid_t)) {
				send_resp(cmd,CODE_INVALID_DATA);
				return;
			}
			const setup_pid_t* pid = static_cast<const setup_pid_t*>(data);
			Motor::setup_PID(pid->P,pid->I,pid->D);
			send_resp(cmd,CODE_OK);
		} break;
		case CMD_PRINT: {
			if (data_size < sizeof(print_t)) {
				send_resp(cmd,CODE_INVALID_DATA);
				return;
			}
			const print_t* print = static_cast<const print_t*>(data);
			if (print->len > max_print_mem) {
				send_resp(cmd,CODE_INVALID_DATA);
				return;
			}
			if (!Queue::has_slot()) {
				send_resp(cmd,CODE_OVERFLOW);
				return;
			}
			::memcpy(Queue::write(),data,sizeof(print_t)+print->len);
			Queue::push();
			send_resp(cmd,CODE_OK);
			if (!Queue::has_slot()) {
				m_state = S_WAIT_SLOT;
			}
		} break;
		case CMD_SETUP_LASER: {
			if (data_size < sizeof(setup_laser_t)) {
				send_resp(cmd,CODE_INVALID_DATA);
				return;
			}
			const setup_laser_t* laser = static_cast<const setup_laser_t*>(data);
			if (laser->mode == LASER_MODE_PRINT) {
				Laser::setup_print(laser->param);
			} else if (laser->mode == LASER_MODE_PWM) {
				Laser::setup_pwm(laser->param);
			}
			send_resp(cmd,CODE_OK);
		} break;
		case CMD_SET_PARAM: {
			if (data_size < sizeof(set_param_t)) {
				send_resp(cmd,CODE_INVALID_DATA);
				return;
			}
			const set_param_t* param = static_cast<const set_param_t*>(data);
			Stepper::set_param(param->param,param->value);
			send_resp(cmd,CODE_OK);
		} break;
		case CMD_FLASH: {
			start_bootloader();
		} break;
		default:
			DBG("unknown cmd");
			m_state = S_WAIT_CMD;
			break;
	}
}
void Protocol::send_resp(uint16_t cmd,uint16_t code,bool immediate) {
	send_resp(cmd,&code,sizeof(code),immediate);
}
void Protocol::send_resp(uint8_t cmd,const void* data,uint16_t data_size,bool immediate) {
	m_tx_packet_pos = 0;
	header_t header;
	header.seq = m_rx_seq;
	header.cmd = cmd;
	header.len = data_size;

	m_slip_encoder.begin();
	uint8_t crc = CRC8::begin();
	m_slip_encoder.write(&header,sizeof(header));
	crc = CRC8::update(crc,&header,sizeof(header));
	if (data && data_size) {
		m_slip_encoder.write(data,data_size);
		crc = CRC8::update(crc,data,data_size);
	}
	crc = CRC8::end(crc);
	m_slip_encoder.write(&crc,1);
	m_tx_packet_size = m_slip_encoder.end();
	if (immediate) {
		uint16_t send_size = send_data(m_tx_packet,m_tx_packet_size);
		m_tx_packet_size -= send_size;
		m_tx_packet_pos += send_size;
	}
	m_state = S_SEND_RESP;
}
