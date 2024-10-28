/**
 * SPDX-License-Identifier: Apache-2.0 
 * Copyright (c) Bao Project and Contributors. All rights reserved
 */

#ifndef __EVENTS_H__
#define __EVENTS_H__

#include <arch/events.h>

#define EVENTS_CNTR_MAX_NUM    EVENTS_ARCH_CNTR_MAX_NUM


/**
 * @brief Enumeration defining different events for performance event counters.
 */
typedef enum {
    mem_access = 0,            // Memory access event.
    l2_cache_access,           // L2 cache access event.
    bus_access,                // Bus access event.
    external_mem_request,      // External memory request event.
    l2_cache_refill            // L2 cache refill event.
} events_enum;

/**
 * @brief Allocates an event counter.
 *
 * @return The allocated event counter.
 */
static inline size_t events_cntr_alloc(void) {
    return events_arch_cntr_alloc();
}

/**
 * @brief Frees the specified event counter.
 *
 * @param counter The event counter to free.
 */
static inline void events_cntr_free(size_t counter) {
    events_arch_cntr_free(counter);
}

/**
 * @brief Enables event.
 */
static inline void events_enable(void) {
    events_arch_enable();
}

/**
 * @brief Disables event.
 */
static inline void events_disable(void) {
    events_arch_disable();
}

/**
 * @brief Enables the specified event counter.
 *
 * @param counter The event counter to enable.
 * @return 0 if successful, or an error code if failed.
 */
static inline int events_cntr_enable(size_t counter) {
    return events_arch_cntr_enable(counter);
}

/**
 * @brief Disables the specified event counter.
 *
 * @param counter The event counter to disable.
 */
static inline void events_cntr_disable(size_t counter) {
    events_arch_cntr_disable(counter);
}

/**
 * @brief Sets the value of the specified event counter.
 *
 * @param counter The event counter to set the value for.
 * @param value The value to set.
 */
static inline void events_cntr_set(size_t counter, unsigned long value) {
    events_arch_cntr_set(counter, value);
}

/**
 * @brief Gets the current value of the specified event counter.
 *
 * @param counter The event counter to get the value from.
 * @return The current value of the event counter.
 */
static inline uint64_t events_get_cntr_value(size_t counter) {
    return events_arch_get_cntr_value(counter);
}

/**
 * @brief Sets the event type for the specified event counter.
 *
 * @param counter The event counter to set the event type for.
 * @param event The event type to set.
 */
static inline void events_set_evtyper(size_t counter, events_enum event) {
    events_arch_set_evtyper(counter, event);
}

/**
 * @brief Clears any counter overflows for the specified event counter.
 *
 * @param counter The event counter to clear overflows for.
 */
static inline void events_clear_cntr_ovs(size_t counter) {
    events_arch_clear_cntr_ovs(counter);
}


#endif /* __EVENTS_H__ */