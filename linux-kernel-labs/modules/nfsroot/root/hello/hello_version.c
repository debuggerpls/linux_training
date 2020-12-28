// SPDX-License-Identifier: GPL-2.0
#include <linux/init.h>
#include <linux/module.h>
#include <linux/utsname.h>
#include <linux/timekeeping32.h>

static char *who = "Deividas";
module_param(who, charp, 0644);
MODULE_PARM_DESC(who, "Recipient of greeting message");

static unsigned long start_time;

static int __init hello_init(void)
{
	pr_alert("Hello %s. You are currently using Linux %s.\n", who,
		 init_uts_ns.name.release);
	start_time = get_seconds();
	return 0;
}

static void __exit hello_exit(void)
{
	pr_alert("Goodbye %s. Elapsed time: %lus\n", who,
		 get_seconds() - start_time);
}

module_init(hello_init);
module_exit(hello_exit);

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Greeting module");
MODULE_AUTHOR("Deividas Puplauskas");
