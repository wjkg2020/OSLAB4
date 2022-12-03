#include "defs.h"
#include "mm.h"
#include "proc.h"

#include "printk.h"
#include "rand.h"

extern void __dummy();

struct task_struct* idle;           // idle process
struct task_struct* current;        // 指向当前运行线程的 `task_struct`
struct task_struct* task[NR_TASKS]; // 线程数组，所有的线程都保存在此

void task_init() {
    idle = (struct task_struct*)kalloc();   // 1. 调用 kalloc() 为 idle 分配一个物理页
    idle->state = TASK_RUNNING;             // 2. 设置 state 为 TASK_RUNNING;
    idle->counter = 0;
    idle->priority = 0;                     // 3. 由于 idle 不参与调度 可以将其 counter / priority 设置为 0
    idle->pid = 0;                          // 4. 设置 idle 的 pid 为 0
    current = idle;
    task[0] = idle;                         // 5. 将 current 和 task[0] 指向 idle
    
    for(int i = 1; i < NR_TASKS; i ++) {    // 1. 参考 idle 的设置, 为 task[1] ~ task[NR_TASKS - 1] 进行初始化
    	task[i] = (struct task_struct*)kalloc();
        task[i]->state = TASK_RUNNING;
        task[i]->counter = 0;
        task[i]->priority = rand();
        task[i]->pid = i;                   // 2. 其中每个线程的 state 为 TASK_RUNNING, counter 为 0, priority 使用 rand() 来设置, pid 为该线程在线程数组中的下标
    	task[i]->thread.ra = (uint64)(&__dummy);
        task[i]->thread.sp = (uint64)(task[i]) + PGSIZE;
                // 3. 为 task[1] ~ task[NR_TASKS - 1] 设置 `thread_struct` 中的 `ra` 和 `sp`,
                // 4. 其中 `ra` 设置为 __dummy （见 4.3.2）的地址， `sp` 设置为 该线程申请的物理页的高地址
    }

    printk("...proc_init done!\n");
}

void do_timer(void) {
    if(current == idle)     // 1. 如果当前线程是idle线程，直接进行调度
        schedule();
    else                    // 2. 如果当前线程不是idle，对当前线程的运行剩余时间减1；若剩余时间仍然大于0，则直接返回，否则进行调度
    {
    	current->counter --;
    	if(current->counter > 0)
    	    return;
    	else
    	    schedule();
    }
}

void schedule(void) {
    int next;
    
    while(1) {
    	for(next = 1; next < NR_TASKS && (task[next]->counter == 0 || task[next]->state != TASK_RUNNING); next ++);
    	if(next == NR_TASKS) {
    	    printk("\n");
    	    for(int i = 1; i < NR_TASKS; i ++) {
    	    	task[i]->counter = rand();
    	    	#ifdef SJF
    	        printk("SET [PID = %d COUNTER = %d]\n", task[i]->pid, task[i]->counter);
    	        #endif
    	        #ifdef PRIORITY
    	        printk("SET [PID = %d PRIORITY = %d COUNTER = %d]\n", task[i]->pid, task[i]->priority, task[i]->counter);
    	        #endif
    	    }
    	}
    	else
    	    break;
    }
    #ifdef SJF
    for(int i = next + 1; i < NR_TASKS; i ++) {
        if(task[i]->counter < task[next]->counter && task[i]->counter != 0 && task[i]->state == TASK_RUNNING)
            next = i;
    }
    #endif
    #ifdef PRIORITY
    for(int i = next + 1; i < NR_TASKS; i ++) {
        if(task[i]->priority > task[next]->priority && task[i]->counter != 0 && task[i]->state == TASK_RUNNING)
            next = i;
    }
    #endif
    switch_to(task[next]);
}

extern void __switch_to(struct task_struct* prev, struct task_struct* next);

void switch_to(struct task_struct* next) {
    struct task_struct* prev = current;
    if(current != next) {
    	#ifdef SJF
    	printk("\nswitch to [PID = %d COUNTER = %d]\n", next->pid, next->counter);
    	#endif
    	#ifdef PRIORITY
    	printk("\nswitch to [PID = %d PRIORITY = %d COUNTER = %d]\n", next->pid, next->priority, next->counter);
    	#endif
    	current = next;
    	__switch_to(prev, next);
    }
}

void dummy() {
    uint64 MOD = 1000000007;
    uint64 auto_inc_local_var = 0;
    int last_counter = -1;
    int last_pid = 0;
    while(1) {
        if (last_counter == -1 || current->counter != last_counter || current->pid != last_pid) {
            last_counter = current->counter;
            last_pid = current->pid;
            auto_inc_local_var = (auto_inc_local_var + 1) % MOD;
            //printk("[PID = %d] is running. auto_inc_local_var = %d\n", current->pid, auto_inc_local_var);
            printk("[PID = %d] is running. thread space begin at = ", current->pid);
            print_llu((uint64)current);
            printk("\n");
        }
    }
}
