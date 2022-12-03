.equ reg_size, 0x8
    
    .section .text.entry
    .align 2
    .globl _trap    
_trap:
    # save 31 register (x0 hardwired zero) and sepc
    csrw sscratch, sp 

    addi sp, sp, -32*reg_size

    sd x1, 0*reg_size(sp)
    sd x3, 2*reg_size(sp)
    sd x4, 3*reg_size(sp)
    sd x5, 4*reg_size(sp)
    sd x6, 5*reg_size(sp)
    sd x7, 6*reg_size(sp)
    sd x8, 7*reg_size(sp)
    sd x9, 8*reg_size(sp)
    sd x10, 9*reg_size(sp)
    sd x11, 10*reg_size(sp)
    sd x12, 11*reg_size(sp)
    sd x13, 12*reg_size(sp)
    sd x14, 13*reg_size(sp)
    sd x15, 14*reg_size(sp)
    sd x16, 15*reg_size(sp)
    sd x17, 16*reg_size(sp)
    sd x18, 17*reg_size(sp)
    sd x19, 18*reg_size(sp)
    sd x20, 19*reg_size(sp)
    sd x21, 20*reg_size(sp)
    sd x22, 21*reg_size(sp)
    sd x23, 22*reg_size(sp)
    sd x24, 23*reg_size(sp)
    sd x25, 24*reg_size(sp)
    sd x26, 25*reg_size(sp)
    sd x27, 26*reg_size(sp)
    sd x28, 27*reg_size(sp)
    sd x29, 28*reg_size(sp)
    sd x30, 29*reg_size(sp)
    sd x31, 30*reg_size(sp)
    
    csrr t0, sepc
    sd t0, 31*reg_size(sp)

# CSRRW reads the old value of the CSR, zero-extends the value to XLEN bits, then writes it to integer register rd.
# The initial value in rs1 is written to the CSR. If rd=x0, then the instruction shall not read the CSR and
# shall not cause any of the side effects that might occur on a CSR read. 
    csrrw t0, sscratch, x0
    sd t0, 1*reg_size(sp)

    csrr a0, scause
    csrr a1, sepc
    mv a2, sp

    call trap_handler

    # load sepc and 31 register (x2(sp) should be loaded last)
    ld t0, 31*reg_size(sp)
    csrw sepc, t0

    ld x1, 0*reg_size(sp)
    ld x3, 2*reg_size(sp)
    ld x4, 3*reg_size(sp)
    ld x5, 4*reg_size(sp)
    ld x6, 5*reg_size(sp)
    ld x7, 6*reg_size(sp)
    ld x8, 7*reg_size(sp)
    ld x9, 8*reg_size(sp)
    ld x10, 9*reg_size(sp)
    ld x11, 10*reg_size(sp)
    ld x12, 11*reg_size(sp)
    ld x13, 12*reg_size(sp)
    ld x14, 13*reg_size(sp)
    ld x15, 14*reg_size(sp)
    ld x16, 15*reg_size(sp)
    ld x17, 16*reg_size(sp)
    ld x18, 17*reg_size(sp)
    ld x19, 18*reg_size(sp)
    ld x20, 19*reg_size(sp)
    ld x21, 20*reg_size(sp)
    ld x22, 21*reg_size(sp)
    ld x23, 22*reg_size(sp)
    ld x24, 23*reg_size(sp)
    ld x25, 24*reg_size(sp)
    ld x26, 25*reg_size(sp)
    ld x27, 26*reg_size(sp)
    ld x28, 27*reg_size(sp)
    ld x29, 28*reg_size(sp)
    ld x30, 29*reg_size(sp)
    ld x31, 30*reg_size(sp)
    
    ld x2, 1*reg_size(sp)
    
    sret
    .globl __dummy
__dummy:
    # 在__dummy中将sepc设置为dummy()的地址, 并使用sret从中断中返回。
    la t0, dummy
    csrw sepc, t0

    sret
    .globl __switch_to
__switch_to:
    # __switch_to接受两个task_struct指针作为参数

    # 保存当前线程的ra、sp和s0 ~ s11，到当前线程的thread_struct中
    sd ra, 40(a0)
    sd sp, 48(a0)
    sd s0, 56(a0)
    sd s1, 64(a0)
    sd s2, 72(a0)
    sd s3, 80(a0)
    sd s4, 88(a0)
    sd s5, 96(a0)
    sd s6, 104(a0)
    sd s7, 112(a0)
    sd s8, 120(a0)
    sd s9, 128(a0)
    sd s10, 136(a0)
    sd s11, 144(a0)

    # 将下一个线程的thread_struct中的相关数据载入到ra、sp和s0 ~ s11中。
    ld ra, 40(a1)
    ld sp, 48(a1)
    ld s0, 56(a1)
    ld s1, 64(a1)
    ld s2, 72(a1)
    ld s3, 80(a1)
    ld s4, 88(a1)
    ld s5, 96(a1)
    ld s6, 104(a1)
    ld s7, 112(a1)
    ld s8, 120(a1)
    ld s9, 128(a1)
    ld s10, 136(a1)
    ld s11, 144(a1)
    
    ret
