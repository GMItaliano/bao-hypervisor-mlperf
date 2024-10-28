## SPDX-License-Identifier: Apache-2.0
## Copyright (c) Bao Project and Contributors. All rights reserved.

cpu-objs-y+=$(ARCH_PROFILE)/pagetables.o
cpu-objs-y+=$(ARCH_PROFILE)/page_table.o
cpu-objs-y+=$(ARCH_PROFILE)/mem.o
cpu-objs-y+=$(ARCH_PROFILE)/vm.o
cpu-objs-y+=$(ARCH_PROFILE)/vmm.o
cpu-objs-y+=$(ARCH_PROFILE)/psci.o
cpu-objs-y+=$(ARCH_PROFILE)/smmuv2.o
cpu-objs-y+=$(ARCH_PROFILE)/iommu.o
cpu-objs-y+=$(ARCH_PROFILE)/cpu.o
cpu-objs-y+=$(ARCH_PROFILE)/smc.o
cpu-objs-y+=$(ARCH_PROFILE)/timer.o
cpu-objs-y+=$(ARCH_PROFILE)/pmu.o
