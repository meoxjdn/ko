#include "include/hbp.h"
#include "include/offsets.h"
#include <linux/version.h> // 【极其关键】引入版本控制宏

#define MAX_BP 64

static struct perf_event *bps[MAX_BP];
static int bp_count = 0;

extern void hbp_handler(struct perf_event *, struct perf_sample_data *, struct pt_regs *);

static void install_bp_thread(struct task_struct *task, unsigned long addr)
{
    struct perf_event_attr attr;
    
    // 1. 标准初始化。自动帮你把 attr.pinned 和 attr.sample_period 置 1
    // 避免因为越过高层 API 导致底层断点挂载失败或被内核优化掉
    hw_breakpoint_init(&attr);
    
    // 2. 填充具体的断点参数
    attr.bp_type = HW_BREAKPOINT_X;
    attr.bp_addr = addr;
    attr.bp_len = HW_BREAKPOINT_LEN_4;
    attr.exclude_kernel = 1; // 仅监听目标进程，防误杀内核态

    if (bp_count >= MAX_BP)
        return;

    // 3. 【核心修复】跨版本兼容挂载逻辑
#if LINUX_VERSION_CODE >= KERNEL_VERSION(6, 4, 0)
    // 6.6 GKI 及以上版本：老接口去掉了 task 参数，必须使用底层核心接口
    // -1 表示不在特定的单个 CPU 上锁定，而是锁定在特定的 task 线程上
    bps[bp_count++] = perf_event_create_kernel_counter(&attr, -1, task, hbp_handler, NULL);
#else
    // 6.1 及以前的内核：原样使用老接口
    bps[bp_count++] = register_user_hw_breakpoint(&attr, hbp_handler, NULL, task);
#endif
}

void install_breakpoints(void)
{
    struct task_struct *t;

    bp_count = 0;

    for_each_thread(target_task, t) {
        if (g_fov_on)
            install_bp_thread(t, base_addr + OFF_FOV);
        if (g_border_on)
            install_bp_thread(t, base_addr + OFF_BORDER);
        if (g_skip_on)
            install_bp_thread(t, base_addr + OFF_SKIP);
        if (g_damage_on)
            install_bp_thread(t, base_addr + OFF_DAMAGE);
        if (g_maxhp_on)
            install_bp_thread(t, base_addr + OFF_MAXHP);
    }
}

void uninstall_breakpoints(void)
{
    int i;

    for (i = 0; i < bp_count; i++) {
        if (bps[i]) {
            unregister_hw_breakpoint(bps[i]);
            bps[i] = NULL;
        }
    }
    bp_count = 0;
}
