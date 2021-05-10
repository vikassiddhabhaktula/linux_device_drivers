#include <linux/module.h>	/* 	Needed by all modules	*/
#include <linux/kernel.h>
#include <linux/printk.h>
#include <linux/interrupt.h>	/*	IRQ routines		*/
#include <asm/io.h>		/*	To use asm		*/
#include <linux/kobject.h>	/*	To use kernel objs	*/
#include <linux/sysfs.h>	/*	routines to add sysfs nodes	*/
#include <linux/device.h>

#ifdef VSDBG
        #define CALL(expr)                              (expr)
        #define PLL(long_long_num)                      pr_emerg("VSDBG: %lld\n", long_long_num);
        #define PI(int_num)                             pr_emerg("VSDBG: %d\n", int_num);
        #define PS(string)                              pr_emerg("VSDBG: %s\n", string);
#else
        #define CALL(expr)
        #define PLL(long_long_num)
        #define PI(int_num)
        #define PS(string)
#endif

/*	Works only for intel x86 arch	*/
#define IRQ_NUM1	11
#define IRQ_NUM2	13
#define IRQ_NUM3	10
#define START_VECTOR_ADDR	(0x20 + 0x10)
#define IRQ_VECTOR_ADDR(irq)	(START_VECTOR_ADDR + irq)

/*======================================================================================================
 *					CREATE KERNEL OBJECTS
 *======================================================================================================
 */

/*
 *	Create kernel objects to fire the IRQs from user space.
 */
static ssize_t raise_irq11(struct kobject *kobj, struct kobj_attribute *attr, char *buf) {
	/*
	 *	IRQ_VECTOR_ADDR(IRQ_NUM) to be passed to the asm.
	 *	This only works for intel x86 arch.
	 */
	asm("int $0x3B");
	return 0;
}

static ssize_t raise_irq13(struct kobject *kobj, struct kobj_attribute *attr, char *buf) {
	asm("int $0x3D");
	return 0;
}

static ssize_t raise_irq16(struct kobject *kobj, struct kobj_attribute *attr, char *buf) {
	asm("int $0x3A");
	return 0;
}

static struct kobject *kobj;
static struct kobj_attribute softirq_attr = __ATTR(raise_softirq, S_IRUGO, raise_irq11, NULL);
static struct kobj_attribute tasklet_attr = __ATTR(raise_tasklet, S_IRUGO, raise_irq13, NULL);
static struct kobj_attribute wq_attr = __ATTR(queue_into_workqueue, S_IRUGO, raise_irq16, NULL);

static struct attribute *attrs[] = {
	&softirq_attr.attr,
	&tasklet_attr.attr,
	&wq_attr.attr,
	NULL,
};

static struct attribute_group attr_gp = {
	.attrs = attrs,
};

/*======================================================================================================
 *					IRQ Routines
 *					-	Top half irq handlers
 *						-	handler1, handler2, handler3
 *					-	Bottom half handlers, one each for:
 *						-	softirq
 *						-	tasklet
 *						-	work queue
 *======================================================================================================
 */

/*
 *	Handler to trigger a softirq
 */
static irqreturn_t handler1(int irq, void *dev) {
	unsigned long flags;
	local_irq_save(flags);
	CALL(pr_emerg("VSDBG: In the IRQ %d handler\n", irq));
	local_irq_restore(flags);
	return IRQ_HANDLED;
}

/*
 *	Handler to trigger a tasklet
 */
static irqreturn_t handler2(int irq, void *dev) {
	unsigned long flags;
	local_irq_save(flags);
	CALL(pr_emerg("VSDBG: In the IRQ %d handler\n", irq));
	local_irq_restore(flags);
	return IRQ_HANDLED;
}

/*
 *	Handler to schedule work
 */
static irqreturn_t handler3(int irq, void *dev) {
	unsigned long flags;
	local_irq_save(flags);
	CALL(pr_emerg("VSDBG: In the IRQ %d handler\n", irq));
	local_irq_restore(flags);
	return IRQ_HANDLED;
}

static int __init init_irq_module(void)
{
	int status;
	PS("=============================================================================");
	PS("-----------------------------------------------------------");
	PS("Assign IRQ");
	if (request_irq(IRQ_NUM1, handler1, IRQF_SHARED, "Indirect softirq handler", (void *)handler1)) {
		pr_emerg("VSDBG: IRQ %d registration unsuccessful\n", IRQ_NUM1);
		return -EIO;
	}
	if (request_irq(IRQ_NUM2, handler2, IRQF_SHARED, "Indirect tasklet handler", (void *)handler2)) {
		pr_emerg("VSDBG: IRQ %d registration unsuccessful\n", IRQ_NUM2);
		return -EIO;
	}
	if (request_irq(IRQ_NUM3, handler3, IRQF_SHARED, "Indirect wq handler", (void *)handler3)) {
		pr_emerg("VSDBG: IRQ %d registration unsuccessful\n", IRQ_NUM3);
		return -EIO;
	}

	kobj = kobject_create_and_add("study_bottom_halves", kernel_kobj);
	if (NULL == kobj) {
		pr_emerg("ERR: Couldn't create kernel obj\n");
		return -EIO;
	}
	status = sysfs_create_group(kobj, &attr_gp);
	if (status) {
		pr_emerg("ERR: Couldn't create sysfs group\n");
		return status;
	}
	return 0;
}

static void __exit exit_irq_module(void)
{
	PS("-----------------------------------------------------------");
	PS("Free IRQs");
	free_irq(IRQ_NUM1, (void *)handler1);
	free_irq(IRQ_NUM2, (void *)handler2);
	free_irq(IRQ_NUM3, (void *)handler3);
	sysfs_remove_group(kobj, &attr_gp);
	kobject_del(kobj);
}

module_init(init_irq_module);
module_exit(exit_irq_module);

MODULE_LICENSE("GPL");
