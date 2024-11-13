#include <perf_monitor.h>


spinlock_t lock;

size_t* profiling_results;
size_t* perf_counters;
size_t timer_count;

size_t num_counters = 0;

#define MAX_MEM_SIZE 1024*1024*1024         // 1GB for storing data
#define MAX_EVENTS 6

size_t perf_monitor_results[MAX_EVENTS];

struct perf_monitor_hypervisor cpu_perf_monitor;

#define PERF_DATA_INDEX(sample_index, event_index, cpu_index, events_num, num_cpus) \
    ((sample_index) * (events_num) * (num_cpus) + (event_index) * (num_cpus) + (cpu_index))

#define PERF_MONITOR_WRITE_SAMPLE(perf_results_arr, sample_index, event_index, cpu_index, events_num, num_cpus, value) \
    perf_results_arr[PERF_DATA_INDEX(sample_index, event_index, cpu_index, events_num, num_cpus)] = value



#define PERF_MONITOR_READ_SAMPLE(perf_results_arr, sample_index, event_index, cpu_index, events_num, num_cpus) \
    perf_results_arr[PERF_DATA_INDEX(sample_index, event_index, cpu_index, events_num, num_cpus)]


#define CHECK_CPUS_MEM_DUMP(bitmap, N) (((bitmap) & ((1UL << (size_t)(N)) - 1)) == ((1UL << (size_t)(N)) - 1))

/*
********************* HELP Functions ***********************
*/

// int perf_count_set_bits(unsigned long num) {
//     int count = 0;
//     while (num) {
//         count += num & 1;
//         num >>= 1;
//     }
//     return count;
// }

/*
************************************************************
*/

struct perf_monitor_hypervisor perf_monitor_init_hypervisor(struct perf_monitor_config hv_perf_config, struct vm_config* hv_vm_list, size_t hv_vmlist_size) {
    
    cpu_perf_monitor.events = hv_perf_config.events;
    cpu_perf_monitor.events_num = hv_perf_config.events_num;
    cpu_perf_monitor.num_vms = hv_vmlist_size;
    cpu_perf_monitor.vm_mem_dump_bitmap = 0;

    for(size_t i = 0; i < cpu_perf_monitor.num_vms; i++)
        cpu_perf_monitor.num_cpus_en += hv_vm_list[i].platform.cpu_num;

    size_t samples_exe = hv_perf_config.num_samples;
    
    size_t mem_reg_size =   sizeof(size_t) *
                            cpu_perf_monitor.events_num *
                            samples_exe *
                            cpu_perf_monitor.num_cpus_en;

    //    console_printk("MEM size reserved: %d - Mem size needed: %d\n", MAX_MEM_SIZE, mem_reg_size); 

    if(mem_reg_size > MAX_MEM_SIZE){   
        size_t rem_samples = (sizeof(size_t) * cpu_perf_monitor.events_num * cpu_perf_monitor.num_profiling_samples * cpu_perf_monitor.num_cpus_en)/MAX_MEM_SIZE;
        WARNING("Exceded number of samples by %d", rem_samples);
        samples_exe = hv_perf_config.num_samples - rem_samples;
    }

    cpu_perf_monitor.num_profiling_samples = samples_exe;

    size_t num_pages = NUM_PAGES(mem_reg_size);         //Save max 1GB
    struct ppages* pa_ptr = NULL;
    vaddr_t va = mem_alloc_map(&cpu()->as, SEC_HYP_GLOBAL, pa_ptr, INVALID_VA, num_pages, PTE_HYP_FLAGS);

    if (va == INVALID_VA) {
        ERROR("failed to VM's perf monitor region");
    }
    cpu_perf_monitor.mem_addr = va;
    
    console_printk("\n> SETUP CONFIG: NUM Samples: %d Samples period: %d, event 0: %d\n", cpu_perf_monitor.num_profiling_samples, hv_perf_config.sampling_period_us, hv_perf_config.events[0]);

    return cpu_perf_monitor;
}

void perf_monitor_init(struct vm* vm) {

    if(cpu()->vcpu->id == cpu()->vcpu->vm->master){
        // vaddr for storing PMU events
        vm->perf_monitor.array_profiling_results = (size_t*)hv_perf_monitor.mem_addr;
        vm->perf_monitor.num_profiling_samples = hv_perf_monitor.num_profiling_samples;
        vm->perf_monitor.sampling_period_us = hv_perf_monitor.sampling_period_us;
        vm->perf_monitor.array_events = hv_perf_monitor.events;
        vm->perf_monitor.events_num = hv_perf_monitor.events_num;
    }

    cpu_sync_barrier(&vm->sync);

    vm->perf_monitor.array_sample_index[cpu()->vcpu->id] = 0;
    vm->perf_monitor.cpu_mem_dump_bitmap = 0;

    perf_monitor_setup_event_counters(vm->perf_monitor.array_events, vm->perf_monitor.events_num);
    perf_monitor_timer_init(vm->perf_monitor.sampling_period_us);
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

    if(cpu()->vcpu->vm->perf_monitor.array_sample_index[cpu()->vcpu->id] >= cpu()->vcpu->vm->perf_monitor.num_profiling_samples) {
    
        cpu()->vcpu->vm->perf_monitor.cpu_mem_dump_bitmap ^= (1UL << (size_t)cpu()->vcpu->id);         // bitmap to know which cpu as reached this position
        
        if(CHECK_CPUS_MEM_DUMP(cpu()->vcpu->vm->perf_monitor.cpu_mem_dump_bitmap, cpu()->vcpu->vm->cpu_num))    
            hv_perf_monitor.vm_mem_dump_bitmap ^= (1UL << (size_t)cpu()->vcpu->vm->id);
        
        // Dump results on cpu master
        if(CHECK_CPUS_MEM_DUMP(hv_perf_monitor.vm_mem_dump_bitmap, hv_perf_monitor.num_vms) && cpu_is_master())
        {
            size_t profiling_result = 0;

            for(size_t cpu_index = 0; cpu_index < hv_perf_monitor.num_cpus_en; cpu_index++)       // Read each CPU
            {
                console_printk("\n\t--VM::%d--vCPU::%d--\n\n",cpu()->vcpu->vm->id, cpu_index);

                for(size_t sample_index = 0; sample_index < cpu()->vcpu->vm->perf_monitor.num_profiling_samples; sample_index++)       // Read each sample
                {
                    console_printk("s%d\n", sample_index);
                    profiling_result = 0;

                    for(size_t counter_index = 0; counter_index < cpu()->vcpu->vm->perf_monitor.events_num; counter_index++)
                    {
                        profiling_result = PERF_MONITOR_READ_SAMPLE(((size_t*)hv_perf_monitor.mem_addr),
                                                                    sample_index,
                                                                    counter_index,
                                                                    cpu_index,
                                                                    cpu()->vcpu->vm->perf_monitor.events_num,
                                                                    hv_perf_monitor.num_cpus_en);

                        console_printk("c%d=%lu\n", counter_index, profiling_result);
                    }
        
                }
            }
            cpu()->vcpu->vm->perf_monitor.cpu_mem_dump_bitmap = 0;
        }
        else if(cpu_is_master())
        {
            timer_reschedule_interrupt(timer_count);
            timer_enable();
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
                                    hv_perf_monitor.num_cpus_en,
                                    counter_value);

            events_clear_cntr_ovs(counter_id);
            events_cntr_set(counter_id, UINT32_MAX);
        }

        cpu()->vcpu->vm->perf_monitor.array_sample_index[cpu()->vcpu->id]++;

        timer_reschedule_interrupt(timer_count);
        timer_enable();
    }
}
