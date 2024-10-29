/**
 * SPDX-License-Identifier: Apache-2.0
 * Copyright (c) Bao Project and Contributors. All rights reserved.
 */

#include <bao.h>
#include <cpu.h>
#include <platform.h>
#include <arch/sysregs.h>

#if (GIC_VERSION == GICV2)
#include <arch/gicv2.h>
#elif (GIC_VERSION == GICV3)
#include <arch/gicv3.h>
#else 
#error "unknown GIV version " GIC_VERSION
#endif

cpuid_t CPU_MASTER __attribute__((section(".data")));

/* Perform architecture dependent cpu cores initializations */
void cpu_arch_init(cpuid_t cpuid, paddr_t load_addr)
{
    cpu()->arch.mpidr = sysreg_mpidr_el1_read();
    cpu_arch_profile_init(cpuid, load_addr);
}

unsigned long cpu_id_to_mpidr(cpuid_t id)
{
    return platform_arch_cpuid_to_mpidr(&platform, id);
}

void cpu_arch_idle()
{
    cpu_arch_profile_idle();

    /*
     * In case the profile implementation does not jump to a predefined wake-up point and just
     * returns from the profile, manually rewind stack and jump to idle wake up. Therefore, we
     * should not return after this point.
     */
    __asm__ volatile("mov sp, %0\n\r"
                     "b cpu_idle_wakeup\n\r" ::"r"(&cpu()->stack[STACK_SIZE]));
    ERROR("returned from idle wake up");
}

void cpu_arch_interrupt_finish(void)
{
    if(cpu()->is_handling_irq)
    {
        gicc_eoir(cpu()->handling_irq_id);
        gicc_dir(cpu()->handling_irq_id);
        cpu()->is_handling_irq = 0;
    }
}


void cpu_arch_standby(void) {
    /**
     * Wait for an interrupt
     */
    __asm__ volatile("wfi");

    __asm__ volatile(
        "mov sp, %0\n\r"
        "b cpu_idle_wakeup\n\r"
        ::"r"(&cpu()->stack[STACK_SIZE]));

    ERROR("returned from idle wake up");

}
