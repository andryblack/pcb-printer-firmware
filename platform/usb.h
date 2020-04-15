#ifndef _USB_H_INCLUDED_
#define _USB_H_INCLUDED_

#include <stdint.h>
#include <stm32f1xx.h>

#define USB_MAX_EP0_SIZE                                  64

#ifdef USB_DEBUG
#include <cstdio>
#define USB_DBG printf
#else
#define USB_DBG(...) do {} while(false)
#endif

#define MAX_ENDPOINTS 8
#ifndef USE_ENDPOINTS
#define USE_ENDPOINTS MAX_ENDPOINTS
#endif

enum USBEPType {
	USB_EP_TYPE_BULK = USB_EP_BULK,
	USB_EP_TYPE_CONTROL = USB_EP_CONTROL,
	USB_EP_TYPE_ISOCHRONOUS = USB_EP_ISOCHRONOUS,
	USB_EP_TYPE_INTERRUPT = USB_EP_INTERRUPT
};
enum USBEPTXStat {
	USB_EP_TX_STAT_DIS = USB_EP_TX_DIS,
	USB_EP_TX_STAT_VALID = USB_EP_TX_VALID,
	USB_EP_TX_STAT_STALL = USB_EP_TX_STALL,
	USB_EP_TX_STAT_NAK = USB_EP_TX_NAK,
};
enum USBEPRXStat {
	USB_EP_RX_STAT_DIS = USB_EP_RX_DIS,
	USB_EP_RX_STAT_VALID = USB_EP_RX_VALID,
	USB_EP_RX_STAT_STALL = USB_EP_RX_STALL,
	USB_EP_RX_STAT_NAK = USB_EP_RX_NAK,
};
class USB_FS {
public:
	class Endpoint {
	private:
		USBEPType m_type;
		uint8_t m_num;
		bool m_isin;
		uint32_t m_pmaaddr_tx;
		uint32_t m_pmaaddr_rx;
		uint32_t m_maxsize;
		uint32_t	m_transmit_size;
		const void* m_transmit_buf;
		uint32_t	m_receive_size;
		void* 		m_receive_buf;
		bool	m_is_double;
	public:
		void init(uint8_t num);
		uint8_t get_num() const { return m_num; }

		void flush_tx();
		void flush_rx();

		USBEPType get_type() const { return m_type; }
		void set_tx_addr(uint32_t addr);
		void set_tx_addr1(uint32_t addr);
		void set_tx_stat(USBEPTXStat stat);
		void clear_tx_dtog();
		void clear_tx_ctr();
		void set_tx_size(uint32_t size);
		void set_tx_size1(uint32_t size);
		uint32_t get_tx_size() const;
		uint32_t get_tx_size1() const;
		void write(const void* data,uint32_t size);
		void write1(const void* data,uint32_t size);
		
		void set_rx_addr(uint32_t addr);
		void set_rx_addr1(uint32_t addr);
		void set_rx_stat(USBEPRXStat stat);
		void clear_rx_dtog();
		void clear_rx_ctr();

		void set_rx_size(uint32_t size);
		void set_rx_size1(uint32_t size);
		uint32_t get_rx_size() const;
		uint32_t get_rx_size1() const;
		
		void activate(USBEPType type,bool isin,uint32_t pmaaddr,uint32_t maxsize);
		void deactivate(bool full);
		void deactivate_rx();
		void deactivate_tx();

		void read(void* data,uint32_t size);

		void start_rx();
		void receive(void* data,uint32_t size);
		bool process_receive();
		
		void set_stall(bool isin);
		void set_stall();
		void reset_stall();

		uint16_t get_value() const;

		void transmit(const void* data,uint32_t size);
		void transmit_iso(const void* data,uint32_t size);
		bool continue_transmit();
	};
	static void init();
	static void set_address(uint8_t addr);
	static void stop();
	static void irq_handler();
	template <uint8_t e> inline
	static Endpoint& get_endpoint() { return m_endpoints[e]; }
protected:
	USB_FS();
	static void disable_interrupts();
	static void enable_interrupts();

	virtual void on_reset_irq();
	static void on_resume_irq();
	static void on_suspend_irq();
	static void on_sof_irq();
	static void on_ep_irq();

	virtual void on_ep_tx(Endpoint& ep) = 0;
	virtual void on_ep_setup(Endpoint& ep) = 0;
	virtual void on_ep_rx(Endpoint& ep) = 0;

	static uint32_t allocate_pma(uint32_t size);
	
	static void flush_ep(uint8_t ep_addr);
private:
	static uint32_t m_pma_allocated;
	static Endpoint	m_endpoints[USE_ENDPOINTS];
	static USB_FS*	m_instance;
	static uint8_t	m_delayed_usb_addr;
};

#endif 