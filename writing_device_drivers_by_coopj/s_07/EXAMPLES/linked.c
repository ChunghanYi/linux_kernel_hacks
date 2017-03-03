/* **************** LDD:2.0 s_07/linked.c **************** */
/*
 * The code herein is: Copyright Jerry Cooperstein, 2012
 *
 * This Copyright is retained for the purpose of protecting free
 * redistribution of source.
 *
 *     URL:    http://www.coopj.com
 *     email:  coop@coopj.com
 *
 * The primary maintainer for this code is Jerry Cooperstein
 * The CONTRIBUTORS file (distributed with this
 * file) lists those known to have contributed to the source.
 *
 * This code is distributed under Version 2 of the GNU General Public
 * License, which you should have received with the source.
 *
 */
LIST_HEAD(my_list);

struct my_entry {
	struct list_head list;
	int intvar;
	char strvar[20];
};

static void mylist_init(void)
{
	struct my_entry *me;
	int j;
	for (j = 0; j < NENTRY; j++) {
		me = kmalloc(sizeof(struct my_entry), GFP_KERNEL);
		me->intvar = j;
		sprintf(me->strvar, "My_%d", j + 1);
		list_add(&me->list, &my_list);
	}
}

static int walk_list(void)
{
	int j = 0;
	struct list_head *l;

	if (list_empty(&my_list))
		return 0;

	list_for_each(l, &my_list) {
		struct my_entry *me = list_entry(l, struct my_entry, list);
		foobar(&me->intvar);
		j++;
	}
	return j;
}
