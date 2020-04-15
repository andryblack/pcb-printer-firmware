#ifndef _USB_DEFS_H_INCLUDED_
#define _USB_DEFS_H_INCLUDED_

#include <stdint.h>

#define LOBYTE(x)  ((uint8_t)((x) & 0x00FF))
#define HIBYTE(x)  ((uint8_t)(((x) & 0xFF00) >>8))
#define USB_BCB(a,b) ((a)<<8|(b))

#define  READWORD(addr)        ((uint16_t)(*(addr)) | ( ((uint16_t)(*((addr)+1))) << 8 ))

#ifndef USB_MAX_EP0_SIZE
#define USB_MAX_EP0_SIZE                                  64
#endif

#define USB_INTERFACE_CLASS_DEVICE_SPECIFIC 	           0xFE

#define  USB_DESC_TYPE_DEVICE                              1
#define  USB_DESC_TYPE_CONFIGURATION                       2
#define  USB_DESC_TYPE_STRING                              3
#define  USB_DESC_TYPE_INTERFACE                           4
#define  USB_DESC_TYPE_ENDPOINT                            5
#define  USB_DESC_TYPE_DEVICE_QUALIFIER                    6
#define  USB_DESC_TYPE_OTHER_SPEED_CONFIGURATION           7
#define  USB_DESC_TYPE_BOS                                 0x0F

struct usb_descr_header_t {
	uint8_t 	bLength;
	uint8_t 	bDescriptorType;
}__attribute__((packed));
static const uint8_t usb_descr_header_size = sizeof(usb_descr_header_t);
static_assert(usb_descr_header_size==2);

struct usb_device_descr_t {
	usb_descr_header_t header;
	uint16_t	bcdUSB;
	uint8_t		bDeviceClass;
	uint8_t		bDeviceSubClass;
	uint8_t		bDeviceProtocol;
	uint8_t		bMaxPacketSize0;
	uint16_t	idVendor;
	uint16_t	idProduct;
	uint16_t	bcdDevice;
	uint8_t		iManufacturer;
	uint8_t		iProduct;
	uint8_t		iSerialNumber;
	uint8_t		bNumConfigurations;
} __attribute__((packed));
static const uint8_t usb_device_descr_size = sizeof(usb_device_descr_t);
static_assert(usb_device_descr_size==0x12);

// Universal Serial Bus Specification Revision 2.0 p265
struct usb_config_descr_t {
	usb_descr_header_t	header;
	uint16_t	wTotalLength;
	uint8_t		bNumInterfaces;
	uint8_t		bConfigurationValue;
	uint8_t		iConfiguration;
	uint8_t		bmAttributes;
	uint8_t		bMaxPower;
} __attribute__((packed));
static const uint8_t usb_config_descr_size = sizeof(usb_config_descr_t);
static_assert(usb_config_descr_size==9);

// Universal Serial Bus Specification Revision 2.0 p268
struct usb_interface_descr_t {
	usb_descr_header_t	header;
	uint8_t		bInterfaceNumber;
	uint8_t		bAlternateSetting;
	uint8_t		bNumEndpoints;
	uint8_t		bInterfaceClass;
	uint8_t		bInterfaceSubClass;
	uint8_t		bInterfaceProtocol;
	uint8_t		iInterface;
} __attribute__((packed));
static const uint8_t usb_interface_descr_size = sizeof(usb_interface_descr_t);
static_assert(usb_interface_descr_size==9);

// Universal Serial Bus Specification Revision 2.0 p269
struct usb_endpoint_descr_t {
	usb_descr_header_t	header;
	uint8_t		bEndpointAddress;
	uint8_t		bmAttributes;
	uint16_t	wMaxPacketSize;
	uint8_t		bInterval;
} __attribute__((packed));
static const uint8_t usb_endpoint_descr_size = sizeof(usb_endpoint_descr_t);
static_assert(usb_endpoint_descr_size==7);


template <uint32_t cnt>
struct usb_lang_id_descr_t {
	usb_descr_header_t header;
	uint16_t wLANGID[cnt];
}__attribute__((packed));
template <uint32_t cnt>
struct usb_string_descr_t {
	usb_descr_header_t header;
	char16_t string[cnt];
}__attribute__((packed));
#define USB_DECL_STRING(name,str) static const usb_string_descr_t<sizeof(str)/2> name \
	__attribute__ ((aligned (4))) = { \
	header : { usb_device_descr_size + sizeof(str) - 2, USB_DESC_TYPE_STRING }, \
	string : str \
}

#define  USB_REQ_RECIPIENT_DEVICE                       0x00
#define  USB_REQ_RECIPIENT_INTERFACE                    0x01
#define  USB_REQ_RECIPIENT_ENDPOINT                     0x02
#define  USB_REQ_RECIPIENT_MASK                         0x03

#define  USB_REQ_TYPE_STANDARD                          0x00
#define  USB_REQ_TYPE_CLASS                             0x20
#define  USB_REQ_TYPE_VENDOR                            0x40
#define  USB_REQ_TYPE_MASK                              0x60

#define  USB_REQ_GET_STATUS                             0x00
#define  USB_REQ_CLEAR_FEATURE                          0x01
#define  USB_REQ_SET_FEATURE                            0x03
#define  USB_REQ_SET_ADDRESS                            0x05
#define  USB_REQ_GET_DESCRIPTOR                         0x06
#define  USB_REQ_SET_DESCRIPTOR                         0x07
#define  USB_REQ_GET_CONFIGURATION                      0x08
#define  USB_REQ_SET_CONFIGURATION                      0x09
#define  USB_REQ_GET_INTERFACE                          0x0A
#define  USB_REQ_SET_INTERFACE                          0x0B
#define  USB_REQ_SYNCH_FRAME                            0x0C


#define  USBD_IDX_LANGID_STR                            0x00 
#define  USBD_IDX_MFC_STR                               0x01 
#define  USBD_IDX_PRODUCT_STR                           0x02
#define  USBD_IDX_SERIAL_STR                            0x03 
#define  USBD_IDX_CONFIG_STR                            0x04 
#define  USBD_IDX_INTERFACE_STR                         0x05 

#define  USB_LEN_DEV_QUALIFIER_DESC                     0x0A
#define  USB_LEN_DEV_DESC                               0x12
#define  USB_LEN_CFG_DESC                               0x09
#define  USB_LEN_IF_DESC                                0x09
#define  USB_LEN_EP_DESC                                0x07
#define  USB_LEN_OTG_DESC                               0x03
#define  USB_LEN_LANGID_STR_DESC                        0x04
#define  USB_LEN_OTHER_SPEED_DESC_SIZ                   0x09

#define USBD_EP_TYPE_CTRL                                 0
#define USBD_EP_TYPE_ISOC                                 1
#define USBD_EP_TYPE_BULK                                 2
#define USBD_EP_TYPE_INTR                                 3

#define USB_FEATURE_EP_HALT                                0
#define USB_FEATURE_REMOTE_WAKEUP                          1
#define USB_FEATURE_TEST_MODE                              2




#endif /*_USB_DEFS_H_INCLUDED_*/