.extern start_kernel
    .section .text.init
    .globl _start
    .align 2
_start:
    la sp, boot_stack_top

    call setup_vm
    call relocate

    call mm_init
    
    call setup_vm_final

    call task_init

    # set stvec = _trap
    la t0, _trap
    csrw stvec, t0

    # enable timer interrupt sie.STIE = 1
    csrr t0, sie
    ori t0, t0, 0x20
    csrw sie, t0

    # set first time interrupt
    rdtime t0
    li t1, 10000000
    add a0, t0, t1
    add a6, zero, zero
    add a7, zero, zero
    ecall

    # enable interrupt sstatus.SIE = 1
    csrr t0, sstatus
    ori t0, t0, 0x2
    csrw sstatus, t0

    j start_kernel

relocate:
    # set ra = ra + PA2VA_OFFSET
    # set sp = sp + PA2VA_OFFSET (If you have set the sp before)
    li t0, 0xffffffdf80000000
    add ra, ra, t0
    add sp, sp, t0

    # set satp with early_pgtbl
    li t0, 0x8000000000000000
    la t1, early_pgtbl
    srli t1, t1, 12
    add t0, t0, t1
    csrw satp, t0

    # flush tlb
    sfence.vma zero, zero

    ret

    .section .bss.stack
    .globl boot_stack
boot_stack:
    .space 4096
    .globl boot_stack_top
boot_stack_top:
