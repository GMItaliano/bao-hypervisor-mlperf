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

#define PMU_N_CNTR_GIVEN    0

#define ERROR_NO_MORE_EVENT_COUNTERS    UINT64_MAX

// ALL PMU EVENTS
#define SW_INCR 0x00
#define L1I_CACHE_REFILL 0x01
#define L1I_TLB_REFILL 0x02
#define L1D_CACHE_REFILL 0x03
#define L1D_CACHE 0x04
#define L1D_TLB_REFILL 0x05
#define LD_RETIRED 0x06
#define ST_RETIRED 0x07
#define INST_RETIRED 0x08
#define EXC_TAKEN 0x09
#define EXC_RETURN 0x0A
#define CID_WRITE_RETIRED 0x0B
#define PC_WRITE_RETIRED 0x0C
#define BR_IMMED_RETIRED 0x0D
#define UNALIGNED_LDST_RETIRED 0x0F
#define BR_MIS_PRED 0x10
#define CPU_CYCLES 0x11
#define BR_PRED 0x12
#define MEM_ACCESS 0x13
#define L1I_CACHE 0x14
#define L1D_CACHE_WB 0x15
#define L2D_CACHE 0x16
#define L2D_CACHE_REFILL 0x17
#define L2D_CACHE_WB 0x18
#define BUS_ACCESS 0x19
#define MEMORY_ERROR 0x1A
#define BUS_CYCLES 0x1D
#define CHAIN 0x1E
#define BUS_ACCESS_LD 0x60
#define BUS_ACCESS_ST 0x61
#define BR_INDIRECT_SPEC 0x7A
#define EXC_IRQ 0x86
#define EXC_FIQ 0x87
#define EXT_MEM_REQ 0xC0                    //External memory request
#define NC_EXT_MEM_REQ 0xC1                 //Non-cacheable external memory request
#define LINEFILL_PREF 0xC2                  //Linefill because of prefetch
#define INST_CACH_THRT_OCC 0xC3             //Instruction Cache Throttle occurred.
#define ENT_RD_ALLOC_MD 0xC4                //Entering read allocate mode.
#define RD_ALLOC_MD 0xC5                    //Read allocate mode.
#define PREDEC_ERROR 0xC6                   //Pre-decode error.             
#define DT_WR_PIPELINE_STALL 0xC7           //Data Write operation that stalls the pipeline because the store buffer is full.
#define SCU_SNOOPED_CPU_F_CPU 0xC8          //SCU Snooped data from another CPU for this CPU
#define CD_BR_EXEC 0xC9                     //Conditional branch executed.
#define IND_BR_MISPRED 0xCA                 //Indirect branch mispredicted.
#define IND_BR_MISPRED_ADDR_MISCOMP 0xCB    //Indirect branch mispredicted because of address miscompare.
#define CD_BR_MISPRED 0xCC                  //Conditional branch mispredicted.
#define L1I_MEM_ERROR 0xD0                  //L1 Instruction Cache (data or tag) memory error.
#define L1D_MEM_ERROR 0xD1                  //L1 Data Cache (data, tag or dirty) memory error, correctable or non-correctable.
#define TLB_MEM_ERROR 0xD2                  //TLB memory error.
#define ATTR_PERF_IMPCT_EVENT1 0xE0         //Attributable Performance Impact Event. Counts every cycle that the DPU IQ is empty and that is not because of a recent micro-TLB miss, instruction cache miss or pre-decode error.
#define ATTR_PERF_IMPCT_EVENT2 0xE1         //Counts every cycle the DPU IQ is empty and there is an instruction cache miss being processed.
#define ATTR_PERF_IMPCT_EVENT3 0xE2         //Counts every cycle the DPU IQ is empty and there is an instruction micro-TLB miss being processed.
#define ATTR_PERF_IMPCT_EVENT4 0xE3         //Counts every cycle the DPU IQ is empty and there is a pre-decode error being processed.
#define ATTR_PERF_IMPCT_EVENT5 0xE4         //Counts every cycle there is an interlock that is not because of an Advanced SIMD or Floating-point instruction, and not because of a load/store instruction waiting for data to calculate the address in the AGU. Stall cycles because of a stall in Wr, typically awaiting load data, are excluded.
#define ATTR_PERF_IMPCT_EVENT6 0xE5         //Counts every cycle there is an interlock that is because of a load/store instruction waiting for data to calculate the address in the AGU.
#define ATTR_PERF_IMPCT_EVENT7 0xE6         //Counts every cycle there is an interlock that is because of an Advanced SIMD or Floating-point instruction.
#define ATTR_PERF_IMPCT_EVENT8 0xE7         //Counts every cycle there is a stall in the Wr stage because of a load miss 
#define ATTR_PERF_IMPCT_EVENT9 0xE8         //Counts every cycle there is a stall in the Wr stage because of a store.

static const size_t array_events[] = {
    SW_INCR,                                            // 0
    L1I_CACHE_REFILL,                                   // 1
    L1I_TLB_REFILL,                                     // 2
    L1D_CACHE_REFILL,                                   // 3
    L1D_CACHE,                                          // 4
    L1D_TLB_REFILL,                                     // 5
    LD_RETIRED,                                         // 6
    ST_RETIRED,                                         // 7
    INST_RETIRED,                                       // 8
    EXC_TAKEN,                                          // 9
    EXC_RETURN,                                         // 10
    CID_WRITE_RETIRED,                                  // 11
    PC_WRITE_RETIRED,                                   // 12
    BR_IMMED_RETIRED,                                   // 13
    UNALIGNED_LDST_RETIRED,                             // 14
    BR_MIS_PRED,                                        // 15
    CPU_CYCLES,                                         // 16
    BR_PRED,                                            // 17
    MEM_ACCESS,                                         // 18
    L1I_CACHE,                                          // 19
    L1D_CACHE_WB,                                       // 20
    L2D_CACHE,                                          // 21
    L2D_CACHE_REFILL,                                   // 22
    L2D_CACHE_WB,                                       // 23
    BUS_ACCESS,                                         // 24
    MEMORY_ERROR,                                       // 25
    BUS_CYCLES,                                         // 26
    CHAIN,                                              // 27
    BUS_ACCESS_LD,                                      // 28
    BUS_ACCESS_ST,                                      // 29
    BR_INDIRECT_SPEC,                                   // 30
    EXC_IRQ,                                            // 31
    EXC_FIQ,                                            // 32
    EXT_MEM_REQ,                                        // 33
    NC_EXT_MEM_REQ,                                     // 34
    LINEFILL_PREF,                                      // 35
    INST_CACH_THRT_OCC,                                 // 36
    ENT_RD_ALLOC_MD,                                    // 37
    RD_ALLOC_MD,                                        // 38
    PREDEC_ERROR,                                       // 39
    DT_WR_PIPELINE_STALL,                               // 40
    SCU_SNOOPED_CPU_F_CPU,                              // 41
    CD_BR_EXEC,                                         // 42
    IND_BR_MISPRED,                                     // 43
    IND_BR_MISPRED_ADDR_MISCOMP,                        // 44
    CD_BR_MISPRED,                                      // 45
    L1I_MEM_ERROR,                                      // 46
    L1D_MEM_ERROR,                                      // 47
    TLB_MEM_ERROR,                                      // 48
    ATTR_PERF_IMPCT_EVENT1,                             // 49
    ATTR_PERF_IMPCT_EVENT2,                             // 50
    ATTR_PERF_IMPCT_EVENT3,                             // 51
    ATTR_PERF_IMPCT_EVENT4,                             // 52
    ATTR_PERF_IMPCT_EVENT5,                             // 53
    ATTR_PERF_IMPCT_EVENT6,                             // 54
    ATTR_PERF_IMPCT_EVENT7,                             // 55
    ATTR_PERF_IMPCT_EVENT8,                             // 56
    ATTR_PERF_IMPCT_EVENT9                              // 57
};

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
    pmxevtyper = bit_insert(pmxevtyper, array_events[event], 0, 10);

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