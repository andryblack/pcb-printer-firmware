#ifndef _USB_CDC_DEFS_H_INCLUDED_
#define _USB_CDC_DEFS_H_INCLUDED_

#include "usb/core/usb_defs.h"

/// CDC Class
#define USB_DEVICE_CLASS_CDC                          0x02
#define USB_INTERFACE_CLASS_CDC 	                  0x02

#define CDC_SUBCLASS_UNDEFINED						0x00
#define CDC_SUBCLASS_DIRECT_LINE_CONTROL			0x01
#define CDC_SUBCLASS_ABSTRACT_CONTROL				0x02
#define CDC_SUBCLASS_TELEPHONE_CONTROL				0x03
#define CDC_SUBCLASS_MULTI_CHANNEL_CONTROL			0x04
#define CDC_SUBCLASS_CAPI_CONTROL					0x05
#define CDC_SUBCLASS_ETHERNET_CONTROL				0x06
#define CDC_SUBCLASS_ATM_CONTROL					0x07


#define CDC_PROTOCOL_UNDEFINED                      0x00
#define CDC_PROTOCOL_AT_COMMANDS					0x01

#define USB_INTERFACE_CLASS_DATA 	              	0x0A
#define DATA_SUBCLASS_UNDEFINED						0x00
#define DATA_PROTOCOL_UNDEFINED                     0x00

/* CDC Descriptor Types */
#define CDC_INTERFACE_DESCRIPTOR_TYPE               0x24
#define CDC_ENDPOINT_DESCRIPTOR_TYPE                0x25

#define CDC_FUNCTIONAL_HEADER						0x00
#define CDC_FUNCTIONAL_CALL							0x01
#define CDC_FUNCTIONAL_ABSTRACT_CONTROL				0x02
#define CDC_FUNCTIONAL_DIRECT_LINE					0x03
#define CDC_FUNCTIONAL_TELEPHONE_RINGER				0x04
#define CDC_FUNCTIONAL_TELEPHONE_CALL				0x05
#define CDC_FUNCTIONAL_UNION						0x06
#define CDC_FUNCTIONAL_COUNTRY						0x07
#define CDC_FUNCTIONAL_TELEPHONE_MODE				0x08
#define CDC_FUNCTIONAL_USB							0x09
#define CDC_FUNCTIONAL_NETWORK_CHANNEL				0x0A
#define CDC_FUNCTIONAL_PROTOCOL_UNIT				0x0B
#define CDC_FUNCTIONAL_EXTENSION_UNIT				0x0C
#define CDC_FUNCTIONAL_MULTI_CHANNEL				0x0D
#define CDC_FUNCTIONAL_CAPI							0x0E
#define CDC_FUNCTIONAL_ETHERNET						0x0F
#define CDC_FUNCTIONAL_ATM							0x10



struct usb_cdc_functional_header_t {
	usb_descr_header_t header;
	uint8_t		bDescriptorSubtype;
	uint16_t	bcdCDC;
} __attribute__((packed));

struct usb_cdc_functional_call_t {
	usb_descr_header_t header;
	uint8_t		bDescriptorSubtype;
	uint8_t		bmCapabilities;
	uint8_t		bDataInterface;
} __attribute__((packed));

struct usb_cdc_functional_abstract_control_t {
	usb_descr_header_t header;
	uint8_t		bDescriptorSubtype;
	uint8_t		bmCapabilities;
} __attribute__((packed));

struct usb_cdc_functional_direct_line_t {
	usb_descr_header_t header;
	uint8_t		bDescriptorSubtype;
	uint8_t		bmCapabilities;
} __attribute__((packed));

struct usb_cdc_functional_telephone_ringer_t {
	usb_descr_header_t header;
	uint8_t		bDescriptorSubtype;
	uint8_t		bRingerVolSteps;
	uint8_t		bNumRingerPatterns;
} __attribute__((packed));

struct usb_cdc_functional_telephone_mode_t {
	usb_descr_header_t header;
	uint8_t		bDescriptorSubtype;
	uint8_t		bmCapabilities;
} __attribute__((packed));

struct usb_cdc_functional_telephone_call_t {
	usb_descr_header_t header;
	uint8_t		bDescriptorSubtype;
	uint8_t		bmCapabilities;
} __attribute__((packed));

template <uint8_t slaves_count>
struct usb_cdc_functional_union_t {
	usb_descr_header_t header;
	uint8_t		bDescriptorSubtype;
	uint8_t		bMasterInterface;
	uint8_t		bSlaveInterface[slaves_count];
} __attribute__((packed));


#define CDC_SEND_ENCAPSULATED_COMMAND               0x00
#define CDC_GET_ENCAPSULATED_RESPONSE               0x01
#define CDC_SET_COMM_FEATURE                        0x02
#define CDC_GET_COMM_FEATURE                        0x03
#define CDC_CLEAR_COMM_FEATURE                      0x04
#define CDC_SET_LINE_CODING                         0x20
#define CDC_GET_LINE_CODING                         0x21
#define CDC_SET_CONTROL_LINE_STATE                  0x22
#define CDC_SEND_BREAK                              0x23

struct usb_cdc_req_line_coding_t{
	uint32_t 	dwDTERate;
	uint8_t		bCharFormat;
	uint8_t		bParityType;
	uint8_t		bDataBits;
} __attribute__((packed));

#endif /*_USB_CDC_DEFS_H_INCLUDED_*/