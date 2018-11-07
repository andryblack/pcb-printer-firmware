#include "uart.h"
#include "hw_config.h"

#define UART_DIV_SAMPLING16(_PCLK_, _BAUD_)         (((_PCLK_)*25)/(4*(_BAUD_)))
#define UART_DIVMANT_SAMPLING16(_PCLK_, _BAUD_)     (UART_DIV_SAMPLING16((_PCLK_), (_BAUD_))/100)
#define UART_DIVFRAQ_SAMPLING16(_PCLK_, _BAUD_)     (((UART_DIV_SAMPLING16((_PCLK_), (_BAUD_)) - (UART_DIVMANT_SAMPLING16((_PCLK_), (_BAUD_)) * 100)) * 16 + 50) / 100)
/* UART BRR = mantissa + overflow + fraction
            = (UART DIVMANT << 4) + (UART DIVFRAQ & 0xF0) + (UART DIVFRAQ & 0x0F) */

#define UART_BRR_SAMPLING16(_PCLK_, _BAUD_)            (((UART_DIVMANT_SAMPLING16((_PCLK_), (_BAUD_)) << 4) + \
                                                        (UART_DIVFRAQ_SAMPLING16((_PCLK_), (_BAUD_)) & 0xF0)) + \
                                                        (UART_DIVFRAQ_SAMPLING16((_PCLK_), (_BAUD_)) & 0x0F))

UART::UART(USART_TypeDef* uart) : m_uart(uart) {
	m_rx_wpos = 0;
	m_rx_rpos = 0;
	m_tx_wpos = 0;
	m_tx_rpos = 0;
	m_errors = 0;
}

void UART::configure_8n1(uint32_t freq) {
	m_uart->CR1 &=  ~USART_CR1_UE;

	MODIFY_REG(m_uart->CR2, USART_CR2_STOP, 0);
	m_uart->CR1 = 
		/* USART_CR1_M | */
		/* USART_CR1_WAKE | */
		/* USART_CR1_PCE | */
		/* USART_CR1_PS | */
		/* USART_CR1_PEIE | */
		/* USART_CR1_TXEIE | */
		/* USART_CR1_TCIE | */
		/* USART_CR1_RXNEIE | */
		/* USART_CR1_IDLEIE | */
		USART_CR1_TE |
		USART_CR1_RE |
		/* USART_CR1_RWU | */
		/* USART_CR1_SBK | */
		0;

	m_uart->CR2 = 
		/* USART_CR2_LINEN | */
		/* USART_CR2_STOP | */
		/* USART_CR2_CLKEN | */
		/* USART_CR2_CPOL | */
		/* USART_CR2_CPHA | */
		/* USART_CR2_LBCL | */
		/* USART_CR2_LBDIE | */
		/* USART_CR2_LBDL | */
		/* USART_CR2_ADD | */
		0;
	m_uart->CR3 =
		/* USART_CR3_CTSIE | */
		/* USART_CR3_CTSE | */
		/* USART_CR3_RTSE | */
		/* USART_CR3_DMAT | */
		/* USART_CR3_DMAR | */
		/* USART_CR3_SCEN | */
		/* USART_CR3_NACK | */
		/* USART_CR3_HDSEL | */
		/* USART_CR3_IRLP | */
		/* USART_CR3_IREN | */
		USART_CR3_EIE | 
		0;

	
	if(m_uart == USART1)
	{
		m_uart->BRR = UART_BRR_SAMPLING16(FREQ_PCLK2, freq);
	}
	else
	{
		m_uart->BRR = UART_BRR_SAMPLING16(FREQ_PCLK1, freq);
	}

  	m_uart->CR1 |=  USART_CR1_UE;
}

void UART::start_rx_it() {
	m_uart->CR1 |= USART_CR1_RXNEIE;
}

void UART::stop() {
	m_uart->CR1 &= ~(USART_CR1_RXNEIE | USART_CR1_TXEIE | USART_CR1_UE);
}

void UART::putc(uint8_t c) {
	uint8_t n = m_tx_wpos + 1;
	while ((n == m_tx_rpos) && (m_uart->CR1&USART_CR1_TXEIE)) {
		__NOP();
	}
	m_tx_buffer[m_tx_wpos] = c;
	m_tx_wpos = n;
	m_uart->CR1 |= USART_CR1_TXEIE;
}

uint32_t UART::transmit(const uint8_t* data,uint32_t size) {
	for (uint32_t i=0;i<size;++i) {
		uint8_t n = m_tx_wpos + 1;
		if ((n == m_tx_rpos)) {
			return i;
		}
		m_tx_buffer[m_tx_wpos] = data[i];
		m_tx_wpos = n;
		m_uart->CR1 |= USART_CR1_TXEIE;
	}
	return size;
}

uint8_t UART::getc() {
	if (m_rx_wpos == m_rx_rpos) {
		return 0;
	}
	uint8_t d = m_rx_buffer[m_rx_rpos++];
	return d;
}

uint8_t UART::read() {
	return m_rx_buffer[m_rx_rpos++];
}

bool UART::rx_empty() const {
	return m_rx_wpos == m_rx_rpos;
}
bool UART::tx_empty() const {
	return m_tx_wpos == m_tx_rpos;
}

void UART::on_irq() {
	__IO uint32_t sr = m_uart->SR;
	if (sr & (USART_SR_ORE | USART_SR_FE | USART_SR_NE)) {
		if (m_errors == 0) {
			m_errors = sr & (USART_SR_ORE | USART_SR_FE | USART_SR_NE);
		}
	}
	if (sr & USART_SR_RXNE) {
		__IO uint32_t tmpreg = m_uart->DR;
		m_rx_buffer[m_rx_wpos++] = tmpreg;
	} 

	if (sr & USART_SR_TXE) {
		if (m_tx_wpos != m_tx_rpos) {
			m_uart->DR = m_tx_buffer[m_tx_rpos++];
		} else if (m_uart->CR1 & USART_CR1_TXEIE) {
			m_uart->CR1 &= ~USART_CR1_TXEIE;
		}
	}
}

