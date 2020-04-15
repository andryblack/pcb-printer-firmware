#ifndef _USB_STD_H_INCLUDED_
#define _USB_STD_H_INCLUDED_

#include "usb.h"
#include "usb_defs.h"


struct usb_setup_req_t
{   
    uint8_t   bmRequestType;                      
    uint8_t   bRequest;                           
    uint16_t  wValue;                             
    uint16_t  wIndex;                             
    uint16_t  wLength;                          
} __attribute__((packed));


class USB_Std : public USB_FS {
public:
	void init();
private:
	enum dev_state_e {
		USBD_STATE_DEFAULT,
		USBD_STATE_ADDRESSED,
		USBD_STATE_CONFIGURED,
		USBD_STATE_SUSPENDED,
	};
	dev_state_e 	m_dev_state;
	uint8_t			m_dev_cfgidx;
	uint16_t			m_dev_status;
protected:
	bool is_configured() const { return m_dev_state == USBD_STATE_CONFIGURED; }

	virtual void on_reset_irq();
	virtual void on_ep_setup(Endpoint& ep);
	virtual void on_ep_tx(Endpoint& ep);
	virtual void on_ep_rx(Endpoint& ep);

	using USB_FS::set_address;

	void send_status(Endpoint& ep);

	void on_device_request(Endpoint& ep,const usb_setup_req_t& req);
	void on_interface_request(Endpoint& ep,const usb_setup_req_t& req);
	void on_endpoint_request(Endpoint& ep,const usb_setup_req_t& req);

	void get_descriptor(Endpoint& ep,const usb_setup_req_t& req);
	void set_address(Endpoint& ep,const usb_setup_req_t& req);
	void set_configuration(Endpoint& ep,const usb_setup_req_t& req);
	void get_configuration(Endpoint& ep,const usb_setup_req_t& req);
	void get_status(Endpoint& ep,const usb_setup_req_t& req);
	void set_feature(Endpoint& ep,const usb_setup_req_t& req);
	void clean_feature(Endpoint& ep,const usb_setup_req_t& req);

	static uint8_t get_num_configurations();
	static uint8_t get_num_interfaces(uint8_t cfgidx);

	static const void* get_device_descriptor(uint32_t& length);
	static const void* get_config_descriptor(uint32_t& length);
	static const void* get_string(uint8_t idx,uint32_t& length);

	virtual bool class_init(uint8_t config_idx) = 0;
	virtual bool class_deinit(uint8_t config_idx) = 0;
	virtual bool class_setup(Endpoint& ep,const usb_setup_req_t& req) = 0;

	static const uint8_t* get_config_ep_data();
};

#endif /*_USB_STD_H_INCLUDED_*/