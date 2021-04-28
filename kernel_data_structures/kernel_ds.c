#include <linux/module.h>	/* 	Needed by all modules 	*/
#include <linux/kernel.h>
#include <linux/printk.h>
#include <linux/list.h>		/* 	Linked list routines 	*/
#include <linux/slab.h>		/*	kzalloc			*/
#include <linux/kfifo.h>	/*	Queue routines		*/
#include <linux/hashtable.h>	/*	hash table macros	*/
#include <linux/types.h>	/*	hash node and hash list	*/
#include <linux/rbtree.h>	/*	red black trees		*/

#include "kernel_ds.h"

#ifdef VSDBG
	#define CALL(expr)				(expr)
	#define PLL(long_long_num)			pr_emerg("VSDBG: %lld\n", long_long_num);
	#define PI(int_num)				pr_emerg("VSDBG: %d\n", int_num);
	#define PS(string)				pr_emerg("VSDBG: %s\n", string);
#else
	#define CALL(expr)
	#define PLL(long_long_num)
	#define PI(int_num)
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
/*	Linked List head is declared and initailized this way	*/
static LIST_HEAD(emp_rcrd_head);

struct emp_record {
	/*
	 *	name, emp id, salary
	 */
	char name[100];
	unsigned int id;
	unsigned long long sal;
	/*	Add a list node to create a linked list	*/
	struct list_head list;
	/*	Add a hash node to create a hash table	*/
	struct hlist_node node;
	/*	Add a rb tree node to form a rb tree	*/
	struct rb_node tree_node;
};

static void init_records(void) {
	/*	Add dummy names, ids, and salaries, add more random data later	*/
	char *names[MAX_EMPS] = {"bob", "carl", "james", "elaine", "kath"};
	unsigned int ids[MAX_EMPS] = {10, 23, 45, 36, 67};
	unsigned long long sals[MAX_EMPS] = {100000, 230000, 450000, 360000, 670000};
	unsigned int i;
	
	/*	Create employee records	*/
	struct emp_record *rec = NULL;
	for (i=0; i<MAX_EMPS; i++) {
		/*	Dynamically allocate memory for each record	*/
		rec = kzalloc(sizeof(struct emp_record), GFP_KERNEL);
		strcpy(rec->name, names[i]);
		rec->id = ids[i];
		rec->sal = sals[i];
		/*	Initialize the list variable & add it to the chain	*/
		INIT_LIST_HEAD(&(rec->list));
		list_add(&(rec->list), &emp_rcrd_head);
	}
}

static void print_records(void) {
	struct emp_record *rec = NULL;
	/*	list_for_each_entry will pick the record inorder starting from head	*/
	list_for_each_entry(rec, &emp_rcrd_head, list) {
		CALL(pr_emerg("VSDBG: %s %d %lld\n", rec->name, rec->id, rec->sal));
	}
}

/*
 *	Fix me: Buggy as of now. Revisit later.
 */
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

/*
 *	Create a FIFO of employee ids and just push them into it.
 *	-	Then deQ each id and print it
 */

/*	Queue in kernel is decalared as follows	*/
static struct kfifo q;

static void init_q(void) {
	int ret;
	unsigned int i;
	unsigned int ids[MAX_EMPS] = {10, 23, 45, 36, 67};
	/*	routine to allocate and initialize the Q	*/
	ret = kfifo_alloc(&q, MAX_EMPS * sizeof(unsigned int), GFP_KERNEL);
	if (ret) {
		pr_emerg("VSDBG: No memory for Q\n");
		return;
	}

	/*	EnQ employee ids	*/
	for (i=0; i<MAX_EMPS; i++) {
		/*	kfifo_in --> same as write system call. Returns number of bytes written	*/
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
	/*	kfifo_is_empty returns non zero if it is empty	*/
	if (kfifo_is_empty(&q))	{
		pr_emerg("VSDBG: Q empty\n");
		return;
	}
	/*	DeQ employee ids	*/
	for (i=0; i<MAX_EMPS; i++) {
		/*	return value same as read system call --> number of bytes read successfully	*/
		ret = kfifo_out(&q, &val, sizeof(val));
		if (sizeof(val) != ret)	{
			pr_emerg("VSDBG: Cannot deQ\n");
			return;
		}
		pr_emerg("VSDBG: DeQVal:%d\n", val);
	}
}

/*
 *	Free the Q once done with it.
 */
static void delete_q(void) {
	kfifo_free(&q);
}

/*==================================================================================================================
 *					MAPS: to be implemented. (idr based)
 *==================================================================================================================
 */

/*==================================================================================================================
 *					HASH TABLE
 *==================================================================================================================
 */

/*
 *	Hash table with the following specifics
 *	-	16 buckets (4 bits)
 *	-	hash function - remainder of Emp ID when divided by 16.
 *	-	Right now not taking care of large key values. Hence sticking to int.
 */

/*
 *	Buckets is key for hash table --> experiment more later.
 *	-	num of buckets is in powers of 2.
 *	-	experiment on hash function.
 */
#define NUM_BITS_FOR_HASH_BUCKETS	4
/*	Fix this: Library wrapper function not working	*/
//hash_init(hash_tbl);
/*
 *	Hash table is decalared as below.
 *	Explore hashtable.h to understand how this macro
 *	expands to a hash list per bucket.
 *
 *	Each hash list is a linked list of data (values) with the same key
 *	from the hash function.
 */
DECLARE_HASHTABLE(hash_tbl, NUM_BITS_FOR_HASH_BUCKETS);

/*	Toy hash function.	*/
static void hash_fn(int val, int *key) {
	*key = val % (1 << (NUM_BITS_FOR_HASH_BUCKETS));
}

static void init_hash_tbl(void) {
	int key;
	struct emp_record *rec = NULL;
	/*	WAR: since hash_init lib call was not working	*/
	__hash_init(hash_tbl, (ARRAY_SIZE(hash_tbl)));
	/*	Take each record from the list and also add it to hash table	*/
	list_for_each_entry(rec, &emp_rcrd_head, list) {
		/*	Find the key	*/
		hash_fn(rec->id, &key);
		/*
		 *	Add it to the hash table. fn takes in hash table,
		 *	hash node from the struct and the key found
		 */
		hash_add(hash_tbl, &(rec->node), key);
	}
}

/*
 *	Printing the whole hash table.
 *	-	Usually not in practice. Just an example.
 */
static void print_hash_tbl(void) {
	int bkt;
	struct emp_record *rec = NULL;
	/*
	 *	hash_for_each
	 *	-	hash table
	 *	-	bkt is a loop variable
	 *	-	rec is struct variable
	 *	-	node from the struct
	 *	Internally uses container_of.
	 */
	hash_for_each(hash_tbl, bkt, rec, node) {
		CALL(pr_emerg("VSDBG: %s %d %lld\n", rec->name, rec->id, rec->sal));
	}
}

static void look_up_hash_tbl(int val) {
	int key;
	struct emp_record *rec = NULL;
	/*	First find the key from the hash_fn	*/
	hash_fn(val, &key);
	/*
	 *	hash_for_each_possible
	 *	-	hash table
	 *	-	rec to fetch the struct of interest
	 *	-	node in the struct
	 *	-	key based list will be iterated.
	 */
	hash_for_each_possible(hash_tbl, rec, node, key) {
		if (rec->id == val)	{
			pr_emerg("VSDBG: ID%d emp name found:%s\n", val, rec->name);
			return;
		}
	}
	pr_emerg("VSDBG: ID%d emp name not found!\n", val);
}

/*==================================================================================================================
 *					BINARY TREES
 *==================================================================================================================
 */

/*
 *	Red black trees
 *	-	Implement inserting data
 *	-	Search for the data
 */

/*	rb trees have the following as a standard to initialize the root	*/
static struct rb_root root = RB_ROOT;

/*
 *	function to insert data into the rb tree
 *	No lib function available, has to be written based on the need
 */
static void ins_rb(struct rb_root *root, struct emp_record *ip_rec) {
	struct rb_node **node = &(root->rb_node);
	struct rb_node *parent = NULL;
	struct emp_record *rec;
	while (NULL != *node) {
		/*
		 *	fetch the rec struct pointer nbased on the location of tree_node in it
		 *
		 *	container_of
		 *	-	variable node
		 *	-	struct whose pointer needs tobe fetched
		 *	-	name of the node in the struct
		 */
		rec = container_of(*node, struct emp_record, tree_node);
		/*	Identify the parent to link later to the child	*/
		parent = *node;
		/*	move left / right based on the value	*/
		/*	Scope to make this generic function	*/
		if (ip_rec->id < rec->id)	{
			node = &((*node)->rb_left);
		}
		else if (ip_rec->id > rec->id) {
			node = &((*node)->rb_right);
		}
		else
			return;
	}
	/*	link parent to the child	*/
	rb_link_node(&(ip_rec->tree_node), parent, node);
	/*	I don't exactly know how this is done but this chooses the color of the node	*/
	rb_insert_color(&(ip_rec->tree_node), root);
}

/*
 *	function to search for a particular key
 *	No lib function available.
 */
static void search_rb_tree(struct rb_root *root, int emp_id) {
	struct rb_node *node = root->rb_node;
	struct emp_record *rec = NULL;
	/*	move either left/right until NULL is reached	*/
	while (NULL != node) {
		rec = container_of(node, struct emp_record, tree_node);
		if (emp_id < rec->id) {
			node = node->rb_left;
		}
		else if (emp_id > rec->id) {
			node = node->rb_right;
		}
		else {
			pr_emerg("VSDBG: emp_id:%d name is:%s\n", rec->id, rec->name);
			return;
		}
	}
	pr_emerg("VSDBG: emp_id:%d not found\n", emp_id);
}

/*
 *	Function to create rb tree
 */
static void create_rb_tree(void) {
	struct emp_record *rec = NULL;
	list_for_each_entry(rec, &emp_rcrd_head, list) {
		ins_rb(&root, rec);
	}
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
	PS("-------------------------------------------------------")
	PS("init: Hash table in kernel");
	PS("-------------------------------------------------------")
	init_hash_tbl();
	print_hash_tbl();
	look_up_hash_tbl(67);
	look_up_hash_tbl(68);
	look_up_hash_tbl(23);
	PS("-------------------------------------------------------")
	PS("init: Red Black trees in kernel");
	PS("-------------------------------------------------------")
	create_rb_tree();
	search_rb_tree(&root, 45);
	search_rb_tree(&root, 10);
	search_rb_tree(&root, 36);
	search_rb_tree(&root, 2);
	return 0;
}

/*
 *	Fix me: Revisit exit function for exiting the right way.
 */
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
