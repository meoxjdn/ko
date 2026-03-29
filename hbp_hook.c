#include "include/hbp.h"
#include "include/offsets.h"
#include <linux/uaccess.h>
#include <linux/percpu.h>
#include <asm/fpsimd.h>

DEFINE_PER_CPU(int, hbp_active);

void hbp_handler(struct perf_event *bp, struct perf_sample_data *data, struct pt_regs *regs) {
    int *active;
    unsigned long pc = regs->pc;
    active = this_cpu_ptr(&hbp_active);
    if (*active) return;
    *active = 1;

    // 1. FOV (4.3f 广角)
    if (g_fov_on && pc == base_addr + OFF_FOV) {
        struct user_fpsimd_state *fpsimd = &current->thread.uw.fpsimd_state;
        ((__u32 *)&fpsimd->vregs[0])[0] = 0x4089999A; 
        set_tsk_thread_flag(current, TIF_FOREIGN_FPSTATE); 
        regs->pc = regs->regs[30]; [span_4](start_span)// 函数起点 RET 安全[span_4](end_span)
        goto out;
    }

    // 2. 去黑边
    if (g_border_on && pc == base_addr + OFF_BORDER) {
        regs->pc = regs->regs[30]; 
        goto out;
    }

    // 3. 副本秒过 (结算跳转)
    if (g_skip_on && pc == base_addr + OFF_SKIP) {
        regs->pc = base_addr + OFF_SKIP_JMP; 
        goto out;
    }

    // 4. Damage (无敌判断 + 安全读取)
    if (g_damage_on && pc == base_addr + OFF_DAMAGE) {
        uint32_t team_id = 0;
        void __user *ptr = (void __user *)(regs->regs[1] + 0x1c);
        if (regs->regs[1] != 0 && access_ok(ptr, 4)) {
            pagefault_disable();
            if (__copy_from_user_inatomic(&team_id, ptr, 4) == 0 && team_id == 0) {
                regs->regs[0] = 1;      // 玩家受击改伤害
                regs->pc = regs->regs[30]; // 提前返回
            }
            pagefault_enable();
        }
        goto out;
    }

    // 5. 最大血量
    if (g_maxhp_on && pc == base_addr + OFF_MAXHP) {
        regs->regs[0] = 1;
        regs->pc = regs->regs[30];
        goto out;
    }

out:
    *active = 0;
}
