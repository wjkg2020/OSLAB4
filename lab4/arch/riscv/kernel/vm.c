#include "vm.h"

unsigned long  early_pgtbl[512] __attribute__((__aligned__(0x1000)));

void setup_vm(void) {
    /* 
    1. 由于是进行 1GB 的映射 这里不需要使用多级页表 
    2. 将 va 的 64bit 作为如下划分： | high bit | 9 bit | 30 bit |
        high bit 可以忽略
        中间9 bit 作为 early_pgtbl 的 index
        低 30 bit 作为 页内偏移 这里注意到 30 = 9 + 9 + 12， 即我们只使用根页表， 根页表的每个 entry 都对应 1GB 的区域。 
    3. Page Table Entry 的权限 V | R | W | X 位设置为 1
    */
    memset(early_pgtbl, 0x0, PGSIZE);
    early_pgtbl[PHY_START >> 30 & 0x1ff] = ((PHY_START & 0x00ffffffc0000000) >> 2) | 0xf;
    early_pgtbl[VM_START >> 30 & 0x1ff] = ((PHY_START & 0x00ffffffc0000000) >> 2) | 0xf;
}

/* swapper_pg_dir: kernel pagetable 根目录， 在 setup_vm_final 进行映射。 */
unsigned long  swapper_pg_dir[512] __attribute__((__aligned__(0x1000)));

const char _stext[], _etext[];
const char _srodata[], _erodata[];
const char _sdata[];

void setup_vm_final(void) {
    memset(swapper_pg_dir, 0x0, PGSIZE);

    // No OpenSBI mapping required

    uint64 va = (uint64)(VM_START + OPENSBI_SIZE);
    uint64 pa = (uint64)(PHY_START + OPENSBI_SIZE);

    // mapping kernel text X|-|R|V
    create_mapping(swapper_pg_dir, va, pa, (uint64)_etext - (uint64)_stext, 0xb);

    // mapping kernel rodata -|-|R|V
    va += (uint64)_srodata - (uint64)_stext;
    pa += (uint64)_srodata - (uint64)_stext;
    create_mapping(swapper_pg_dir, va, pa, (uint64)_erodata - (uint64)_srodata, 0x3);

    // mapping other memory -|W|R|V
    va += (uint64)_sdata - (uint64)_srodata;
    pa += (uint64)_sdata - (uint64)_srodata;
    // 所有物理内存共128M
    create_mapping(swapper_pg_dir, va, pa, (uint64)0x8000000 - ((uint64)_sdata - (uint64)_stext), 0x7);

    // set satp with swapper_pg_dir
    uint64 swapper_pg_dir_phy = (uint64)swapper_pg_dir - PA2VA_OFFSET;
    asm volatile (
        "li t0, 0x8000000000000000\n"
        "mv t1, %0\n"
        "srli t1, t1, 12\n"
        "add t0, t0, t1\n"
        "csrw satp, t0"
        :
        : "r"(swapper_pg_dir_phy)
        :
    );

    // flush TLB
    asm volatile("sfence.vma zero, zero");
    return;
}


/* 创建多级页表映射关系 */
void create_mapping(uint64 *pgtbl, uint64 va, uint64 pa, uint64 sz, int perm) {
    /*
    pgtbl 为根页表的基地址
    va, pa 为需要映射的虚拟地址、物理地址
    sz 为映射的大小
    perm 为映射的读写权限

    创建多级页表的时候可以使用 kalloc() 来获取一页作为页表目录
    可以使用 V bit 来判断页表项是否存在
    */
    uint64 va_tmp = va;
    uint64 pa_tmp = pa;

    while(va_tmp < va + sz) {
        uint64 *pgtbl_2 = pgtbl;
        uint64 vpn_2 = (va_tmp >> 30) & 0x1ff;
        uint64 pte_2 = pgtbl_2[vpn_2];
        if(!(pte_2 & 1)) {
            uint64 *new_pg_addr = (uint64 *)kalloc();
            pte_2 = ((((uint64)new_pg_addr - PA2VA_OFFSET) & 0x00fffffffffff000) >> 2) | 1;
            pgtbl[vpn_2] = pte_2;
        }

        uint64 *pgtbl_1 = (uint64 *)((pte_2 & 0x003ffffffffffc00) << 2);
        uint64 vpn_1 = (va_tmp >> 21) & 0x1ff;
        uint64 pte_1 = pgtbl_1[vpn_1];
        if(!(pte_1 & 1)) {
            uint64 *new_pg_addr = (uint64 *)kalloc();
            pte_1 = ((((uint64)new_pg_addr - PA2VA_OFFSET) & 0x00fffffffffff000) >> 2) | 1;
            pgtbl_1[vpn_1] = pte_1;
        }

        uint64 *pgtbl_0 = (uint64 *)((pte_1 & 0x003ffffffffffc00) << 2);
        uint64 vpn_0 = (va_tmp >> 12) & 0x1ff;
        uint64 pte_0 = ((pa_tmp & 0x00fffffffffff000) >> 2) | perm;
        pgtbl_0[vpn_0] = pte_0;

        va_tmp += PGSIZE;
        pa_tmp += PGSIZE;
    }
}