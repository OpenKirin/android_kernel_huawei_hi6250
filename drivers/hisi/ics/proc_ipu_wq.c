#include <linux/module.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <linux/jiffies.h>
#include <linux/slab.h>
#include <linux/uaccess.h>

#define STRING_LEN 4
char ipu_wq_enable_str[STRING_LEN] = "1";
EXPORT_SYMBOL_GPL(ipu_wq_enable_str);

static int ipu_wq_show(struct seq_file *m, void *v)
{
	seq_printf(m, "%s\n", ipu_wq_enable_str);

	return 0;
}

static ssize_t ipu_wq_write(struct file *file, const char __user *buffer, size_t count, loff_t *f_pos)
{
	unsigned int len;

	if (count >= STRING_LEN)
		len = STRING_LEN - 1;
	else
		len = count;

	if (len && copy_from_user(ipu_wq_enable_str, buffer, len))
		return -EFAULT;
	ipu_wq_enable_str[1] = '\0';

	return count;
}

static int ipu_wq_open(struct inode *inode, struct file *file)
{
	return single_open(file, ipu_wq_show, NULL);
}

static const struct file_operations ipu_wq_fops =
{
	.owner		= THIS_MODULE,
	.open		= ipu_wq_open,
	.read		= seq_read,
	.write 		= ipu_wq_write,
	.llseek		= seq_lseek,
	.release	= single_release,
};

static int __init ipu_wq_init(void)
{
	struct proc_dir_entry* ipu_wq_file;

	ipu_wq_file = proc_create("ipu_wq_enable", 0x0666, NULL, &ipu_wq_fops);
	if (NULL == ipu_wq_file)
	{
	    return -ENOMEM;
	}

	return 0;
}

static void __exit ipu_wq_exit(void)
{
	remove_proc_entry("ipu_wq_enable", NULL);
}

module_init(ipu_wq_init);
module_exit(ipu_wq_exit);

MODULE_AUTHOR("Cambricon Limited");
MODULE_LICENSE("GPL");
