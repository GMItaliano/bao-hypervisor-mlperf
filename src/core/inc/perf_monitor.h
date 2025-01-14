/**
 * SPDX-License-Identifier: Apache-2.0
 * Copyright (c) Bao Project and Contributors. All rights reserved.
 */

#ifndef __PERF_MONITOR_H__
#define __PERF_MONITOR_H__

#include <bao.h>
#include <timer.h>
#include <events.h>

#define PERF_MONITOR_MAX_EVENTS 32
#define PERF_MONITOR_MAX_CPUS   32
#define PERF_TICK_COUNT         30000

struct perf_monitor {
    size_t array_events[PERF_MONITOR_MAX_EVENTS];
    size_t array_sample_index[PERF_MONITOR_MAX_CPUS];
    size_t events_num;
    size_t sampling_period_us;

    size_t* array_profiling_results;
    size_t num_profiling_samples;
    size_t cpu_mem_dump_bitmap;

    size_t array_cpus_id[PERF_MONITOR_MAX_CPUS];
    size_t array_tick_count[PERF_MONITOR_MAX_CPUS][PERF_TICK_COUNT];
};

struct perf_monitor_config {
    size_t events_num;
    size_t* events;
    size_t sampling_period_us;
    size_t num_samples;
};

void perf_monitor_init(struct vm* vm, struct perf_monitor_config perf_config);
void perf_monitor_setup_event_counters(size_t* events, size_t num_events);
void perf_monitor_timer_init(size_t perf_monitor_period_us);
void perf_monitor_irq_handler(unsigned int irq);

#endif /* __PERF_MONITOR_H__ */