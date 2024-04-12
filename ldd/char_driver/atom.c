#include<linux/module.h>
#include<linux/fs.h>
#include<linux/cdev.h>
#include<linux/device.h>
#include<linux/uaccess.h>

#define DEV_MEM_SIZE 1024

#undef pr_fmt
#define pr_fmt(fmt) "%s :" fmt,__func__ 

/* ATOM device's memory */
char device_buffer[DEV_MEM_SIZE];

/* This holds the device number */
dev_t device_number;

/* CDEV variable */
struct cdev atom_cdev;

loff_t atom_llseek (struct file *filp, loff_t off, int whence)
{
	loff_t temp;

	pr_info("llseek requested\n");
	pr_info("Current value of the file position = %lld\n",filp->f_pos);

	switch(whence)
	{
		case SEEK_SET:
			if((off > DEV_MEM_SIZE) || (off < 0))
				return -EINVAL;
			filp->f_pos = off;
			break;
		case SEEK_CUR:
			temp = filp->f_pos + off;
			if((temp > DEV_MEM_SIZE) || (temp < 0))
				return -EINVAL;
			filp->f_pos = temp;
			break;
		case SEEK_END:
			temp = DEV_MEM_SIZE + off;
			if((temp > DEV_MEM_SIZE) || (temp < 0))
				return -EINVAL;
			filp->f_pos = temp;
			break;
		default:
			return -EINVAL;
	}

	pr_info("New value of the file position = %lld\n",filp->f_pos);

	return filp->f_pos;
}

ssize_t atom_read (struct file *flip, char __user *buff, size_t count, loff_t *off)
{
	pr_info("Read requested for %zu count\n",count);

	pr_info("Current file position = %lld\n",*off);

	/* Adjust the count */
	if((*off + count) > DEV_MEM_SIZE)
	{
		count = DEV_MEM_SIZE - *off;
	}

	/* Copy to user */

	if(copy_to_user(buff, &device_buffer[*off], count))
	{
		return -EFAULT;
	}

	/* Update the current file postion */
	*off += count;

	pr_info("Number of bytes successfully read = %zu\n", count);
	pr_info("Updated file position = %lld\n",*off);

	/* Return number of bytes */
	return count;
}

ssize_t atom_write (struct file *flip, const char __user *buff, size_t count, loff_t *off)
{
	pr_info("Write requested for %zu count\n",count);

	pr_info("Current file position = %lld\n",*off);

	/* Adjust the count */
	if((*off + count) > DEV_MEM_SIZE)
	{
		count = DEV_MEM_SIZE - *off;
	}

	if(!count)
	{
		pr_err("No space left on the device.\n");
		return -ENOMEM;
	}

	/* Copy from user */

	if(copy_from_user(&device_buffer[*off], buff, count))
	{
		return -EFAULT;
	}

	/* Update the current file postion */
	*off += count;

	pr_info("Number of bytes successfully written = %zu\n", count);
	pr_info("Updated file position = %lld\n",*off);

	/* Return number of bytes */
	return count;
}

int atom_open (struct inode *inode, struct file *flip)
{
	pr_info("Open successful\n");
	return 0;
}

int atom_release (struct inode *inode, struct file *flip)
{
	pr_info("Release successful\n");
	return 0;
}

/* File Operations of the driver */
struct file_operations atom_fops=
{
	.open = atom_open,
	.read = atom_read,
	.write = atom_write,
	.llseek = atom_llseek,
	.release = atom_release,
	.owner = THIS_MODULE
};

struct class *class_atom;

struct device *device_atom;

static int __init atom_init(void)
{
	int ret;

	pr_info("ATOM Initialzed\n");
	/* Dynamically allocate a device number */
	ret = alloc_chrdev_region(&device_number,0,1,"atom_devices");

	pr_info("Device number <major>:<minor> = %d:%d", MAJOR(device_number), MINOR(device_number));


	/* Initialize the cdev structure */
	cdev_init(&atom_cdev, &atom_fops);
	atom_cdev.owner = THIS_MODULE;

	/* Register a device cdev struct with VFS */
	cdev_add(&atom_cdev, device_number, 1);

	/* Create device class under /sys/class/ */
	class_atom = class_create(THIS_MODULE, "atom_class");

	/* Populate the sysfs with device information */
	device_atom = device_create(class_atom, NULL, device_number, NULL, "atom");

	pr_info("Module init successful\n");

	return 0;
}

static void __exit atom_fini(void)
{
	device_destroy(class_atom, device_number);
	class_destroy(class_atom);
	cdev_del(&atom_cdev);
	pr_info("ATOM Finished\n");
}

module_init(atom_init);
module_exit(atom_fini);

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Very First ATOM project");
MODULE_AUTHOR("VARUN PYASI");
