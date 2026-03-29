#include "include/hbp.h"
#include "include/ioctl.h"
#include <linux/version.h>

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

    [span_3](start_span)// 修复 GKI 6.6 (Linux 6.4+) 的参数变更[span_3](end_span)
#if LINUX_VERSION_CODE >= KERNEL_VERSION(6, 4, 0)
    hbp_class = class_create("hbp");
#else
    hbp_class = class_create(THIS_MODULE, "hbp");
#endif

    device_create(hbp_class, NULL, devno, NULL, "hbp");
    printk("[HBP] Driver Loaded\n");
    return 0;
}

static void __exit hbp_exit(void)
{
    uninstall_breakpoints();
    device_destroy(hbp_class, devno);
    class_destroy(hbp_class);
    cdev_del(&hbp_cdev);
    unregister_chrdev_region(devno, 1);
    printk("[HBP] Driver Unloaded\n");
}

module_init(hbp_init);
module_exit(hbp_exit);
MODULE_LICENSE("GPL");
