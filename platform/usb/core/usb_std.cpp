#include "usb_std.h"


static uint8_t data_buffer[64];

#ifdef USB_DEBUG
static const char hex[] = "0123456789abcdef";
static void dump_mem(const void* data,uint32_t length) {
	const uint8_t* b = static_cast<const uint8_t*>(data);
	while (length) {
		char buf[32*3+4];
		uint32_t l = length > 32 ? 32 : length;
		char* o = buf;
		for (uint32_t i=0;i<l;++i) {
			*o++ = hex[(*b&0xf0)>>4];
			*o++ = hex[(*b&0x0f)];
			*o++ = ' ';
			++b;
		}
		*o++ ='\n';
		*o++ = 0;
		printf(buf);
		length -= l;
	}
} 
#endif


const uint8_t* USB_Std::get_config_ep_data()  {
	return &data_buffer[8];
}

void USB_Std::init() {
	m_dev_state = USBD_STATE_DEFAULT;
	USB_FS::init();
#ifdef USB_DEBUG
	uint32_t len = 0;
	const void * descr = get_config_descriptor(len);
	dump_mem(descr,len);
#endif
}

void USB_Std::send_status(Endpoint& ep) {
	ep.transmit(0,0);
}

void USB_Std::on_reset_irq() {
	//DBG("reset\n");
	m_dev_state = USBD_STATE_DEFAULT;
	USB_FS::on_reset_irq();
}
void USB_Std::on_ep_setup(Endpoint& ep) {
	if (ep.get_num()!=0) {
		USB_DBG("on_ep_setup err %d\n",int(ep.get_num()));
		ep.set_stall(); 
		return;
	}
	uint32_t size = ep.get_rx_size();
	if (size<8) {
		USB_DBG("on_ep_setup err size %d\n",int(size));
		ep.set_stall(); 
		return;
	}
	ep.read(data_buffer,size);
	const usb_setup_req_t* req = reinterpret_cast<const usb_setup_req_t*>(data_buffer);
	
	switch (req->bmRequestType & 0x1F) 
	{
		case USB_REQ_RECIPIENT_DEVICE:   
			on_device_request(ep,*req);
			break;

		case USB_REQ_RECIPIENT_INTERFACE:   
			on_interface_request(ep,*req);
			break;

		case USB_REQ_RECIPIENT_ENDPOINT: 
			on_endpoint_request(ep,*req);      
			break;

		default:          
			ep.set_stall(); 
			break;
	}  
}

void USB_Std::on_ep_rx(Endpoint& ep) {
	if (ep.get_num() == 0) {
		send_status(ep);
	}	
}

void USB_Std::on_device_request(Endpoint& ep,const usb_setup_req_t& req) {
	switch (req.bRequest) 
	{
		case USB_REQ_GET_DESCRIPTOR: 
			get_descriptor(ep,req);
			break;

		case USB_REQ_SET_ADDRESS:    
			set_address(ep,req);     
			break;

		case USB_REQ_SET_CONFIGURATION:   
			set_configuration(ep,req);
			break;

		case USB_REQ_GET_CONFIGURATION:
			get_configuration(ep,req);
			break;

		case USB_REQ_GET_STATUS:      
			get_status(ep,req);
			break;

		case USB_REQ_SET_FEATURE:   
			set_feature(ep,req);
			break;

		case USB_REQ_CLEAR_FEATURE:                                   
			clean_feature(ep,req);
			break;

		default:  
			USB_DBG("on_device_request err %d\n",int(req.bRequest));
			ep.set_stall();
			break;
	}
}

void USB_Std::get_descriptor(Endpoint& ep,const usb_setup_req_t& req) {
	const void* data = 0;
	uint32_t length = 0;
	switch (req.wValue >> 8) {
		case USB_DESC_TYPE_DEVICE:
			data = get_device_descriptor(length);
			break;
		case USB_DESC_TYPE_CONFIGURATION:  
			data = get_config_descriptor(length);
			break;
		case USB_DESC_TYPE_STRING:
			data = get_string(req.wValue&0xff,length);
			if (!data) {
				USB_DBG("USB_DESC_TYPE_STRING err\n");
				ep.set_stall();
			}
			break;
		case USB_DESC_TYPE_DEVICE_QUALIFIER: 
			USB_DBG("USB_DESC_TYPE_DEVICE_QUALIFIER\n");
			break;
		case USB_DESC_TYPE_OTHER_SPEED_CONFIGURATION:
			USB_DBG("USB_DESC_TYPE_OTHER_SPEED_CONFIGURATION\n");
			break;
		default:
			USB_DBG("get_descriptor err %d\n",int(req.wValue));
			ep.set_stall();
			break;
	}
	if((length != 0)&& (req.wLength != 0))
	{
		length = (length < req.wLength) ? length : req.wLength;
		ep.transmit(data,length);
	}
}

void USB_Std::set_address(Endpoint& ep,const usb_setup_req_t& req) {
	if ((req.wIndex == 0) && (req.wLength == 0)) 
	{
		uint8_t dev_addr = req.wValue & 0x7F;     

		if (m_dev_state == USBD_STATE_CONFIGURED) 
		{
			USB_DBG("set_address err USBD_STATE_CONFIGURED\n");
			ep.set_stall();
		} 
		else 
		{
			m_dev_cfgidx = 0;

			set_address(dev_addr);

			send_status(ep);                  

			if (dev_addr != 0) 
			{
				m_dev_state = USBD_STATE_ADDRESSED;
			} 
			else 
			{
				m_dev_state = USBD_STATE_DEFAULT; 
			}
		}
	} 
	else 
	{
		USB_DBG("set_address err\n");
		ep.set_stall();                       
	} 
}

uint8_t USB_Std::get_num_configurations() {
	uint32_t len = 0;
	const uint8_t* config = static_cast<const uint8_t*>(get_device_descriptor(len));
	return config[0x11];
}
uint8_t USB_Std::get_num_interfaces(uint8_t cfgidx) {
	uint32_t len = 0;
	const uint8_t* config = static_cast<const uint8_t*>(get_config_descriptor(len));
	return config[4];
}

void USB_Std::set_configuration(Endpoint& ep,const usb_setup_req_t& req) {
	uint8_t cfgidx = req.wValue;
	if (cfgidx > get_num_configurations()) {
		USB_DBG("set_configuration err\n");
		ep.set_stall();
		return;
	}  
	switch (m_dev_state) {
		case USBD_STATE_ADDRESSED: 
			if (cfgidx) {
				m_dev_cfgidx = cfgidx;
				m_dev_state = USBD_STATE_CONFIGURED;
				if (!class_init(cfgidx)) {
					ep.set_stall();
				} else {
					send_status(ep);
				}
			} else {
				send_status(ep);
			}
			break;
		case USBD_STATE_CONFIGURED:
			if (cfgidx == 0) {
				class_deinit(m_dev_cfgidx);
				m_dev_cfgidx = 0;
				m_dev_state = USBD_STATE_ADDRESSED;
				send_status(ep);
			} else if (cfgidx != m_dev_cfgidx) {
				class_deinit(m_dev_cfgidx);
				m_dev_cfgidx = cfgidx;
				if (!class_init(cfgidx)) {
					ep.set_stall();
				} else {
					send_status(ep);
				}
			} else {
				send_status(ep);
			}
			break;
		default:
			USB_DBG("set_configuration state err\n");
			ep.set_stall();
			break;
	}
	
}

void USB_Std::get_configuration(Endpoint& ep,const usb_setup_req_t& req) {
	USB_DBG("get_configuration\n");
}

void USB_Std::get_status(Endpoint& ep,const usb_setup_req_t& req) {
	
	m_dev_status = 0;
	switch (m_dev_state) {
		case USBD_STATE_ADDRESSED:
		case USBD_STATE_CONFIGURED:
			ep.transmit(&m_dev_status,2);
			break;
		default:
			USB_DBG("get_status err\n");
			ep.set_stall();
			break;
	}
}

void USB_Std::set_feature(Endpoint& ep,const usb_setup_req_t& req) {
	// todo
	USB_DBG("set_feature\n");
	send_status(ep);
}

void USB_Std::clean_feature(Endpoint& ep,const usb_setup_req_t& req) {
	USB_DBG("clean_feature\n");
}

void USB_Std::on_interface_request(Endpoint& ep,const usb_setup_req_t& req) {
	switch(m_dev_state) {
		case USBD_STATE_CONFIGURED:
			if (LOBYTE(req.wIndex) < get_num_interfaces(m_dev_cfgidx)) {
				if (class_setup(ep,req)) {
				} else {
					USB_DBG("on_interface_request class_setup\n");
					ep.set_stall();
				}
			} else {
				USB_DBG("on_interface_request invalid interface\n");
				ep.set_stall();
			}
			break;
		default:
			USB_DBG("on_interface_request invalid state\n");
			ep.set_stall();
			break;
	}
}

void USB_Std::on_endpoint_request(Endpoint& ep,const usb_setup_req_t& req) {
	uint8_t   ep_addr = LOBYTE(req.wIndex);   
	if ((req.bmRequestType & USB_REQ_TYPE_MASK) == USB_REQ_TYPE_CLASS) {
		class_setup(ep,req);
		return;
	} 
	switch (req.bRequest) {
		case USB_REQ_SET_FEATURE:
			switch (m_dev_state) {
				case USBD_STATE_ADDRESSED:
					if ((ep_addr != 0x00) && (ep_addr != 0x80)) {
						ep.set_stall();
						return;
					}
					break;
				case USBD_STATE_CONFIGURED:  
					if (req.wValue == USB_FEATURE_EP_HALT) {
						if ((ep_addr != 0x00) && (ep_addr != 0x80)) {
							ep.set_stall();
							return;
						}
					}
					class_setup(ep,req);
					break;
				default:
					ep.set_stall();
					return;
			}
			break;
		case USB_REQ_CLEAR_FEATURE:
			switch (m_dev_state) {
				case USBD_STATE_ADDRESSED:
					if ((ep_addr & 0x7F) != 0x00) {
						ep.set_stall();
						return;
					}
					break;
				case USBD_STATE_CONFIGURED:  
					if (req.wValue == USB_FEATURE_EP_HALT) {
						if ((ep_addr & 0x7F) != 0x00) {
							ep.reset_stall();
							class_setup(ep,req);
							return;
						}
					}
					break;
				default:
					ep.set_stall();
					return;
			}
			break;
		case USB_REQ_GET_STATUS:   
			switch (m_dev_state) {
				case USBD_STATE_ADDRESSED:
					if ((ep_addr & 0x7F) != 0x00) {
						ep.set_stall();
						return;
					}
					break;
				case USBD_STATE_CONFIGURED:  
					if (req.wValue == USB_FEATURE_EP_HALT) {
						if ((ep_addr & 0x7F) != 0x00) {
							ep.reset_stall();
							class_setup(ep,req);
							return;
						}
					}
					break;
				default:
					ep.set_stall();
					return;
			}
			break;
		default:
			break;
	}
}

void USB_Std::on_ep_tx(Endpoint& ep) {
	if (ep.get_num()==0) {
		ep.start_rx();
	}
}