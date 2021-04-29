#include <linux/module.h>	/* Needed by all modules */
#include <linux/kernel.h>	/* Needed for KERN_INFO */
#include <linux/printk.h>

static int hello_world(void)
{
	pr_emerg("Hello World\n");
	/* 
	 * A non 0 return means init_module failed; module can't be loaded. 
	 */
	return 0;
}

static void bye_world(void)
{
	pr_emerg("Bye World!\n");
}

module_init(hello_world);
module_exit(bye_world);
