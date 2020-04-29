# include <linux/init.h>
# include <linux/slab.h>
# include <linux/sched.h>
# include <linux/module.h>
# include <linux/kernel.h>
# include <linux/proc_fs.h>
# include <linux/vmalloc.h>
# include <linux/uaccess.h>
# include <asm/uaccess.h>

# define BUFFER_SIZE 128
# define PROC_NAME "pid"

// the current pid 
static long cur_pid;

static ssize_t proc_read(struct file *file, char *buf, size_t count, loff_t *pos);
static ssize_t proc_write(struct file *file, const char __user *usr_buf, size_t count, loff_t *pos);

static struct file_operations proc_ops = {
	.owner = THIS_MODULE,
	.read = proc_read,
	.write = proc_write,
};

static int proc_init(void) {
	/* create /proc files */
	proc_create(PROC_NAME, 0666, NULL, &proc_ops);
	printk(KERN_INFO "/proc/" PROC_NAME " is created!\n");
	return 0;
}

static void proc_exit(void) {
	/* remove /proc files */
	remove_proc_entry(PROC_NAME, NULL);
	printk(KERN_INFO "/proc/" PROC_NAME " is removed!\n");
}

static ssize_t proc_read(struct file *file, char __user *usr_buf, size_t count, loff_t *pos) {
	int rv = 0;
	char buffer[BUFFER_SIZE];
	static int completed = 0;
	struct task_struct *PCB = NULL;

	if (completed) {
		completed = 0;
		return 0;
	}

	PCB = pid_task(find_vpid(cur_pid), PIDTYPE_PID);
	if (PCB == NULL) {
		printk(KERN_INFO "Invalid PID!\n");
		return 0;
	}

	completed = 1;

	rv = sprintf(buffer, "command = [%s] pid = [%ld] state = [%ld]\n", PCB -> comm, cur_pid, PCB -> state);
	
	copy_to_user(usr_buf, buffer, rv);

	return rv;
}

static ssize_t proc_write(struct file *file, const char __user *usr_buf, size_t count, loff_t *pos) {
	char *k_mem;

	// allocate kernel memory
	k_mem = kmalloc(count, GFP_KERNEL);

	// copies user space usr_buf to kernel buffer
	if (copy_from_user(k_mem, usr_buf, count)) {
		printk(KERN_INFO "Error copying from user\n");
		return -1;
	}
	
	// the end of string k_mem
	k_mem[count] = 0;

	// extract the number into the variable pid using kstrtol
	kstrtol(k_mem, 10, &cur_pid);

	// free the memory
	kfree(k_mem);

	return count;
}

module_init(proc_init);
module_exit(proc_exit);

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Pid Module");
MODULE_AUTHOR("Galaxies");
