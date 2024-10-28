#include <arch/pmu.h>
#include <cpu.h>
#include <bitmap.h>

uint64_t pmu_cntr_alloc(void)
{
    uint32_t index = PMU_N_CNTR_GIVEN;

    for (unsigned int __bit = bitmap_get(cpu()->perf_events_bitmap, index); index < cpu()->perf_events_counters_num;
                        __bit = bitmap_get(cpu()->perf_events_bitmap, ++index))
        if(!__bit)
            break;

    if(index == cpu()->perf_events_counters_num)
        return ERROR_NO_MORE_EVENT_COUNTERS;

    bitmap_set(cpu()->perf_events_bitmap, index);
    return index;
}

void pmu_cntr_free(uint64_t counter)
{
    bitmap_clear(cpu()->perf_events_bitmap, counter);
}

/* Enable the pmu in the EL2*/
void pmu_enable(void)
{
    uint32_t pmcr = (uint32_t) sysreg_pmcr_el0_read(); //MRS(PMCR_EL0);
    uint64_t mdcr = (uint32_t) sysreg_mdcr_el2_read(); //MRS(MDCR_EL2);

    cpu()->perf_events_counters_num = ((pmcr & PMCR_EL0_N_MASK) >> PMCR_EL0_N_POS);

    mdcr &= (uint64_t) ~MDCR_EL2_HPMN_MASK;
	mdcr |= (uint64_t) MDCR_EL2_HPME + (PMU_N_CNTR_GIVEN);

    sysreg_mdcr_el2_write(mdcr); //MSR(MDCR_EL2, mdcr);
}
