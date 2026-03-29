#include "include/hbp.h"
#include "include/ioctl.h"

long hbp_ioctl(struct file *f, unsigned int cmd, unsigned long arg)
{
    switch (cmd) {

    case CMD_SET_MODE:
        if (copy_from_user(&g_mode, (void __user *)arg, sizeof(int)))
            return -1;

        if (g_mode == 1) {
            g_fov_on = 1;
            g_border_on = 1;
        } else if (g_mode == 2) {
            g_skip_on = 1;
            g_damage_on = 1;
            g_maxhp_on = 1;
        }
        break;

    case CMD_ENABLE:

        target_task = find_task_by_name("Kihan");
        if (!target_task) {
            printk("[HBP] target not found\n");
            return -1;
        }

        base_addr = get_module_base(target_task, "libil2cpp.so");
        if (!base_addr) {
            printk("[HBP] base not found\n");
            return -1;
        }

        printk("[HBP] base = %lx\n", base_addr);

        install_breakpoints();
        break;

    case CMD_DISABLE:
        uninstall_breakpoints();
        break;
    }

    return 0;
}