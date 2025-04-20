#ifndef _IMMORTAL_PROC_H
#define _IMMORTAL_PROC_H

#include <linux/types.h>
#include <linux/list.h>
#include <linux/spinlock.h>
#include <linux/pid.h>

// Protected PID entry
struct immortal_pid {
    pid_t pid;
    char comm[TASK_COMM_LEN];  // Process name
    ktime_t protection_time;   // When protection was added
    struct list_head list;
};

// Statistics structure
struct immortal_stats {
    atomic_t total_protections;
    atomic_t blocked_signals;
    atomic_t blocked_sigkills;
    ktime_t last_block_time;
};

// Function prototypes
int immortal_pid_list_init(void);
void immortal_pid_list_exit(void);
bool is_protected_pid(pid_t pid);
int add_protected_pid(pid_t pid);
int remove_protected_pid(pid_t pid);

int immortal_proc_init(void);
void immortal_proc_exit(void);

int immortal_hooks_init(void);
void immortal_hooks_exit(void);

// Global variables
extern struct immortal_stats stats;

#endif /* _IMMORTAL_PROC_H */
