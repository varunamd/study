#include<linux/module.h>


static int __init atom_init(void)
{
	pr_info("Module init successful\n");

	return 0;
}

static void __exit atom_fini(void)
{
	pr_info("Module deinit\n");
}

module_init(atom_init);
module_exit(atom_fini);

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("First module project");
MODULE_AUTHOR("VARUN PYASI");
