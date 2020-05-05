# include <linux/gcd.h>
# include <linux/hash.h>
# include <linux/init.h>
# include <linux/kernel.h>
# include <linux/module.h>

int simple_init(void) {
  printk(KERN_INFO "Loading Kernel Module\n");
  printk(KERN_INFO "The golden ratio prime is: %llu\n", GOLDEN_RATIO_PRIME);
  return 0;
}

void simple_exit(void) {
  printk(KERN_INFO "Removing Kernel Module\n");
  printk(KERN_INFO "The greatest common divisor of %d and %d is: %lu\n", 3300, 24, gcd(3300, 24));
}

module_init(simple_init);
module_exit(simple_exit);

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Simple Module");
MODULE_AUTHOR("Galaxies");
