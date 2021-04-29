#include <linux/module.h>	/* Needed by all modules */
#include <linux/kernel.h>
#include <linux/printk.h>
#include <linux/interrupt.h>
#include <asm/io.h>

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
#define IRQ_NUM	11
#define START_VECTOR_ADDR	(0x20 + 0x10)
#define IRQ_VECTOR_ADDR(irq)	(START_VECTOR_ADDR + irq)

/*
 *	Handler to just acknowledge that the IRQ has received.
 *	-	Add different bottom halves based on something.
 */

static irqreturn_t handler(int irq, void *dev) {
	CALL(pr_emerg("VSDBG: In the IRQ %d handler\n", irq));
	return IRQ_HANDLED;
}

static int init_irq_module(void)
{
	PS("=============================================================================");
	PS("-----------------------------------------------------------");
	PS("Assign IRQ");
	if (request_irq(IRQ_NUM, handler, IRQF_SHARED, "software IRQ handler", (void *)handler)) {
		pr_emerg("VSDBG: IRQ registration unsuccessful\n");
		return -EIO;
	}
	/*
	 *	IRQ_VECTOR_ADDR(IRQ_NUM) to be passed to the asm.
	 *	This only works for intel x86 arch.
	 */
	asm("int $0x3B");
	return 0;
}

static void exit_irq_module(void)
{
	PS("-----------------------------------------------------------");
	PS("Free IRQ");
	free_irq(IRQ_NUM, (void *)handler);
}

module_init(init_irq_module);
module_exit(exit_irq_module);
