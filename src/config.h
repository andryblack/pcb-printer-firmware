#ifndef _CONFIG_H_INCLUDED_
#define _CONFIG_H_INCLUDED_

#include "gpio.h"
#include "hw_config.h"
#include "timer.h"


typedef StaticPin<PortB,6> pin_enc_a;
typedef StaticPin<PortB,7> pin_enc_b;

typedef StaticPin<PortA,3> usb_pull_pin;
typedef StaticPin<PortC,13> test_led_pin;

typedef StaticTimer<TimerTIM4>  timer_enc;
#define ENC_TIMER_IRQn TIM4_IRQn
#define ENC_TIMER_IRQ TIM4_IRQHandler
#define ENC_TIMER_IRQ_PRI 1

typedef StaticPin<PortB,13> pin_motor_a;
typedef StaticPin<PortB,14> pin_motor_b;
typedef StaticPin<PortA,8> pin_motor_enable;
#define MOTOR_PWM_CHANNEL 1


typedef StaticTimer<TimerTIM1> timer_motor;
//typedef StaticTimer<TimerTIM3> timer_motor_speed;

#define MOTOR_TIMER_IRQn TIM1_UP_IRQn
#define MOTOR_TIMER_IRQ TIM1_UP_IRQHandler
#define MOTOR_TIMER_IRQ_PRI 4

typedef StaticTimer<TimerTIM2> timer_laser;
#define LASER_FREQ 1000000 /* 1us */
typedef StaticPin<PortA,0> pin_laser;
#define dma_channel_laser DMA1_Channel2

#define LASER_TIMER_IRQn TIM2_IRQn
#define LASER_TIMER_IRQ TIM2_IRQHandler
#define LASER_TIMER_IRQ_PRI 2

typedef StaticTimer<TimerTIM3> timer_stepper;
typedef StaticPin<PortA,6> pin_stepper_step;
typedef StaticPin<PortA,7> pin_stepper_dir;
typedef StaticPin<PortA,5> pin_stepper_enable;
#define STEPPER_PWM_CHANNEL 1

#define STEPPER_TIMER_IRQn TIM3_IRQn
#define STEPPER_TIMER_IRQ TIM3_IRQHandler
#define STEPPER_TIMER_IRQ_PRI 4

#ifdef DEBUG
#include <cstdio>
#define DBG printf
#else
#define DBG(...) do{}while(false)
#endif

#endif