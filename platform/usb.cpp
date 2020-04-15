#include "usb.h"
#include <stm32f1xx.h>

#include "gpio.h"


#define BTABLE_ADDRESS                         (0x000)

USB_FS* USB_FS::m_instance = 0;

#define USB_GET_BTABLE		BTABLE_ADDRESS

struct usb_ep_pma_regs_t {
	volatile uint32_t	tx_addr;
	volatile uint32_t	tx_count;
	volatile uint32_t	rx_addr;
	volatile uint32_t	rx_count;
};
struct usb_ep_reg_t {
  volatile uint16_t EPR;                 /*!< USB Endpoint 0 register,                   Address offset: 0x00 */ 
  volatile uint16_t RESERVED;            /*!< Reserved */     
};

static usb_ep_pma_regs_t* USB_PMA_REGS = (usb_ep_pma_regs_t*)(USB_PMAADDR);
static usb_ep_reg_t* USB_REGS = (usb_ep_reg_t*)(USB_BASE);

#define SET_EP_REG(wRegValue)  (USB_REGS[m_num].EPR = (uint16_t)(wRegValue))
#define GET_EP_REG            (USB_REGS[m_num].EPR)


#define TX_DTOG() SET_EP_REG(USB_EP_CTR_RX|USB_EP_CTR_TX|USB_EP_DTOG_TX|(GET_EP_REG&USB_EPREG_MASK))
#define RX_DTOG() SET_EP_REG(USB_EP_CTR_RX|USB_EP_CTR_TX|USB_EP_DTOG_RX|(GET_EP_REG&USB_EPREG_MASK))


void USB_FS::Endpoint::init(uint8_t num) {
	m_num = num;
	m_isin = false;
	m_pmaaddr_tx = 0;
	m_pmaaddr_rx = 0;
	m_maxsize = 0;
	m_transmit_size = 0;
	m_transmit_buf = 0;
	m_is_double = false;
}

uint16_t USB_FS::Endpoint::get_value() const {
	return GET_EP_REG;
}

void USB_FS::Endpoint::flush_tx() {

}

void USB_FS::Endpoint::flush_rx() {

}

void USB_FS::Endpoint::set_tx_addr(uint32_t addr) {
	USB_PMA_REGS[m_num].tx_addr = addr & 0xffff;
}
void USB_FS::Endpoint::set_tx_addr1(uint32_t addr) {
	USB_PMA_REGS[m_num].rx_addr = addr & 0xffff;
}
void USB_FS::Endpoint::set_tx_stat(USBEPTXStat stat) {
	volatile uint16_t val = GET_EP_REG & USB_EPTX_DTOGMASK;
	val ^= stat;
   	SET_EP_REG(USB_EP_CTR_RX|USB_EP_CTR_TX|val);
}
void USB_FS::Endpoint::set_tx_size(uint32_t size) {
	USB_PMA_REGS[m_num].tx_count = size & USB_COUNT0_TX_COUNT0_TX;
}
void USB_FS::Endpoint::set_tx_size1(uint32_t size) {
	USB_PMA_REGS[m_num].rx_count = size & USB_COUNT0_TX_COUNT0_TX;
}
uint32_t USB_FS::Endpoint::get_tx_size() const {
	return USB_PMA_REGS[m_num].tx_count & USB_COUNT0_TX_COUNT0_TX;
}
uint32_t USB_FS::Endpoint::get_tx_size1() const {
	return USB_PMA_REGS[m_num].rx_count & USB_COUNT0_TX_COUNT0_TX;
}
void USB_FS::Endpoint::clear_tx_dtog() {
	if ((GET_EP_REG & USB_EP_DTOG_TX) != 0) {
	 	TX_DTOG();
	}
}
void USB_FS::Endpoint::clear_tx_ctr() {
	SET_EP_REG(((GET_EP_REG & (~USB_EP_CTR_TX)) & USB_EPREG_MASK) | USB_EP_CTR_RX);
}

void USB_FS::Endpoint::set_rx_addr(uint32_t addr) {
	USB_PMA_REGS[m_num].rx_addr = addr & 0xffff;
}
void USB_FS::Endpoint::set_rx_addr1(uint32_t addr) {
	USB_PMA_REGS[m_num].tx_addr = addr & 0xffff;
}

void USB_FS::Endpoint::set_rx_stat(USBEPRXStat stat) {
	register uint16_t val = GET_EP_REG & USB_EPRX_DTOGMASK;
	val ^= stat;
   	SET_EP_REG(USB_EP_CTR_RX|USB_EP_CTR_TX|val);
}
void USB_FS::Endpoint::clear_rx_dtog() {
	if ((GET_EP_REG & USB_EP_DTOG_RX) != 0) {
	 	RX_DTOG();
	}
}
void USB_FS::Endpoint::clear_rx_ctr() {
	SET_EP_REG(((GET_EP_REG & (~USB_EP_CTR_RX)) & USB_EPREG_MASK) | USB_EP_CTR_TX);
}

void USB_FS::Endpoint::set_rx_size(uint32_t size) {
	uint32_t blck;
	if (size > 62) {
		blck = size >> 5;
		if ((size & 0x1f)==0) {
			--blck;
		}
		USB_PMA_REGS[m_num].rx_count = ((blck << 10)&USB_COUNT0_RX_NUM_BLOCK) | USB_COUNT0_RX_BLSIZE;
	} else {
		blck = size >> 1;
		if (size & 1) {
			++blck;
		}
		USB_PMA_REGS[m_num].rx_count = (blck << 10)&USB_COUNT0_RX_NUM_BLOCK;
	}
	//printf("set_rx_size %x\n", GET_EP_DREG(USB_EP_RX_COUNT));
}
void USB_FS::Endpoint::set_rx_size1(uint32_t size) {
	uint32_t blck;
	if (size > 62) {
		blck = size >> 5;
		if ((size & 0x1f)==0) {
			--blck;
		}
		USB_PMA_REGS[m_num].tx_count = ((blck << 10)&USB_COUNT0_RX_NUM_BLOCK) | USB_COUNT0_RX_BLSIZE;
	} else {
		blck = size >> 1;
		if (size & 1) {
			++blck;
		}
		USB_PMA_REGS[m_num].tx_count = (blck << 10)&USB_COUNT0_RX_NUM_BLOCK;
	}
	//printf("set_rx_size %x\n", GET_EP_DREG(USB_EP_RX_COUNT));
}

uint32_t USB_FS::Endpoint::get_rx_size() const {
	return USB_PMA_REGS[m_num].rx_count & USB_COUNT0_RX_COUNT0_RX;
}
uint32_t USB_FS::Endpoint::get_rx_size1() const {
	return USB_PMA_REGS[m_num].tx_count & USB_COUNT0_RX_COUNT0_RX;
}



void USB_FS::Endpoint::activate(USBEPType type,bool isin,uint32_t pmaaddr,uint32_t maxsize) {
	//printf("init %d\n", (int)m_num);
	if (!m_pmaaddr_tx && !m_pmaaddr_rx) {
		m_type = type;
		SET_EP_REG((GET_EP_REG&USB_EP_T_MASK)|type);
		SET_EP_REG(USB_EP_CTR_RX|USB_EP_CTR_TX|(GET_EP_REG&USB_EPREG_MASK)|m_num);
		//printf("init %x\n", GET_EP_REG);
	}
	m_isin = isin;
	m_is_double = type == USB_EP_TYPE_ISOCHRONOUS;
	m_maxsize = maxsize;

	if (m_isin) {
		m_pmaaddr_tx = pmaaddr;

		clear_tx_dtog();
		
		set_tx_addr(m_pmaaddr_tx);
		//printf("tx0 %x\n", GET_EP_REG);
		if (m_is_double) {
			set_tx_addr1(m_pmaaddr_tx + m_maxsize);
		}
		//printf("tx1 %x\n", GET_EP_REG);
		if (type == USB_EP_TYPE_ISOCHRONOUS) {
			set_tx_stat(USB_EP_TX_STAT_DIS);
		} else {
			set_tx_stat(USB_EP_TX_STAT_NAK);
		}
		//printf("tx2 %x\n", GET_EP_REG);
	} else {
		m_pmaaddr_rx = pmaaddr;
		
		clear_rx_dtog();
	
		set_rx_addr(m_pmaaddr_rx);
		set_rx_size(maxsize);

		if (m_is_double) {
			// some mem
			set_rx_addr1(m_pmaaddr_rx);
			set_rx_size1(maxsize);
		} 

		if (m_num == 0) {
			set_rx_stat(USB_EP_RX_STAT_VALID);
		} else {
			set_rx_stat(USB_EP_RX_STAT_DIS);
		}
		//printf("rx %x\n", GET_EP_REG);
	}
}

void USB_FS::Endpoint::deactivate(bool full) {
	deactivate_rx();
	deactivate_tx();
	if (full) {
		m_pmaaddr_tx = 0;
		m_pmaaddr_rx = 0;
	}
}

void USB_FS::Endpoint::deactivate_rx() {
	m_receive_size = 0;
	m_receive_buf = 0;
	set_rx_stat(USB_EP_RX_STAT_DIS);
	clear_rx_dtog();
}
void USB_FS::Endpoint::deactivate_tx() {
	m_transmit_size = 0;
	m_transmit_buf = 0;
	set_tx_stat(USB_EP_TX_STAT_DIS);
	clear_tx_dtog();
}

void USB_FS::Endpoint::write(const void* data,uint32_t size) {
	uint32_t nbytes = (size + 1) >> 1;   /* nbytes = (wNBytes + 1) / 2 */
  	volatile uint16_t *pdwVal = 0;
  	const uint8_t* pbUsrBuf = (const uint8_t*)data;
  
  	pdwVal = (uint16_t *)(m_pmaaddr_tx * 2 + USB_PMAADDR);
  	for (; nbytes != 0; nbytes--)
  	{
    	uint32_t temp1 = (uint16_t) *pbUsrBuf;
    	pbUsrBuf++;
    	uint32_t temp2 = temp1 | ((uint16_t) *pbUsrBuf) << 8;
    	*pdwVal = temp2;
    	pdwVal+=2;
    	pbUsrBuf++;
  	}
}

void USB_FS::Endpoint::write1(const void* data,uint32_t size) {
	uint32_t nbytes = (size + 1) >> 1;   /* nbytes = (wNBytes + 1) / 2 */
  	volatile uint16_t *pdwVal = 0;
  	const uint8_t* pbUsrBuf = (const uint8_t*)data;
  
  	pdwVal = (uint16_t *)((m_pmaaddr_tx+m_maxsize) * 2 + USB_PMAADDR);
  	for (; nbytes != 0; nbytes--)
  	{
    	uint32_t temp1 = (uint16_t) *pbUsrBuf;
    	pbUsrBuf++;
    	uint32_t temp2 = temp1 | ((uint16_t) *pbUsrBuf) << 8;
    	*pdwVal = temp2;
    	pdwVal+=2;
    	pbUsrBuf++;
  	}
}


void USB_FS::Endpoint::read(void* data,uint32_t size) {
	uint32_t nbytes = (size + 1) >> 1;   /* nbytes = (wNBytes + 1) / 2 */
  	volatile uint32_t *pdwVal = 0;
  	uint8_t* pbUsrBuf = (uint8_t*)data;
  	pdwVal = (uint32_t *)(m_pmaaddr_rx * 2 + USB_PMAADDR);
	for (; nbytes != 0; nbytes--) {
		*(uint16_t*)pbUsrBuf = *pdwVal++;
		pbUsrBuf+=2;
	}
}

void USB_FS::Endpoint::set_stall() {
	if (m_pmaaddr_tx) {
		set_tx_stat(USB_EP_TX_STAT_STALL);
	}
	if (m_pmaaddr_rx) {
		set_rx_stat(USB_EP_RX_STAT_STALL);
	}
}

void USB_FS::Endpoint::set_stall(bool isin) {
	if (isin) {
		if (m_pmaaddr_tx) {
			set_tx_stat(USB_EP_TX_STAT_STALL);
		}
	} else {
		if (m_pmaaddr_rx) {
			set_rx_stat(USB_EP_RX_STAT_STALL);
		}
	}
}

void USB_FS::Endpoint::reset_stall() {
	if (m_pmaaddr_tx) {
		clear_tx_dtog();
		set_tx_stat(USB_EP_TX_STAT_VALID);
	}
	if (m_pmaaddr_rx) {
		clear_rx_dtog();
		set_rx_stat(USB_EP_RX_STAT_VALID);
	}
}


void USB_FS::Endpoint::start_rx() {
	// if ((get_value() & USB_EPRX_STAT) == USB_EP_RX_VALID) {
	// 	return;
	// }
	if ((m_receive_size==0) || (m_receive_size>m_maxsize)) {
		if (!m_is_double || (get_value() & USB_EP_DTOG_RX)==0) 
			set_rx_size(m_maxsize);
		else
			set_rx_size1(m_maxsize);
		// if (m_num!=0) {
		// 	USB_DBG("start rx0 %lu/%lu\n",m_maxsize,m_receive_size);
		// }
	} else {
		if (!m_is_double || (get_value() & USB_EP_DTOG_RX)==0) 
			set_rx_size(m_receive_size);
		else
			set_rx_size1(m_receive_size);
		// if (m_num!=0) {
		// 	USB_DBG("start rx1 %lu\n",m_receive_size);
		// }
	}
	set_rx_stat(USB_EP_RX_STAT_VALID);
}

void USB_FS::Endpoint::receive(void* data,uint32_t size) {
	m_receive_size = size;
	m_receive_buf = data;
	
	size = m_receive_size < m_maxsize ? m_receive_size : m_maxsize;

	// if (m_num!=0) {
	// 	USB_DBG("start rx %lu/%lu\n",size,m_receive_size);
	// }
	if (!m_is_double || (get_value() & USB_EP_DTOG_RX)==0) 
		set_rx_size(size);
	else
		set_rx_size1(size);
	set_rx_stat(USB_EP_RX_STAT_VALID);
}
bool USB_FS::Endpoint::process_receive() {
	// if ((get_value() & USB_EPRX_STAT) == USB_EP_RX_VALID) {
	// 	return false;
	// }
	uint32_t size = (!m_is_double || (get_value() & USB_EP_DTOG_RX)) ? get_rx_size() : get_rx_size1();
	//if (m_num!=0) USB_DBG("process_receive %lu\n",size);
	if (size && m_receive_buf) {
		uint32_t sz = size < m_receive_size ? size : m_receive_size;
		//if (m_num!=0) USB_DBG("received %lu->%lu\n",size,sz);
		read(m_receive_buf,sz);
		m_receive_buf = static_cast<uint8_t*>(m_receive_buf) + sz;
	}
	if (size <= m_receive_size) {
		m_receive_size -= size;	
	} else {
		m_receive_size = 0;
		m_receive_buf = 0;
	}
	clear_rx_ctr();
	return m_receive_size == 0;
}

void USB_FS::Endpoint::transmit(const void* data,uint32_t size) {
	
	if (size > m_maxsize) {
		write(data,m_maxsize);
		set_tx_size(m_maxsize);

		m_transmit_size = size - m_maxsize;
		m_transmit_buf = static_cast<const uint8_t*>(data) + m_maxsize;

	} else {
		m_transmit_buf = 0;
		m_transmit_size = 0;
		if (data)
			write(data,size);
		set_tx_size(size);
	}
	set_tx_stat(USB_EP_TX_STAT_VALID);
}

bool USB_FS::Endpoint::continue_transmit() {
	if (m_transmit_size) {
		transmit(m_transmit_buf,m_transmit_size);
		return true;
	}
	return false;
}

void USB_FS::Endpoint::transmit_iso(const void* data,uint32_t size) {
	bool dtog = (get_value() & USB_EP_DTOG_TX) ;
	size = size <= m_maxsize ? size : m_maxsize;
	
	if (!dtog) {
		write1(data,size);
		set_tx_size1(size);
	} else {
		write(data,size);
		set_tx_size(size);
	}
	set_tx_stat(USB_EP_TX_STAT_VALID);
}

uint32_t USB_FS::m_pma_allocated = 0;
USB_FS::Endpoint USB_FS::m_endpoints[USE_ENDPOINTS];
uint8_t	USB_FS::m_delayed_usb_addr = 0;

USB_FS::USB_FS() {
	m_instance = this;
}

void USB_FS::init() {
	
	for (uint8_t i=0;i<USE_ENDPOINTS;++i) {
		m_endpoints[i].init(i);
	}
	m_delayed_usb_addr = 0;

	// // enable usb
	SET_BIT(RCC->APB1ENR, RCC_APB1ENR_USBEN);
	volatile uint32_t tmpreg = READ_BIT(RCC->APB1ENR, RCC_APB1ENR_USBEN);
	(void)tmpreg;

	disable_interrupts();

	/* Init Device */
  	USB->CNTR = USB_CNTR_FRES;
  	USB->CNTR = 0;
 	USB->ISTR = 0;
 	USB->BTABLE = BTABLE_ADDRESS;

 	
	NVIC_SetPriority(USB_LP_CAN1_RX0_IRQn, 
    	NVIC_EncodePriority(NVIC_GetPriorityGrouping(), 
    		0, 0));
    NVIC_EnableIRQ(USB_LP_CAN1_RX0_IRQn);
    

 	enable_interrupts();

}

void USB_FS::disable_interrupts() {
	USB->CNTR &= ~(USB_CNTR_CTRM  | USB_CNTR_WKUPM | USB_CNTR_SUSPM | 
		USB_CNTR_ERRM | USB_CNTR_ESOFM | USB_CNTR_RESETM);
}
void USB_FS::enable_interrupts() {
	USB->CNTR |= (USB_CNTR_CTRM  | USB_CNTR_WKUPM | USB_CNTR_SUSPM | 
		USB_CNTR_ERRM | USB_CNTR_ESOFM | USB_CNTR_RESETM);
}

void USB_FS::set_address(uint8_t addr) {
	m_delayed_usb_addr = addr;
	if (addr == 0) {
		USB->DADDR = (addr & USB_DADDR_ADD) | USB_DADDR_EF;
	}
}

void USB_FS::stop() {
	USB->CNTR = USB_CNTR_FRES;
	USB->ISTR = 0;
	USB->CNTR = (USB_CNTR_FRES | USB_CNTR_PDWN);
}

void USB_FS::on_ep_irq() {
	volatile uint16_t wIstr = USB->ISTR;  
	
	while (wIstr&USB_ISTR_CTR) {
		uint8_t epindex = (wIstr & USB_ISTR_EP_ID);
		Endpoint& ep(m_endpoints[epindex]);
		if (ep.get_type() == USB_EP_TYPE_ISOCHRONOUS) {
			uint16_t wEPVal = ep.get_value();
			if (wEPVal & USB_EP_CTR_TX) {
				ep.clear_tx_ctr();
				m_instance->on_ep_tx(ep);
			}
			if (wEPVal & USB_EP_CTR_RX) {
				if (ep.process_receive()) {
					m_instance->on_ep_rx(ep);
				} else {
					ep.start_rx();
				}
			}
		} else {
			if ((wIstr & USB_ISTR_DIR) == 0) {
				
				if (ep.continue_transmit()) {
					ep.clear_tx_ctr();
				} else {
					m_instance->on_ep_tx(ep);
					ep.clear_tx_ctr();
					if (m_delayed_usb_addr && epindex == 0) {
						USB->DADDR = (m_delayed_usb_addr & USB_DADDR_ADD) | USB_DADDR_EF;
						m_delayed_usb_addr = 0;
					}
				}
			} else {
				uint16_t wEPVal = ep.get_value();
				if (wEPVal & USB_EP_SETUP) {
					ep.clear_rx_ctr();
					m_instance->on_ep_setup(ep);
				} else if (wEPVal & USB_EP_CTR_RX) {
					if (ep.process_receive()) {
						m_instance->on_ep_rx(ep);
					} else {
						ep.start_rx();
					}
				}
			}
		}

		wIstr = USB->ISTR;  
	}
}

void USB_FS::on_reset_irq() {
	//USB_DBG("reset\n");

	for (uint8_t i=0;i<USE_ENDPOINTS;++i) {
		m_endpoints[i].deactivate(true);
	}
	

	m_pma_allocated = sizeof(usb_ep_pma_regs_t)*USE_ENDPOINTS;
	m_endpoints[0].activate(USB_EP_TYPE_CONTROL,true,m_pma_allocated,USB_MAX_EP0_SIZE);
	m_pma_allocated += USB_MAX_EP0_SIZE;
	m_endpoints[0].activate(USB_EP_TYPE_CONTROL,false,m_pma_allocated,USB_MAX_EP0_SIZE);
	m_pma_allocated += USB_MAX_EP0_SIZE;
}

uint32_t USB_FS::allocate_pma(uint32_t size) {
	uint32_t res = m_pma_allocated;
	m_pma_allocated += size;
	return res;
}

void USB_FS::on_resume_irq() {
	//USB_DBG("resume\n");
}

void USB_FS::on_suspend_irq() {
	//USB_DBG("suspend\n");
}

void USB_FS::on_sof_irq() {
	//USB_DBG("sof\n");
}

void USB_FS::flush_ep(uint8_t ep_addr) {
	if (ep_addr & 0x80) {
		m_endpoints[ep_addr&0x7F].flush_tx();
	} else {
		m_endpoints[ep_addr].flush_rx();
	}
}

#define GET_ISTR_FLAG(F) (((USB->ISTR)&(F))==(F))
#define CLEAR_ISTR_FLAG(F) ((USB->ISTR)&=~(F))

void USB_FS::irq_handler() {
	
  if (GET_ISTR_FLAG(USB_ISTR_CTR)) {
    /* servicing of the endpoint correct transfer interrupt */
    /* clear of the CTR flag into the sub */
    m_instance->on_ep_irq();
  }

  if (GET_ISTR_FLAG ( USB_ISTR_RESET))
  {
    CLEAR_ISTR_FLAG( USB_ISTR_RESET);
    m_instance->on_reset_irq();
    set_address(0);
  }

  if (GET_ISTR_FLAG ( USB_ISTR_PMAOVR))
  {
    CLEAR_ISTR_FLAG( USB_ISTR_PMAOVR);  
    USB_DBG("PMAOVR\n");  
  }
  if (GET_ISTR_FLAG ( USB_ISTR_ERR))
  {
    CLEAR_ISTR_FLAG( USB_ISTR_ERR); 
    USB_DBG("ERR\n"); 
  }

  if (GET_ISTR_FLAG ( USB_ISTR_WKUP))
  {  
    USB->CNTR &= ~(USB_CNTR_LP_MODE);
    
    /*Set interrupt mask*/
    USB->CNTR = USB_CNTR_CTRM  | USB_CNTR_WKUPM | USB_CNTR_SUSPM | USB_CNTR_ERRM \
      | USB_CNTR_ESOFM | USB_CNTR_RESETM;;
    
    m_instance->on_resume_irq();
    
    CLEAR_ISTR_FLAG(USB_ISTR_WKUP);     
  }

  if (GET_ISTR_FLAG (USB_ISTR_SUSP))
  {
    /* clear of the ISTR bit must be done after setting of CNTR_FSUSP */
    CLEAR_ISTR_FLAG( USB_ISTR_SUSP);  
    
    /* Force low-power mode in the macrocell */
    USB->CNTR |= USB_CNTR_FSUSP;
    USB->CNTR |= USB_CNTR_LP_MODE;
    if (!GET_ISTR_FLAG ( USB_ISTR_WKUP))
    {
    	m_instance->on_suspend_irq();
    }
  }

  if (GET_ISTR_FLAG ( USB_ISTR_SOF))
  {
    CLEAR_ISTR_FLAG( USB_ISTR_SOF); 
    m_instance->on_sof_irq();
  }

  if (GET_ISTR_FLAG ( USB_ISTR_ESOF))
  {
    /* clear ESOF flag in ISTR */
    CLEAR_ISTR_FLAG(USB_ISTR_ESOF); 
    //DBG("ESOF\n"); 
  }
}

extern "C" void USB_LP_CAN1_RX0_IRQHandler(void) {
	USB_FS::irq_handler();
}