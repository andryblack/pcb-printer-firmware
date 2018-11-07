#include "dma.h"


DMAChannel::DMAChannel(DMA_Channel_TypeDef* ch ) : m_ch(ch) {

}

void DMAChannel::configure_p2m(uint8_t priority,
					DataSize m_size,
					DataSize p_size,
					bool m_inc,
					bool p_inc) {
	CLEAR_BIT(m_ch->CCR,DMA_CCR_EN);
	m_ch->CCR = (priority << 12) | (m_size << 10) | ( p_size << 8) | 
		(m_inc ? DMA_CCR_MINC : 0) | (p_inc ? DMA_CCR_PINC : 0);
}

void DMAChannel::configure_m2p(uint8_t priority,
					DataSize m_size,
					DataSize p_size,
					bool m_inc,
					bool p_inc) {
	CLEAR_BIT(m_ch->CCR,DMA_CCR_EN);
	m_ch->CCR = (priority << 12) | (m_size << 10) | ( p_size << 8) | 
		(m_inc ? DMA_CCR_MINC : 0) | (p_inc ? DMA_CCR_PINC : 0);
}

void DMAChannel::set_p(volatile void* ptr) {
	m_ch->CPAR = (uint32_t)ptr;
}
void DMAChannel::start_read(uint16_t size,void* ptr,bool auto_) {
	CLEAR_BIT(m_ch->CCR,DMA_CCR_DIR|DMA_CCR_EN|DMA_CCR_MEM2MEM);
	m_ch->CMAR = (uint32_t)ptr;
	m_ch->CNDTR = size;
	if (auto_) {
		SET_BIT(m_ch->CCR,DMA_CCR_MEM2MEM);
	}
	SET_BIT(m_ch->CCR,DMA_CCR_EN);
}
void DMAChannel::start_write(uint16_t size,const void* ptr,bool auto_) {
	CLEAR_BIT(m_ch->CCR,DMA_CCR_EN | DMA_CCR_MEM2MEM);
	m_ch->CMAR = (uint32_t)ptr;
	m_ch->CNDTR = size;
	SET_BIT(m_ch->CCR,DMA_CCR_DIR | (auto_ ? DMA_CCR_MEM2MEM : 0));
	SET_BIT(m_ch->CCR,DMA_CCR_EN);
}

void DMAChannel::set_circular(bool en) {
	MODIFY_REG(m_ch->CCR,DMA_CCR_CIRC,en?DMA_CCR_CIRC:0);
}

void DMAChannel::wait() {
	while (m_ch->CNDTR) {
		__NOP();
	}
}

void DMAChannel::on_irq() {

}