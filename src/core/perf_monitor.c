#include <perf_monitor.h>
#include <page_table.h>
#include <vm.h>
#include <mem.h>

size_t* profiling_results;
size_t* perf_counters;
size_t timer_count;

size_t num_counters = 0;

#define MAX_EVENTS 6
size_t perf_monitor_results[MAX_EVENTS];

perf_monitor performance_monitor;

#define NUM_PERF_MON_SAMPLES    3
void perf_monitor_init(struct perf_monitor_config perf_config) {

    size_t mem_reg_size = NUM_PAGES(perf_config.results_size);
    struct ppages* pa_ptr = NULL;
    vaddr_t va = mem_alloc_map(&cpu()->as, SEC_HYP_GLOBAL, pa_ptr, INVALID_VA, mem_reg_size, PTE_HYP_FLAGS);
    if (va == INVALID_VA) {
        ERROR("failed to VM's perf monitor region");
    }

    performance_monitor.profiling_results = (size_t*)va;
    performance_monitor.events_num = perf_config.events_num;
    performance_monitor.num_profiling_samples = NUM_PERF_MON_SAMPLES * perf_config.events_num;
    perf_monitor_setup_event_counters(perf_config.events, perf_config.events_num);
    perf_monitor_timer_init(perf_config.sampling_period_us);
}


void perf_monitor_setup_event_counters(size_t* events, size_t num_events) {

    num_counters = num_events;

    for(size_t i = 0; i < num_events; i++) {

        size_t counter_id = events_cntr_alloc();
        if(counter_id == ERROR_NO_MORE_EVENT_COUNTERS) {
            ERROR("Failed to allocate event counter for event %lu", events[i]);
        }

        events_set_evtyper(counter_id, events[i]);
        events_cntr_set(counter_id, UINT32_MAX);
        events_clear_cntr_ovs(counter_id);
        events_cntr_enable(counter_id);
    }

    events_enable();
}

void perf_monitor_timer_init(size_t perf_monitor_period_us) {
    // Setup the timer to trigger the perf_monitor_timer_irq_handler
    // at the specified sampling period
    timer_define_irq_callback(perf_monitor_irq_handler);
    // cpu.mem_throt.period_counts = timer_init(cpu.mem_throt.period_us);
    timer_count = timer_init(perf_monitor_period_us);
}

void perf_monitor_irq_handler(unsigned int irq) {
    // Read the performance counters and store the results
    // in the profiling_results array
    (void) irq;
    timer_disable();
    console_printk("performance monitor interrupt %d\n", performance_monitor.sample_index);

    for(size_t counter_id = 0; counter_id < num_counters; counter_id++) {
        performance_monitor.profiling_results[performance_monitor.sample_index] = events_get_cntr_value(counter_id);
        performance_monitor.sample_index++;
        events_clear_cntr_ovs(counter_id);
        events_cntr_set(counter_id, UINT32_MAX);
    }

    if(performance_monitor.sample_index >= performance_monitor.num_profiling_samples) {
        console_printk("Profiling results array is full\n");
        for(size_t i = 0; i < performance_monitor.num_profiling_samples; i++) {
            console_printk("Event %lu: %lu\n", i, performance_monitor.profiling_results[i]);
        }
        performance_monitor.sample_index = SIZE_MAX;
    }

    timer_reschedule_interrupt(timer_count);
    timer_enable();
}
