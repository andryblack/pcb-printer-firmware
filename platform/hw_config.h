#ifndef _HW_CONFIG_H_INCLUDED_
#define _HW_CONFIG_H_INCLUDED_

#define FREQ_HSE 		8000000UL
#define FREQ_PLLCLK 	(FREQ_HSE * 9)
#define FREQ_CPU 		FREQ_PLLCLK
#define FREQ_PCLK1 		(FREQ_CPU / 2)
#define FREQ_PCLK2 		(FREQ_CPU)

#define FREQ_APB1_TIMER (FREQ_PCLK1*2)
#define FREQ_APB2_TIMER (FREQ_PCLK2)

#endif /*_HW_CONFIG_H_INCLUDED_*/