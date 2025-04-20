#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/version.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <linux/uaccess.h>
#include "immortal_proc.h"

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Your Name");
MODULE_DESCRIPTION("Immortal Process - Prevent process termination");
MODULE_VERSION("0.1");

static int __init immortal_init(void) {
    int ret;
    
    // Initialize PID list
    ret = immortal_pid_list_init();
    if (ret != 0)
        return ret;
    
    // Setup /proc interface
    ret = immortal_proc_init();
    if (ret != 0) {
        immortal_pid_list_exit();
        return ret;
    }
    
    // Hook system calls
    ret = immortal_hooks_init();
    if (ret != 0) {
        immortal_proc_exit();
        immortal_pid_list_exit();
        return ret;
    }
    
    printk(KERN_INFO "Immortal Process module loaded successfully\n");
    return 0;
}

static void __exit immortal_exit(void) {
    // Unhook system calls
    immortal_hooks_exit();
    
    // Remove /proc interface
    immortal_proc_exit();
    
    // Free PID list
    immortal_pid_list_exit();
    
    printk(KERN_INFO "Immortal Process module unloaded\n");
}

module_init(immortal_init);
module_exit(immortal_exit);
