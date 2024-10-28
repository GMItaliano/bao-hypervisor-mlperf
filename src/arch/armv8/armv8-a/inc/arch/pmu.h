#ifndef __ARCH_PMU_H__
#define __ARCH_PMU_H__

#include <interrupts.h>
#include <arch/sysregs.h>
#include <platform.h>
#include <printk.h>
#include <bit.h>

#define PMU_CNTR_MAX_NUM 32

#if !defined(UINT32_MAX)
#define UINT32_MAX		0xffffffffU /* 4294967295U */
#endif


#define PMCR_EL0_N_POS		(11)
#define PMCR_EL0_N_MASK		(0x1F << PMCR_EL0_N_POS)

#define MDCR_EL2_HPME		(1 << 7)
#define MDCR_EL2_HPMN_MASK	(0x1F)

#define PMEVTYPER_P				31 
#define PMEVTYPER_U				30 
#define PMEVTYPER_NSK			29 
#define PMEVTYPER_NSU			28 
#define PMEVTYPER_NSH			27  
#define PMEVTYPER_M				26 
#define PMEVTYPER_MT			25
#define PMEVTYPER_SH			24

#define PMU_N_CNTR_GIVEN    1

#define ERROR_NO_MORE_EVENT_COUNTERS    UINT64_MAX


uint64_t pmu_cntr_alloc(void);
void pmu_cntr_free(uint64_t);
void pmu_enable(void);
void pmu_interrupt_enable(uint64_t cpu_id);
void pmu_define_event_cntr_irq_callback(irq_handler_t handler, size_t counter);


/* Disable the pmu */
/**
 * @brief Disables the Performance Monitor Unit (PMU).
 */
static inline void pmu_disable(void) {
    uint64_t mdcr;

    // Read the current value of the MDCR_EL2 register to configure the PMU.
    mdcr = sysreg_mdcr_el2_read(); 

    // MDCR_EL2_HPME --> Hypervisor Performance Monitor Enable, bit [7]
    // --> 0b0 - EL2 performance monitor counters disabled
    // --> 0b1 - EL2 performance monitor counters enabled
    mdcr = bit_clear(mdcr, MDCR_EL2_HPME);

    // Write the updated value back to the MDCR_EL2 register to disable the PMU.
    sysreg_mdcr_el2_write(mdcr);
}

/**
 * @brief Enables the specified PMU counter.
 *
 * @param counter The PMU counter to enable.
 * @return 0 on success, or an error code on failure.
 */
static inline int pmu_cntr_enable(size_t counter) {
    uint64_t pmcntenset;
    
    // PMCNTENSET_EL0 -> Performance Monitors Count Enable Set Register
    // Read the current value of the PMCNTENSET_EL0 register to enable
    // the PMU counter.
    pmcntenset = sysreg_pmcntenset_el0_read();

    // Set the bit corresponding to the specified PMU counter in the
    // PMCNTENSET_EL0 register.
    pmcntenset = bit_set(pmcntenset, counter);

    // Write the updated value back to the PMCNTENSET_EL0 register to enable
    // the PMU counter.
    sysreg_pmcntenset_el0_write(pmcntenset);

    return 0;
}

/**
 * @brief Disables the specified PMU counter.
 *
 * @param counter The PMU counter to disable.
 */
static inline void pmu_cntr_disable(size_t counter) {
    uint64_t pmcntenclr = 0;

    // PMCNTENCLR_EL0 -> Performance Monitors Count Enable Clear Register
    // Set the bit corresponding to the specified PMU counter in the
    // PMCNTENCLR_EL0 register.
    pmcntenclr = bit_set(pmcntenclr, counter);

    // Write the value of pmcntenclr to the PMCNTENCLR_EL0 register to disable
    // the PMU counter.
    sysreg_pmcntenclr_el0_write(pmcntenclr);
}

/**
 * @brief Sets the specified PMU counter with the given value.
 *
 * @param counter The PMU counter to set.
 * @param value The value to set for the PMU counter.
 */
static inline void pmu_cntr_set(size_t counter, unsigned long value) {
    uint64_t pmselr;

    // PMSELR_EL0 -> Performance Monitors Event Counter Selection Register
    pmselr = sysreg_pmselr_el0_read();
    pmselr = bit_insert(pmselr, counter, 0, 5);
    sysreg_pmselr_el0_write(pmselr);

    // Set the PMU counter to overflow after the occurrence of "value" events
    value = UINT32_MAX - value;
    sysreg_pmxevcntr_el0_write(value);
}

/**
 * @brief Gets the value of the specified PMU counter.
 *
 * @param counter The PMU counter to get the value from.
 * @return The value of the specified PMU counter.
 */
static inline unsigned long pmu_cntr_get(size_t counter) {
    uint64_t pmselr;

     // PMSELR_EL0 -> Performance Monitors Event Counter Selection Register
    pmselr = sysreg_pmselr_el0_read();
    pmselr = bit_insert(pmselr, counter, 0, 5);
    sysreg_pmselr_el0_write(pmselr);

    // Read the value of the PMU counter from the PMXEVCNTR_EL0 register.
    return sysreg_pmxevcntr_el0_read();
}


/**
 * @brief Sets the event type for the specified PMU counter.
 *
 * @param counter The PMU counter for which to set the event type.
 * @param event The event type to set for the PMU counter.
 */
static inline void pmu_set_evtyper(size_t counter, size_t event) {
    uint64_t pmselr;

    // PMSELR_EL0 -> Performance Monitors Event Counter Selection Register
    pmselr = sysreg_pmselr_el0_read();
    pmselr = bit_insert(pmselr, counter, 0, 5);
    sysreg_pmselr_el0_write(pmselr);

    // Read the current value of the PMXEVTYPER_EL0 register to configure the
    // event type.
    uint64_t pmxevtyper = sysreg_pmxevtyper_el0_read();

    // Clear all bits in the PMXEVTYPER_EL0 register related to the event type.
    // PMXEVTYPER_EL0 --> P, bit [31]
    // --> 0b0 - Count events in EL1.
    // --> 0b1 - Do not count events in EL1.
    pmxevtyper = bit_clear(pmxevtyper, PMEVTYPER_P);

    // PMXEVTYPER_EL0 --> U, bit [30]
    // 0b0 - Count events in EL0.
    // 0b1 - Do not count events in EL0.
    pmxevtyper = bit_clear(pmxevtyper, PMEVTYPER_U);

    // PMXEVTYPER_EL0 --> NSK, bit [29]
    // 0b0 - Do not count events in Non-secure EL1.
    // 0b1 - Count events in Non-secure EL1 if equal to PMEVTYPER<n>_EL0.P.
    pmxevtyper = bit_clear(pmxevtyper, PMEVTYPER_NSK);

    // PMXEVTYPER_EL0 --> NSU, bit [28]
    // - If the value of this bit is equal to the value of the 
    //   PMEVTYPER<n>_EL0.U bit, events in Non-secure EL0 are counted.
    // - Otherwise, events in Non-secure EL0 are not counted.
    pmxevtyper = bit_clear(pmxevtyper, PMEVTYPER_NSU);

    // PMXEVTYPER_EL0 --> NSH, bit [27]
    // 0b0 - Do not count events in EL2.
    // 0b1 - Count events in EL2.
    pmxevtyper = bit_clear(pmxevtyper, PMEVTYPER_NSH);

    // PMXEVTYPER_EL0 --> M, bit [26]
    // - If the value of this bit is equal to the value of the 
    //   PMEVTYPER<n>_EL0.P bit, events in Secure EL3 are counted.
    // - Otherwise, events in Secure EL3 are not counted.
    pmxevtyper = bit_clear(pmxevtyper, PMEVTYPER_M);

    // PMXEVTYPER_EL0 --> MT, bit [25]
    // 0b0 - Count events only on controlling PE.
    // 0b1 - Count events from any PE with the same affinity at level 1 and 
    //       above as this PE.
    pmxevtyper = bit_clear(pmxevtyper, PMEVTYPER_MT);

    // PMXEVTYPER_EL0 --> SH, bit [24]
    // - Count events in Secure EL2 if not equal to PMEVTYPER<n>_EL0.NSH.
    // - Otherwise, do not count events in Secure EL2.
    pmxevtyper = bit_clear(pmxevtyper, PMEVTYPER_SH);

    // Set the event type value in the PMXEVTYPER_EL0 register based on the
    // event.
    pmxevtyper = bit_insert(pmxevtyper, event, 0, 10);

    // Write the updated value back to the PMXEVTYPER_EL0 register to set
    // the event type.
    sysreg_pmxevtyper_el0_write(pmxevtyper);
}


/**
 * @brief Clears the overflow status of the specified PMU counter.
 *
 * @param counter The PMU counter for which to clear the overflow status.
 */
static inline void pmu_clear_cntr_ovs(size_t counter) {
    uint64_t pmovsclr = 0;
    
    // PMOVSCLR_EL0 -> Performance Monitors Overflow Flag Status Clear Register
    // Set the bit corresponding to the specified PMU counter in the
    // PMOVSCLR_EL0 register.
    pmovsclr = bit_set(pmovsclr, counter);

    // Write the value of pmovsclr to the PMOVSCLR_EL0 register to clear the
    // overflow status of the PMU counter.
    sysreg_pmovsclr_el0_write(pmovsclr); 
}


#endif /* __ARCH_PMU_H__ */