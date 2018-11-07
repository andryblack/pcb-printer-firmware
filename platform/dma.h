#ifndef _DMA_H_INCLUDED_
#define _DMA_H_INCLUDED_

#include <stm32f1xx.h>

class DMAChannel {
private:
	DMA_Channel_TypeDef* m_ch;
public:
	enum DataSize {
		DS8 = 0,
		DS16 = 1,
		DS32 = 2
	};
	explicit DMAChannel(DMA_Channel_TypeDef* ch);
	void configure_p2m(uint8_t priority,
					DataSize m_size,
					DataSize p_size,
					bool m_inc,
					bool p_inc);
	void configure_m2p(uint8_t priority,
					DataSize m_size,
					DataSize p_size,
					bool m_inc,
					bool p_inc);
	void set_circular(bool en);
	void set_p(volatile void* ptr);
	void start_read(uint16_t size,void* ptr,bool _auto);
	void start_write(uint16_t size,const void* ptr,bool _auto);
	void wait();
	void on_irq();
};

#endif /*_DMA_H_INCLUDED_*/