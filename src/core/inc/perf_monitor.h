/**
 * SPDX-License-Identifier: Apache-2.0
 * Copyright (c) Bao Project and Contributors. All rights reserved.
 */

#ifndef __PERF_MONITOR_H__
#define __PERF_MONITOR_H__

#include <bao.h>
#include <timer.h>
#include <events.h>

struct perf_monitor {
    size_t* array_events;
    size_t* array_sample_index;
    size_t events_num;
    size_t sampling_period_us;

    size_t* array_profiling_results;
    size_t num_profiling_samples;
};

struct perf_monitor_config {
    uint64_t events_num;
    size_t* events;
    uint64_t sampling_period_us;
    paddr_t results_base_addr;
    size_t num_samples;
};

void perf_monitor_init(struct vm* vm, struct perf_monitor_config perf_config);
void perf_monitor_setup_event_counters(size_t* events, size_t num_events);
void perf_monitor_timer_init(size_t perf_monitor_period_us);
void perf_monitor_irq_handler(unsigned int irq);

#endif /* __PERF_MONITOR_H__ */