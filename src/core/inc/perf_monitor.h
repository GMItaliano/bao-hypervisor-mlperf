/**
 * SPDX-License-Identifier: Apache-2.0
 * Copyright (c) Bao Project and Contributors. All rights reserved.
 */

#ifndef __PERF_MONITOR_H__
#define __PERF_MONITOR_H__

#include <bao.h>
#include <timer.h>
#include <events.h>
#include <page_table.h>
#include <mem.h>
#include <spinlock.h>
#include <vm.h> 
#include <vmm.h>


#define PERF_MONITOR_MAX_EVENTS 32
#define PERF_MONITOR_MAX_CPUS   32

// struct perf_monitor_hv{
//     size_t num_vms[MAX_NUM_VMS];
//     size_t* profiling_results;
// };

struct perf_monitor_vm {
    size_t array_events[PERF_MONITOR_MAX_EVENTS];
    size_t array_sample_index[PERF_MONITOR_MAX_CPUS];
    size_t events_num;
    size_t sampling_period_us;

    size_t* array_profiling_results;
    size_t num_profiling_samples;
    size_t cpu_mem_dump_bitmap;
};

struct perf_monitor_hypervisor {
    size_t events_num;
    size_t* events;
    size_t sampling_period_us;                      // period is global
    size_t num_profiling_samples;                   // number of samples needed
    size_t vm_mem_dump_bitmap;                      // To know when each VM as finished all samples, each bit to know when a vm as finished
    vaddr_t mem_addr;
    size_t num_vms;
    size_t num_cpus_en;
};

struct perf_monitor_config {
    size_t events_num;
    size_t* events;
    size_t sampling_period_us;
    size_t num_samples;
};

extern struct perf_monitor_hypervisor hv_perf_monitor;

// Hypervisor perf monitot struct creation
struct perf_monitor_hypervisor perf_monitor_init_hypervisor(struct perf_monitor_config hv_perf_config, struct vm_config* hv_vm_list, size_t hv_vmlist_size);

void perf_monitor_init(struct vm* vm);
void perf_monitor_setup_event_counters(size_t* events, size_t num_events);
void perf_monitor_timer_init(size_t perf_monitor_period_us);
void perf_monitor_irq_handler(unsigned int irq);

#endif /* __PERF_MONITOR_H__ */