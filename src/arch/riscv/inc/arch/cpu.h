/**
 * Bao Hypervisor
 *
 * Copyright (c) Bao Project (www.bao-project.org), 2019-
 *
 * Authors:
 *      Jose Martins <jose.martins@bao-project.org>
 *
 * Bao is free software; you can redistribute it and/or modify it under the
 * terms of the GNU General Public License version 2 as published by the Free
 * Software Foundation, with a special exception exempting guest code from such
 * license. See the COPYING file in the top-level directory for details.
 *
 */

#ifndef __ARCH_CPU_H__
#define __ARCH_CPU_H__

#include <bao.h>

extern cpuid_t CPU_MASTER;

struct cpu_arch {
    unsigned hart_id;
    unsigned plic_cntxt;
};

static inline struct cpu* cpu() {
    return (struct cpu*)BAO_CPU_BASE;
}

#endif /* __ARCH_CPU_H__ */
