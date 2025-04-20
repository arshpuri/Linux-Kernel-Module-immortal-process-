#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <linux/uaccess.h>
#include <linux/slab.h>
#include "immortal_proc.h"

static struct proc_dir_entry *proc_entry;

// Show current protected PIDs
static int immortal_proc_show(struct seq_file *m, void *v) {
    struct immortal_pid *entry;
    
    seq_printf(m, "PID\tCOMM\tPROTECTED_SINCE\n");
    
    rcu_read_lock();
    list_for_each_entry_rcu(entry, &protected_pids, list) {
        seq_printf(m, "%d\t%s\t%lld ns\n", 
                  entry->pid, entry->comm, 
                  ktime_to_ns(entry->protection_time));
    }
    rcu_read_unlock();
    
    // Print statistics
    seq_printf(m, "\nTotal protections: %d\n", 
              atomic_read(&stats.total_protections));
    seq_printf(m, "Blocked signals: %d\n", 
              atomic_read(&stats.blocked_signals));
    seq_printf(m, "Blocked SIGKILL: %d\n", 
              atomic_read(&stats.blocked_sigkills));
    
    return 0;
}

static int immortal_proc_open(struct inode *inode, struct file *file) {
    return single_open(file, immortal_proc_show, NULL);
}

// Handle write to proc entry (add or remove PIDs)
static ssize_t immortal_proc_write(struct file *file, 
                                  const char __user *buffer,
                                  size_t count, loff_t *pos) {
    char *kbuf, *tmp;
    int pid, ret;
    bool add_pid = true;
    
    kbuf = kmalloc(count + 1, GFP_KERNEL);
    if (!kbuf)
        return -ENOMEM;
    
    if (copy_from_user(kbuf, buffer, count)) {
        kfree(kbuf);
        return -EFAULT;
    }
    
    kbuf[count] = '\0';
    tmp = kbuf;
    
    // Check for remove (-) prefix
    if (*tmp == '-') {
        add_pid = false;
        tmp++;
    } else if (*tmp == '+') {
        add_pid = true;
        tmp++;
    }
    
    // Parse PID
    ret = kstrtoint(tmp, 10, &pid);
    if (ret) {
        kfree(kbuf);
        return ret;
    }
    
    if (pid <= 0) {
        kfree(kbuf);
        return -EINVAL;
    }
    
    // Add or remove PID
    if (add_pid)
        ret = add_protected_pid(pid);
    else
        ret = remove_protected_pid(pid);
    
    kfree(kbuf);
    return (ret == 0) ? count : ret;
}

static const struct proc_ops immortal_proc_fops = {
    .proc_open = immortal_proc_open,
    .proc_read = seq_read,
    .proc_write = immortal_proc_write,
    .proc_lseek = seq_lseek,
    .proc_release = single_release,
};

int immortal_proc_init(void) {
    proc_entry = proc_create("immortal_proc", 0600, NULL, &immortal_proc_fops);
    if (!proc_entry)
        return -ENOMEM;
    
    return 0;
}

void immortal_proc_exit(void) {
    if (proc_entry)
        remove_proc_entry("immortal_proc", NULL);
}
