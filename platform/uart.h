#ifndef _UART_H_INCLUDED_
#define _UART_H_INCLUDED_


#include <stm32f1xx.h>

class UART {
private:
	USART_TypeDef* m_uart;
	uint8_t m_rx_buffer[256];
	volatile uint8_t m_rx_wpos;
	uint8_t m_rx_rpos;
	uint32_t m_tx_buffer[256];
	uint8_t m_tx_wpos;
	volatile uint8_t m_tx_rpos;
	uint32_t m_errors;
public:
	explicit UART(USART_TypeDef* tim);
	void configure_8n1(uint32_t freq);
	void on_irq();
	void putc(uint8_t c);
	uint32_t transmit(const uint8_t* data,uint32_t size);
	uint8_t getc();
	uint8_t read();
	void start_rx_it();
	void stop();
	bool tx_empty() const;
	bool rx_empty() const;
};

#endif /*_UART_H_INCLUDED_*/