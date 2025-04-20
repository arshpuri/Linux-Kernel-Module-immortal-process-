#include <linux/kernel.h>
#include <linux/kallsyms.h>
#include <linux/ftrace.h>
#include <linux/version.h>
#include <linux/uaccess.h>
#include <linux/sched/signal.h>
#include "immortal_proc.h"

// For older kernels
#if LINUX_VERSION_CODE < KERNEL_VERSION(5,7,0)
static unsigned long lookup_name(const char *name) {
    return kallsyms_lookup_name(name);
}
#else
// For newer kernels (5.7+)
#include <linux/kprobes.h>
static unsigned long lookup_name(const char *name) {
    unsigned long addr;
    struct kprobe kp = {
        .symbol_name = name
    };
    
    if (register_kprobe(&kp) < 0)
        return 0;
    
    addr = (unsigned long)kp.addr;
    unregister_kprobe(&kp);
    
    return addr;
}
#endif

// Function pointers
static asmlinkage long (*orig_kill)(const struct pt_regs *);
static unsigned long *sys_call_table;

// Disable write protection
static inline void write_protect_disable(void) {
#ifdef CONFIG_X86
    unsigned long cr0 = read_cr0();
    write_cr0(cr0 & ~0x10000);  // Clear WP bit
#endif
}

// Enable write protection
static inline void write_protect_enable(void) {
#ifdef CONFIG_X86
    unsigned long cr0 = read_cr0();
    write_cr0(cr0 | 0x10000);  // Set WP bit
#endif
}

// Our custom kill syscall handler
static asmlinkage long immortal_kill(const struct pt_regs *regs) {
    pid_t pid = (pid_t)regs->di;
    int sig = (int)regs->si;
    struct task_struct *task;
    
    // Process group or all processes - pass through
    if (pid <= 0)
        return orig_kill(regs);
    
    // Check if this PID is protected
    if (is_protected_pid(pid)) {
        task = pid_task(find_vpid(pid), PIDTYPE_PID);
        if (task) {
            printk(KERN_INFO "IMMORTAL: Blocked %s[%d] signal %d from %s[%d]\n",
                   task->comm, pid, sig,
                   current->comm, current->pid);
            
            // Update statistics
            atomic_inc(&stats.blocked_signals);
            if (sig == SIGKILL)
                atomic_inc(&stats.blocked_sigkills);
            stats.last_block_time = ktime_get();
            
            return -EPERM;  // Operation not permitted
        }
    }
    
    // Pass through to original handler
    return orig_kill(regs);
}

int immortal_hooks_init(void) {
    // Find syscall table
    sys_call_table = (unsigned long *)lookup_name("sys_call_table");
    if (!sys_call_table)
        return -EFAULT;
    
    // Save original syscall handler
    orig_kill = (void *)sys_call_table[__NR_kill];
    
    // Install our hook
    write_protect_disable();
    sys_call_table[__NR_kill] = (unsigned long)immortal_kill;
    write_protect_enable();
    
    printk(KERN_INFO "Syscall hooking installed\n");
    return 0;
}

void immortal_hooks_exit(void) {
    // Restore original syscall handler
    if (sys_call_table && orig_kill) {
        write_protect_disable();
        sys_call_table[__NR_kill] = (unsigned long)orig_kill;
        write_protect_enable();
    }
    
    printk(KERN_INFO "Syscall hooking removed\n");
}
