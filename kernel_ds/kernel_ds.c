#include <linux/module.h>	/* Needed by all modules */
#include <linux/kernel.h>
#include <linux/printk.h>
#include <linux/list.h>		/* Linked list routines 	*/
#include <linux/slab.h>		/*	kzalloc			*/

#include "kernel_ds.h"

#ifdef VSDBG
	#define CALL(expr)				(expr)
	#define PLL(long_long_num)			pr_emerg("VSDBG: %lld", long_long_num);
	#define PS(string)				pr_emerg("VSDBG: %s", string);
#else
	#define CALL(expr)
	#define PLL(long_long_num)
	#define PS(string)
#endif

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

static int init_kernel_ds(void)
{
	PS("============================================================================\n");
	PS("-------------------------------------------------------\n")
	PS("init: Linked lists in kernel\n");
	init_records();
	print_records();
	/* 
	 * A non 0 return means init_module failed; module can't be loaded. 
	 */
	return 0;
}

static void exit_kernel_ds(void)
{
	PS("-------------------------------------------------------\n")
	PS("exit: Linked lists in kernel\n");
	//delete_records();
}

module_init(init_kernel_ds);
module_exit(exit_kernel_ds);
