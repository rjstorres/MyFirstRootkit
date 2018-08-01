#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/namei.h>
#include <linux/sched.h>
#include <linux/proc_fs.h>
#include <linux/version.h>
#include <linux/delay.h>

static int __init main_start(void)
{
    printk(KERN_INFO "Loading hello module...\n");
    printk(KERN_INFO "Hello world\n");
    return 0;
}
 
static void __exit main_end(void)
{
    printk(KERN_INFO "Goodbye Mr.\n");
}

module_init(main_start);
module_exit(main_end);