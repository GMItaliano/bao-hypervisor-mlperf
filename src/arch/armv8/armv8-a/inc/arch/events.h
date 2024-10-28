#ifndef __ARCH_EVENTS_H__
#define __ARCH_EVENTS_H__

#include <arch/pmu.h>

#define EVENTS_ARCH_CNTR_MAX_NUM    PMU_CNTR_MAX_NUM

static inline size_t events_arch_cntr_alloc(void)
{
    return pmu_cntr_alloc();
}

static inline void events_arch_cntr_free(size_t counter)
{
    pmu_cntr_free(counter);
}

/* enable events */
static inline void events_arch_enable(void)
{
    pmu_enable();
}

/* disable events */
static inline void events_arch_disable(void)
{
    pmu_disable();
}

/* enable events counter */
static inline int events_arch_cntr_enable(size_t counter)
{
    return pmu_cntr_enable(counter);
}

/* disable events counter */
static inline void events_arch_cntr_disable(size_t counter)
{
    pmu_cntr_disable(counter);
}

/* set event counter value */
static inline void events_arch_cntr_set(size_t counter, unsigned long value)
{
    pmu_cntr_set(counter, value);
}

/* get event counter value */
static inline uint64_t events_arch_get_cntr_value(size_t counter)
{
    return pmu_cntr_get(counter);
}

/* define event to count */
static inline void events_arch_set_evtyper(size_t counter, size_t event)
{
    pmu_set_evtyper(counter, event);
}

static inline void events_arch_clear_cntr_ovs(size_t counter)
{
    pmu_clear_cntr_ovs(counter);
}


#endif /* __ARCH_EVENTS_H__ */