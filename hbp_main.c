#include "include/hbp.h"
#include "include/ioctl.h"
#include <linux/version.h>

static dev_t devno;
static struct cdev hbp_cdev;
static struct class *hbp_class;

int g_mode = 0, g_fov_on = 0, g_border_on = 0, g_skip_on = 0, g_damage_on = 0, g_maxhp_on = 0;
struct task_struct *target_task = NULL;
unsigned long base_addr = 0;

extern long hbp_ioctl(struct file *, unsigned int, unsigned long);

static struct file_operations fops = {
    .owner = THIS_MODULE,
    .unlocked_ioctl = hbp_ioctl,
};

static int __init hbp_init(void) {
    if (alloc_chrdev_region(&devno, 0, 1, "hbp") < 0) return -EBUSY;
    cdev_init(&hbp_cdev, &fops);
    if (cdev_add(&hbp_cdev, devno, 1) < 0) {
        unregister_chrdev_region(devno, 1);
        return -ENOMEM;
    }

#if LINUX_VERSION_CODE >= KERNEL_VERSION(6, 4, 0)
    hbp_class = class_create("hbp"); 
#else
    hbp_class = class_create(THIS_MODULE, "hbp");
#endif

    if (IS_ERR(hbp_class)) {
        cdev_del(&hbp_cdev);
        unregister_chrdev_region(devno, 1);
        return PTR_ERR(hbp_class);
    }

    device_create(hbp_class, NULL, devno, NULL, "hbp");
    pr_info("[HBP] GKI 6.1/6.6 Compatible Loaded\n");
    return 0;
}

static void __exit hbp_exit(void) {
    uninstall_breakpoints();
    device_destroy(hbp_class, devno);
    class_destroy(hbp_class);
    cdev_del(&hbp_cdev);
    unregister_chrdev_region(devno, 1);
}

module_init(hbp_init);
module_exit(hbp_exit);
MODULE_LICENSE("GPL");
