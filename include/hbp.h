#ifndef _HBP_H_
#define _HBP_H_

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/sched/signal.h>
#include <linux/ptrace.h>
#include <linux/uaccess.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/hw_breakpoint.h>
#include <linux/mm.h>
#include <linux/mm_types.h>

extern int g_mode;
extern int g_fov_on;
extern int g_border_on;
extern int g_skip_on;
extern int g_damage_on;
extern int g_maxhp_on;

extern struct task_struct *target_task;
extern unsigned long base_addr;

void install_breakpoints(void);
void uninstall_breakpoints(void);

struct task_struct *find_task_by_name(const char *name);
unsigned long get_module_base(struct task_struct *task, const char *module);

#endif
