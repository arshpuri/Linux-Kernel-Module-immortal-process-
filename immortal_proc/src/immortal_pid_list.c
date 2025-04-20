#include <linux/slab.h>
#include <linux/spinlock.h>
#include <linux/list.h>
#include <linux/sched.h>
#include "immortal_proc.h"

// Global variables
static LIST_HEAD(protected_pids);
static DEFINE_SPINLOCK(pids_lock);
struct immortal_stats stats;

int immortal_pid_list_init(void) {
    // Initialize statistics
    atomic_set(&stats.total_protections, 0);
    atomic_set(&stats.blocked_signals, 0);
    atomic_set(&stats.blocked_sigkills, 0);
    stats.last_block_time = ktime_get();
    
    return 0;
}

void immortal_pid_list_exit(void) {
    struct immortal_pid *entry, *tmp;
    
    // Free all entries
    spin_lock(&pids_lock);
    list_for_each_entry_safe(entry, tmp, &protected_pids, list) {
        list_del(&entry->list);
        kfree(entry);
    }
    spin_unlock(&pids_lock);
}

bool is_protected_pid(pid_t pid) {
    struct immortal_pid *entry;
    bool found = false;
    
    rcu_read_lock();
    list_for_each_entry_rcu(entry, &protected_pids, list) {
        if (entry->pid == pid) {
            found = true;
            break;
        }
    }
    rcu_read_unlock();
    
    return found;
}

int add_protected_pid(pid_t pid) {
    struct immortal_pid *entry;
    struct task_struct *task;
    
    // Check if already protected
    if (is_protected_pid(pid))
        return -EEXIST;
    
    // Verify process exists
    task = pid_task(find_vpid(pid), PIDTYPE_PID);
    if (!task)
        return -ESRCH;
    
    // Create new entry
    entry = kmalloc(sizeof(*entry), GFP_KERNEL);
    if (!entry)
        return -ENOMEM;
    
    entry->pid = pid;
    memcpy(entry->comm, task->comm, TASK_COMM_LEN);
    entry->protection_time = ktime_get();
    
    // Add to list with lock
    spin_lock(&pids_lock);
    list_add_rcu(&entry->list, &protected_pids);
    spin_unlock(&pids_lock);
    
    atomic_inc(&stats.total_protections);
    printk(KERN_INFO "Process %s[%d] is now protected\n", entry->comm, pid);
    
    return 0;
}

int remove_protected_pid(pid_t pid) {
    struct immortal_pid *entry, *tmp;
    int ret = -ENOENT;
    
    spin_lock(&pids_lock);
    list_for_each_entry_safe(entry, tmp, &protected_pids, list) {
        if (entry->pid == pid) {
            list_del_rcu(&entry->list);
            spin_unlock(&pids_lock);
            synchronize_rcu();  // Wait for readers
            kfree(entry);
            printk(KERN_INFO "Process %d is no longer protected\n", pid);
            return 0;
        }
    }
    spin_unlock(&pids_lock);
    
    return ret;
}
