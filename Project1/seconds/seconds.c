# include <linux/init.h>
# include <linux/kernel.h>
# include <linux/module.h>
# include <linux/proc_fs.h>
# include <asm/uaccess.h>
# include <linux/uaccess.h>
# include <linux/jiffies.h>   // jiffies is in this headfile

# define BUFFER_SIZE 128
# define PROC_NAME "seconds"

unsigned long volatile begin_count, seconds;

ssize_t proc_read(struct file *file, char __user *usr_buf, size_t count, loff_t *pos);

static struct file_operations proc_ops = {
  .owner = THIS_MODULE,
  .read = proc_read,
};

int proc_init(void) {
  /* create /proc files */
  proc_create(PROC_NAME, 0666, NULL, &proc_ops);
  printk(KERN_INFO "/proc/" PROC_NAME " is created!\n");
  begin_count = jiffies;
  return 0;
}

void proc_exit(void) {
  /* remove /proc files */
  remove_proc_entry(PROC_NAME, NULL);
  printk(KERN_INFO "/proc/" PROC_NAME " is removed!\n");
}

ssize_t proc_read(struct file *file, char __user *usr_buf, size_t count, loff_t *pos) {
  int rv = 0;
  char buffer[BUFFER_SIZE];
  static int completed = 0;
  
  if (completed) {
    completed = 0;
    return 0;
  }
  
  completed = 1;
  
  seconds = (jiffies - begin_count) / HZ;
  rv = sprintf(buffer, "%lu seconds have elapsed since the kernel module was first loaded\n", seconds);

  copy_to_user(usr_buf, buffer, rv);
  
  return rv;
}

module_init(proc_init);
module_exit(proc_exit);

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Seconds Module");
MODULE_AUTHOR("Galaxies");

