#ifndef _USB_CDC_H_INCLUDED_
#define _USB_CDC_H_INCLUDED_

#include "usb/core/usb_std.h"
#include "usb_cdc_defs.h"

class USB_CDC : public USB_Std {
protected:
	
	virtual bool class_init(uint8_t config_idx);
	virtual bool class_deinit(uint8_t config_idx);

	virtual bool class_setup(Endpoint& ep,const usb_setup_req_t& req);

	virtual void on_ep_rx(Endpoint& ep);
	virtual void on_ep_tx(Endpoint& ep);
	uint32_t data_transmit(const uint8_t* data,uint32_t size);
	virtual void on_data_transmit();
protected:
	virtual void on_data_received(const uint8_t* data,uint32_t size);
	virtual void control(uint8_t code,uint16_t value,void* data,uint16_t size);
	virtual uint8_t	get_num_endpoints() const { return 3; }
	usb_cdc_req_line_coding_t	m_line_coding;
	uint16_t	m_line_state;
private:
	uint8_t 	m_cmd_code;   
	uint16_t	m_cmd_length;
	uint32_t 	m_data[64/4];
	uint8_t		m_tx_buffer[64];
	uint8_t		m_rx_buffer[64];

		
};

#endif /*_USB_CDC_H_INCLUDED_*/