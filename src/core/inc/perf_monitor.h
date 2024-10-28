/**
 * SPDX-License-Identifier: Apache-2.0
 * Copyright (c) Bao Project and Contributors. All rights reserved.
 */

#ifndef __PERF_MONITOR_H__
#define __PERF_MONITOR_H__

#include <bao.h>
#include <timer.h>
#include <config.h>
#include <events.h>

typedef struct {
    size_t events_num;
    size_t* events;
    size_t sampling_period_us;
    size_t sample_index;
    size_t* perf_counters;

    size_t* profiling_results;
    size_t num_profiling_samples;
} perf_monitor;


void perf_monitor_init(struct perf_monitor_config config);
void perf_monitor_setup_event_counters(size_t* events, size_t num_events);
void perf_monitor_timer_init(size_t perf_monitor_period_us);
void perf_monitor_irq_handler(unsigned int irq);

#endif /* __PERF_MONITOR_H__ */