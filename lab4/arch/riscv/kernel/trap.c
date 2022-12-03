#include "trap.h"
#include "types.h"
#include "clock.h"
#include "printk.h"

#include "proc.h"

void handler_interrupt(uint64 scause, uint64 sepc, uint64 regs) {
    switch (scause & ~TRAP_MASK) {
        case STI:
            clock_set_next_event();
            do_timer();
            break;
        default:
            break;
    }
}

void handler_exception(uint64 scause, uint64 sepc, uint64 regs) {
}

void trap_handler(uint64 scause, uint64 sepc, uint64 regs) {
    if (scause & TRAP_MASK)                    // #define TRAP_MASK (1UL << 63)
        handler_interrupt(scause, sepc, regs);
    else
        handler_exception(scause, sepc, regs);
}
