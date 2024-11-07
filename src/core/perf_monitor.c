#include <perf_monitor.h>
#include <page_table.h>
#include <vm.h>
#include <mem.h>

size_t* profiling_results;
size_t* perf_counters;
size_t timer_count;

size_t num_counters = 0;

#define MAX_MEM_SIZE 1024*1024*1024         // 1GB for storing data
#define MAX_EVENTS 6
size_t perf_monitor_results[MAX_EVENTS];


#define PERF_DATA_INDEX(sample_index, event_index, cpu_index, events_num, num_cpus) \
    ((sample_index) * (events_num) * (num_cpus) + (event_index) * (num_cpus) + (cpu_index))

#define PERF_MONITOR_WRITE_SAMPLE(perf_results_arr, sample_index, event_index, cpu_index, events_num, num_cpus, value) \
    perf_results_arr[PERF_DATA_INDEX(sample_index, event_index, cpu_index, events_num, num_cpus)] = value



#define PERF_MONITOR_READ_SAMPLE(perf_results_arr, sample_index, event_index, cpu_index, events_num, num_cpus) \
    perf_results_arr[PERF_DATA_INDEX(sample_index, event_index, cpu_index, events_num, num_cpus)]


#define CHECK_CPUS_MEM_DUMP(bitmap, N) (((bitmap) & ((1UL << (size_t)(N)) - 1)) == ((1UL << (size_t)(N)) - 1))


void perf_monitor_init(struct vm* vm, struct perf_monitor_config perf_config) {

    if(cpu_is_master()){
        // vaddr for storing PMU events
        vm->perf_monitor.events_num = perf_config.events_num;
        size_t samples_exe = perf_config.num_samples;
        size_t mem_reg_size =   sizeof(size_t) *
                                vm->perf_monitor.events_num *
                                perf_config.num_samples *
                                vm->cpu_num;

    //    console_printk("MEM size reserved: %d - Mem size needed: %d\n", MAX_MEM_SIZE, mem_reg_size); 

        if(mem_reg_size > MAX_MEM_SIZE){   
            size_t rem_samples = (sizeof(size_t) * vm->perf_monitor.events_num * vm->perf_monitor.num_profiling_samples * vm->cpu_num)/MAX_MEM_SIZE;
            WARNING("Exceded number of samples by %d", rem_samples);
            samples_exe = perf_config.num_samples - rem_samples;
        }

        vm->perf_monitor.num_profiling_samples = samples_exe;

        size_t num_pages = NUM_PAGES(MAX_MEM_SIZE);
        struct ppages* pa_ptr = NULL;
        vaddr_t va = mem_alloc_map(&cpu()->as, SEC_HYP_GLOBAL, pa_ptr, INVALID_VA, num_pages, PTE_HYP_FLAGS);

        if (va == INVALID_VA) {
            ERROR("failed to VM's perf monitor region");
        }
        vm->perf_monitor.array_profiling_results = (size_t*)va;
    }
    
    cpu_sync_barrier(&vm->sync);

    vm->perf_monitor.array_sample_index[cpu()->id] = 0;
    vm->perf_monitor.array_sample_index[cpu()->id] = 0;
    vm->perf_monitor.cpu_mem_dump_bitmap = 0;

    perf_monitor_setup_event_counters(perf_config.events, perf_config.events_num);
    perf_monitor_timer_init(perf_config.sampling_period_us);
}


void perf_monitor_setup_event_counters(size_t* events, size_t num_events) {

    num_counters = num_events;
    events_enable();

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
}

void perf_monitor_timer_init(size_t perf_monitor_period_us) {
    timer_define_irq_callback(perf_monitor_irq_handler);
    timer_count = timer_init(perf_monitor_period_us);
}

void perf_monitor_irq_handler(unsigned int irq) {
    (void) irq;
    timer_disable();

    if(cpu()->vcpu->vm->perf_monitor.array_sample_index[cpu()->id] >= cpu()->vcpu->vm->perf_monitor.num_profiling_samples) {
    
        cpu()->vcpu->vm->perf_monitor.cpu_mem_dump_bitmap ^= (1UL << (size_t)cpu()->id);         // bitmap to know which cpu as reached this position

        // Dump results on cpu master
        if(CHECK_CPUS_MEM_DUMP(cpu()->vcpu->vm->perf_monitor.cpu_mem_dump_bitmap, cpu()->vcpu->vm->cpu_num) && cpu_is_master())
        {
            size_t profiling_result = 0;

            for(size_t cpu_index = 0; cpu_index < cpu()->vcpu->vm->cpu_num; cpu_index++)       // Read each CPU
            {
                console_printk("\n\t--CPU-%d--\n\n", cpu_index);

                for(size_t sample_index = 0; sample_index < cpu()->vcpu->vm->perf_monitor.num_profiling_samples; sample_index++)       // Read each sample
                {
                    for(size_t counter_index = 0; counter_index < cpu()->vcpu->vm->perf_monitor.events_num; counter_index++)
                    {
                        profiling_result = PERF_MONITOR_READ_SAMPLE(cpu()->vcpu->vm->perf_monitor.array_profiling_results,
                                                                    sample_index,
                                                                    counter_index,
                                                                    cpu_index,
                                                                    cpu()->vcpu->vm->perf_monitor.events_num,
                                                                    cpu()->vcpu->vm->cpu_num);

                        console_printk("[%d][%d]=%lu\n",sample_index, counter_index, profiling_result);
                    }
        
                }
            }
        }
    }
    else 
    {
        for(size_t counter_id = 0; counter_id < num_counters; counter_id++) {
            size_t counter_value = events_get_cntr_value(counter_id);

            PERF_MONITOR_WRITE_SAMPLE(cpu()->vcpu->vm->perf_monitor.array_profiling_results,
                                    cpu()->vcpu->vm->perf_monitor.array_sample_index[cpu()->id],
                                    counter_id,
                                    cpu()->id,
                                    cpu()->vcpu->vm->perf_monitor.events_num,
                                    cpu()->vcpu->vm->cpu_num,
                                    counter_value);

            events_clear_cntr_ovs(counter_id);
            events_cntr_set(counter_id, UINT32_MAX);
        }

        cpu()->vcpu->vm->perf_monitor.array_sample_index[cpu()->id]++;
    }

    timer_reschedule_interrupt(timer_count);
    timer_enable();
}
