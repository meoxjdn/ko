#include "include/hbp.h"
#include "include/ioctl.h"

static dev_t devno;
static struct cdev hbp_cdev;
static struct class *hbp_class;

int g_mode = 0;
int g_fov_on = 0;
int g_border_on = 0;
int g_skip_on = 0;
int g_damage_on = 0;
int g_maxhp_on = 0;

struct task_struct *target_task = NULL;
unsigned long base_addr = 0;

extern long hbp_ioctl(struct file *, unsigned int, unsigned long);

static struct file_operations fops = {
    .owner = THIS_MODULE,
    .unlocked_ioctl = hbp_ioctl,
};

static int __init hbp_init(void)
{
    alloc_chrdev_region(&devno, 0, 1, "hbp");

    cdev_init(&hbp_cdev, &fops);
    cdev_add(&hbp_cdev, devno, 1);

    hbp_class = class_create(THIS_MODULE, "hbp");
    device_create(hbp_class, NULL, devno, NULL, "hbp");

    printk("[HBP] loaded\n");
    return 0;
}

static void __exit hbp_exit(void)
{
    uninstall_breakpoints();

    device_destroy(hbp_class, devno);
    class_destroy(hbp_class);
    cdev_del(&hbp_cdev);
    unregister_chrdev_region(devno, 1);

    printk("[HBP] unloaded\n");
}

module_init(hbp_init);
module_exit(hbp_exit);
MODULE_LICENSE("GPL");