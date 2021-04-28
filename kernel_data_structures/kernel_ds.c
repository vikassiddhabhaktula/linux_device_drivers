#include <linux/module.h>	/* Needed by all modules */
#include <linux/kernel.h>
#include <linux/printk.h>
#include <linux/list.h>		/* Linked list routines 	*/
#include <linux/slab.h>		/*	kzalloc			*/
#include <linux/kfifo.h>	/*	Queue routines		*/

#include "kernel_ds.h"

#ifdef VSDBG
	#define CALL(expr)				(expr)
	#define PLL(long_long_num)			pr_emerg("VSDBG: %lld\n", long_long_num);
	#define PS(string)				pr_emerg("VSDBG: %s\n", string);
#else
	#define CALL(expr)
	#define PLL(long_long_num)
	#define PS(string)
#endif

/*==================================================================================================================
 *					LINKED LISTS
 *==================================================================================================================
 */

/*
 *	Linked list example:
 *	-	Make a list of employee records
 *		-	id
 *		-	name
 *		-	salary
 *	-	Add a list node
 */

#define MAX_EMPS 5
static LIST_HEAD(emp_rcrd_head);

struct emp_record {
	char name[100];
	unsigned int id;
	unsigned long long sal;
	struct list_head list;
};

static void init_records(void) {
	char *names[MAX_EMPS] = {"bob", "carl", "james", "elaine", "kath"};
	unsigned int ids[MAX_EMPS] = {10, 23, 45, 36, 67};
	unsigned long long sals[MAX_EMPS] = {100000, 230000, 450000, 360000, 670000};
	unsigned int i;
	
	struct emp_record *rec = NULL;
	for (i=0; i<MAX_EMPS; i++) {
		rec = kzalloc(sizeof(struct emp_record), GFP_KERNEL);
		strcpy(rec->name, names[i]);
		rec->id = ids[i];
		rec->sal = sals[i];
		INIT_LIST_HEAD(&(rec->list));
		list_add(&(rec->list), &emp_rcrd_head);
	}
	
}

static void print_records(void) {
	struct emp_record *rec = NULL;
	list_for_each_entry(rec, &emp_rcrd_head, list) {
		CALL(pr_emerg("VSDBG: %s %d %lld\n", rec->name, rec->id, rec->sal));
	}
}

static void delete_records(void) {
	struct emp_record *rec = NULL;
	if (list_empty(&emp_rcrd_head))	return;
	list_for_each_entry(rec, &emp_rcrd_head, list) {
		list_del(&(rec->list));
		kfree(rec);
	}
}

/*==================================================================================================================
 *					STACK: to be implemented. (Plan is to use LL in reverse)
 *==================================================================================================================
 */

/*==================================================================================================================
 *					QUEUEs
 *==================================================================================================================
 */

static struct kfifo q;

static void init_q(void) {
	int ret;
	unsigned int i;
	unsigned int ids[MAX_EMPS] = {10, 23, 45, 36, 67};
	ret = kfifo_alloc(&q, MAX_EMPS * sizeof(unsigned int), GFP_KERNEL);
	if (ret) {
		pr_emerg("VSDBG: No memory for Q\n");
		return;
	}

	for (i=0; i<MAX_EMPS; i++) {
		ret = kfifo_in(&q, &ids[i], sizeof(ids[i]));
		if (sizeof(ids[i]) != ret)	{
			pr_emerg("VSDBG: No space left to enQ\n");
			return;
		}
	}
}

static void print_q(void) {
	int ret;
	unsigned int i, val;
	if (kfifo_is_empty(&q))	{
		pr_emerg("VSDBG: Q empty\n");
		return;
	}
	for (i=0; i<MAX_EMPS; i++) {
		ret = kfifo_out(&q, &val, sizeof(val));
		if (sizeof(val) != ret)	{
			pr_emerg("VSDBG: Cannot deQ\n");
			return;
		}
		PLL(val);
	}
}

static void delete_q(void) {
	kfifo_free(&q);
}


static int init_kernel_ds(void)
{
	PS("============================================================================");
	PS("-------------------------------------------------------")
	PS("init: Linked lists in kernel");
	PS("-------------------------------------------------------")
	init_records();
	print_records();
	PS("-------------------------------------------------------")
	PS("init: Queues in kernel");
	PS("-------------------------------------------------------")
	init_q();
	print_q();
	return 0;
}

static void exit_kernel_ds(void)
{
	PS("-------------------------------------------------------")
	PS("exit: Linked lists in kernel");
	PS("-------------------------------------------------------")
	/*	fix me: Not working delete_records	*/
	//delete_records();
	PS("-------------------------------------------------------")
	PS("exit: Queues in kernel");
	PS("-------------------------------------------------------")
	delete_q();
}

module_init(init_kernel_ds);
module_exit(exit_kernel_ds);
