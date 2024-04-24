#include<linux/module.h>
#include<linux/fs.h>
#include<linux/cdev.h>
#include<linux/device.h>
#include<linux/uaccess.h>

#define DEV_MEM_SIZE_1 1024
#define DEV_MEM_SIZE_2 512
#define DEV_MEM_SIZE_3 1024
#define DEV_MEM_SIZE_4 512

#define NO_OF_DEVICES 4

#undef pr_fmt
#define pr_fmt(fmt) "%s :" fmt,__func__ 

#define RDONLY     0x01
#define WRONLY     0x10
#define RDWR       0x11


/* ATOM device's memory */
char device_buffer1[DEV_MEM_SIZE_1];
char device_buffer2[DEV_MEM_SIZE_2];
char device_buffer3[DEV_MEM_SIZE_3];
char device_buffer4[DEV_MEM_SIZE_4];

/* Device private data structure */

struct pcdev_private_data
{
    char *buffer;
    unsigned size;
    const char *serial_number;
    int perm;
    struct cdev atom_cdev;
};


/*Driver private data structure */

struct pcdrv_private_data
{
    int total_devices;
    /* this holds the device number */
    dev_t device_number;
    struct class *class_atom;
    struct device *device_atom;
    struct pcdev_private_data pcdev_data[NO_OF_DEVICES];
};

struct pcdrv_private_data pcdrv_data =
{
    .total_devices = NO_OF_DEVICES,
    .pcdev_data = {
        [0] = {
            .buffer = device_buffer1,
            .size = DEV_MEM_SIZE_1,
            .serial_number = "PCDEV1XYZ123",
            .perm = RDONLY
        },
        [1] = {
            .buffer = device_buffer2,
            .size = DEV_MEM_SIZE_2,
            .serial_number = "PCDEV2XYZ123",
            .perm = WRONLY
        },
        [2] = {
            .buffer = device_buffer3,
            .size = DEV_MEM_SIZE_3,
            .serial_number = "PCDEV3XYZ123",
            .perm = RDWR
        },
        [3] = {
            .buffer = device_buffer4,
            .size = DEV_MEM_SIZE_4,
            .serial_number = "PCDEV4XYZ123",
            .perm = RDWR
        }

    }
};

loff_t atom_llseek (struct file *filp, loff_t off, int whence)
{
    struct pcdev_private_data *pcdev_data = (struct pcdev_private_data*)filp->private_data;
    int max_size = pcdev_data->size;

	loff_t temp;

	pr_info("llseek requested\n");
	pr_info("Current value of the file position = %lld\n",filp->f_pos);

	switch(whence)
	{
		case SEEK_SET:
			if((off > max_size) || (off < 0))
				return -EINVAL;
			filp->f_pos = off;
			break;
		case SEEK_CUR:
			temp = filp->f_pos + off;
			if((temp > max_size) || (temp < 0))
				return -EINVAL;
			filp->f_pos = temp;
			break;
		case SEEK_END:
			temp = max_size + off;
			if((temp > max_size) || (temp < 0))
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
    struct pcdev_private_data *pcdev_data = (struct pcdev_private_data*)flip->private_data;
    int max_size = pcdev_data->size;

	pr_info("Read requested for %zu count\n",count);
	pr_info("Current file position = %lld\n",*off);

	/* Adjust the count */
	if((*off + count) > max_size)
	{
		count = max_size - *off;
	}

	/* Copy to user */

	if(copy_to_user(buff, pcdev_data->buffer+(*off), count))
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
    struct pcdev_private_data *pcdev_data = (struct pcdev_private_data*)flip->private_data;
    int max_size = pcdev_data->size;
	
    pr_info("Write requested for %zu count\n",count);
	pr_info("Current file position = %lld\n",*off);

	/* Adjust the count */
	if((*off + count) > max_size)
	{
		count = max_size - *off;
	}

	if(!count)
	{
		pr_err("No space left on the device.\n");
		return -ENOMEM;
	}

	/* Copy from user */

	if(copy_from_user(pcdev_data->buffer+(*off), buff, count))
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

int check_permission(int dev_perm, int acc_mode)
{
    if(dev_perm == RDWR)
        return 0;
    if( (dev_perm == RDONLY) && ((acc_mode & FMODE_READ) && !(acc_mode & FMODE_WRITE)) )
    {
        return 0;
    }

    if( (dev_perm == WRONLY) && ((acc_mode & FMODE_WRITE) && !(acc_mode & FMODE_READ)) )
    {
        return 0;
    }

    return -EPERM;
}

int atom_open (struct inode *inode, struct file *flip)
{
    int ret;
    int minor_n;
    struct pcdev_private_data *pcdev_data;

    minor_n = MINOR(inode->i_rdev);
    pr_info("minor access = %d\n",minor_n);

    /* get device's private data structure */
    pcdev_data = container_of(inode->i_cdev, struct pcdev_private_data, atom_cdev);

    /* To supply device private data to other methods of the driver */
    flip->private_data = pcdev_data;
    /*check permission*/
    ret = check_permission(pcdev_data->perm, flip->f_mode);

    (!ret)?pr_info("Open was successful\n"):pr_info("Open was unsuccessful\n");

	return ret;
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

static int __init atom_init(void)
{
	int ret,i;

	pr_info("ATOM Initialzed\n");

    /* Dynamically allocate a device number */
	ret = alloc_chrdev_region(&pcdrv_data.device_number,0,NO_OF_DEVICES,"atom_devices");
    if(ret < 0){
        pr_err("Allocation chrdev failed\n");
        goto out;
    }

    /* Create device class under /sys/class/ */
    pcdrv_data.class_atom = class_create("atom_class");
    if(IS_ERR(pcdrv_data.class_atom)){
        pr_err("Class creation failed.\n");
        ret = PTR_ERR(pcdrv_data.class_atom);
        goto unreg_chrdev;
    }



    for(i=0; i<NO_OF_DEVICES; i++)
    {
        pr_info("Device number %d: <major>:<minor> = %d:%d", i, MAJOR(pcdrv_data.device_number + i), MINOR(pcdrv_data.device_number + i));
    
    	/* Initialize the cdev structure */
    	cdev_init(&pcdrv_data.pcdev_data[i].atom_cdev, &atom_fops);
    	pcdrv_data.pcdev_data[i].atom_cdev.owner = THIS_MODULE;

	    /* Register a device cdev struct with VFS */
	    ret = cdev_add(&pcdrv_data.pcdev_data[i].atom_cdev, pcdrv_data.device_number + i, 1);
        if(ret < 0)
        {
            pr_err("Cdev add failed\n");
            goto cdev_del;
        }

	    /* Populate the sysfs with device information */
	    pcdrv_data.device_atom = device_create(pcdrv_data.class_atom, NULL, pcdrv_data.device_number + i, NULL, "atom-%d",i);
        if(IS_ERR(pcdrv_data.device_atom)){
            pr_err("Device creation failed.\n");
            ret = PTR_ERR(pcdrv_data.device_atom);
            goto class_del;
        }
    }

	pr_info("Module init successful\n");

	return 0;

cdev_del:
class_del:
    for(;i>=0;i--){
        device_destroy(pcdrv_data.class_atom,pcdrv_data.device_number+i);
        cdev_del(&pcdrv_data.pcdev_data[i].atom_cdev);
    }
    class_destroy(pcdrv_data.class_atom);
unreg_chrdev:
    unregister_chrdev_region(pcdrv_data.device_number,NO_OF_DEVICES);
out: 
    pr_err("Module insertion failed\n");
    return ret;
}

static void __exit atom_fini(void)
{
    int i=0;

    for(i=0;i<NO_OF_DEVICES;i++){
        device_destroy(pcdrv_data.class_atom,pcdrv_data.device_number+i);
        cdev_del(&pcdrv_data.pcdev_data[i].atom_cdev);
    }
    class_destroy(pcdrv_data.class_atom);
    unregister_chrdev_region(pcdrv_data.device_number,NO_OF_DEVICES);
	pr_info("ATOM Finished\n");
}

module_init(atom_init);
module_exit(atom_fini);

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Very First ATOM project");
MODULE_AUTHOR("VARUN PYASI");
