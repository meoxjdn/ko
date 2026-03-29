#include "include/hbp.h"
#include "include/offsets.h"
#include <linux/version.h>
#include <linux/perf_event.h>
#include <linux/hw_breakpoint.h>

#define MAX_BP 128
static struct perf_event *bps[MAX_BP];
static int bp_count = 0;

extern void hbp_handler(struct perf_event *, struct perf_sample_data *, struct pt_regs *);

static void install_bp_thread(struct task_struct *task, unsigned long addr) {
    struct perf_event_attr attr;
    struct perf_event *bp;
    hw_breakpoint_init(&attr);
    attr.bp_type = HW_BREAKPOINT_X;
    attr.bp_addr = addr;
    attr.bp_len = HW_BREAKPOINT_LEN_4;
    attr.exclude_kernel = 1;

    if (bp_count >= MAX_BP) return;

#if LINUX_VERSION_CODE >= KERNEL_VERSION(6, 1, 0)
    // 6.1+ 必须使用底层接口来指定特定 task
    bp = perf_event_create_kernel_counter(&attr, -1, task, hbp_handler, NULL);
#else
    bp = register_user_hw_breakpoint(&attr, hbp_handler, NULL, task);
#endif

    if (!IS_ERR(bp)) bps[bp_count++] = bp;
}

void install_breakpoints(void) {
    struct task_struct *t;
    bp_count = 0;
    if (!target_task) return;
    for_each_thread(target_task, t) {
        if (g_fov_on) install_bp_thread(t, base_addr + OFF_FOV);
        if (g_border_on) install_bp_thread(t, base_addr + OFF_BORDER);
        if (g_skip_on) install_bp_thread(t, base_addr + OFF_SKIP);
        if (g_damage_on) install_bp_thread(t, base_addr + OFF_DAMAGE);
        if (g_maxhp_on) install_bp_thread(t, base_addr + OFF_MAXHP);
    }
}

void uninstall_breakpoints(void) {
    int i;
    for (i = 0; i < bp_count; i++) {
        if (bps[i]) { unregister_hw_breakpoint(bps[i]); bps[i] = NULL; }
    }
    bp_count = 0;
}
