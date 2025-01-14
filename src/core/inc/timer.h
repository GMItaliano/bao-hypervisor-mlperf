#ifndef __TIMER_MOD_H__
#define __TIMER_MOD_H__s

#include <interrupts.h>
#include <arch/timer.h>

/* timer initialization */
static inline uint64_t timer_init(uint64_t period)
{
	return timer_arch_init(period);
}

/* enable timer */
static inline void timer_enable(void)
{
	timer_arch_enable();
}

/* disable timer */
static inline void timer_disable(void)
{
	timer_arch_disable();
}

/* define interrupt timer callback */
static inline void timer_define_irq_callback(irq_handler_t handler)
{
	timer_arch_define_irq_callback(handler);
}

/* clean timer overflow flags */
static inline void timer_reschedule_interrupt(uint64_t count)
{
	timer_arch_reschedule_interrupt(count);
}

/*  */
static inline uint64_t timer_reschedule_interrupt_us(uint64_t period)
{
	return timer_arch_reschedule_interrupt_us(period);
}

static inline uint64_t timer_get(void)
{
    return timer_arch_get();
}

static inline uint64_t timer_freq(void)
{
    return timer_arch_freq(); 
}


#endif /* __TIMER_MOD_H__ */