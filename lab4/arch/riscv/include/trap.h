#ifndef _TRAP_H
#define _TRAP_H

#define TRAP_MASK (1UL << 63)
#define STI 5

#include "trap.h"
#include "types.h"
#include "clock.h"
#include "printk.h"

void handler_interrupt(uint64 scause, uint64 sepc, uint64 regs);

void handler_exception(uint64 scause, uint64 sepc, uint64 regs);

void trap_handler(uint64 scause, uint64 sepc, uint64 regs);

#endif
