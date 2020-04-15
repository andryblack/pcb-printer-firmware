#include "usb_cdc.h"
#include "usb_cdc_defs.h"
#include <cstring>

bool USB_CDC::class_init(uint8_t config_idx) {
	m_cmd_code = 0xff;
	get_endpoint<1>().activate(USB_EP_TYPE_INTERRUPT,true,
		allocate_pma(8),8);
	get_endpoint<2>().activate(USB_EP_TYPE_BULK,true,
		allocate_pma(64),64);
	get_endpoint<2>().activate(USB_EP_TYPE_BULK,false,
		allocate_pma(64),64);
	
	get_endpoint<2>().start_rx();
	return true;
}

bool USB_CDC::class_deinit(uint8_t config_idx) {
	get_endpoint<1>().deactivate(true);
	get_endpoint<2>().deactivate(true);
	return true;
}
static const uint8_t alt_interface = 0;
bool USB_CDC::class_setup(Endpoint& ep,const usb_setup_req_t& req) {
	switch (req.bmRequestType & USB_REQ_TYPE_MASK) {
		case USB_REQ_TYPE_CLASS : 
			if (req.wLength) {
				if (req.bRequest & 0x80) {
					control(req.bRequest,req.wValue,m_data,req.wLength); 
					ep.transmit(m_data,req.wLength);
					return true;
				} else {
					m_cmd_code = req.bRequest;
					m_cmd_length = req.wLength;
					//DBG("receive %d %d\n",int(m_cmd_code),int(m_cmd_length));
					ep.receive(m_data,req.wLength);
				}
			} else {
				control(req.bRequest,req.wValue,0,0);
			}
			break;
		case USB_REQ_TYPE_STANDARD:
			switch (req.bRequest) {
				case USB_REQ_GET_DESCRIPTOR: 
					//DBG("USB_REQ_GET_DESCRIPTOR\n");
					break;
				case USB_REQ_GET_INTERFACE:
					ep.transmit(&alt_interface,1);
					return true;
					break;
				case USB_REQ_SET_INTERFACE:
					break;
				default:
					//DBG("USB_REQ_TYPE_STANDARD\n");
					return false;
					break;
			}
			break;
		default:
			//DBG("class_setup\n");
			return false;
			break;
	}
	send_status(ep);
	return true;
}

void USB_CDC::on_ep_rx(Endpoint& ep) {
	if (ep.get_num() == 0 && is_configured()) {
		if (m_cmd_code != 0xff) {
			control(m_cmd_code,0,m_data,m_cmd_length);
			m_cmd_code = 0xff;
		}
	} else if (ep.get_num() == 2 && is_configured()) {
		uint32_t size = ep.get_rx_size();
		ep.read(m_rx_buffer,size);
		on_data_received(m_rx_buffer,size);
		ep.start_rx();
	}
	USB_Std::on_ep_rx(ep);
}

void USB_CDC::on_data_received(const uint8_t* data,uint32_t size) {
	//DBG("received %d\n",int(size));
}

uint32_t USB_CDC::data_transmit(const uint8_t* data,uint32_t size) {
	if (size > sizeof(m_tx_buffer)) {
		size = sizeof(m_tx_buffer);
	}
	memcpy(m_tx_buffer,data,size);
	get_endpoint<2>().transmit(m_tx_buffer,size);
	return size;
}
void USB_CDC::on_data_transmit() {
	
}

void USB_CDC::on_ep_tx(Endpoint& ep) {
	USB_Std::on_ep_tx(ep);
	if (ep.get_num() == 2) {
		on_data_transmit();
	}
}

void USB_CDC::control(uint8_t code,uint16_t value,void* data,uint16_t size) {
  switch (code)
  {
  case CDC_SEND_ENCAPSULATED_COMMAND:
 	//DBG("CDC_SEND_ENCAPSULATED_COMMAND\n");
    break;

  case CDC_GET_ENCAPSULATED_RESPONSE:
 	//DBG("CDC_GET_ENCAPSULATED_RESPONSE\n");
    break;

  case CDC_SET_COMM_FEATURE:
 	//DBG("CDC_SET_COMM_FEATURE\n");
    break;

  case CDC_GET_COMM_FEATURE:
  	//DBG("CDC_GET_COMM_FEATURE\n");
    break;

  case CDC_CLEAR_COMM_FEATURE:
  	//DBG("CDC_CLEAR_COMM_FEATURE\n");
    break;

  case CDC_SET_LINE_CODING:   
  	memcpy(&m_line_coding,data,sizeof(m_line_coding));
    break;

  case CDC_GET_LINE_CODING:     
  	memcpy(data,&m_line_coding,sizeof(m_line_coding));
    break;

  case CDC_SET_CONTROL_LINE_STATE:
  	m_line_state = value;
    break;

  case CDC_SEND_BREAK:
 	//DBG("CDC_SEND_BREAK\n");
    break;    
    
  default:
  	//DBG("control %d\n",int(code));
    break;
  }

}