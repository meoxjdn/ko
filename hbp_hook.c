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

    // 1. 全屏广角 (FPSIMD 修改)
    if (g_fov_on && pc == base_addr + OFF_FOV) {
        // [修复] 移除 .uw，直接调用标准 GKI 结构
        struct user_fpsimd_state *fpsimd = &current->thread.fpsimd_state;
        ((__u32 *)&fpsimd->vregs[0])[0] = 0x4089999A; // 锁定 4.3f
        set_tsk_thread_flag(current, TIF_FOREIGN_FPSTATE); // 强制刷入硬件
        regs->pc = regs->regs[30]; // 函数起点直接 RET
        goto out;
    }

    // 2. 去黑边
    if (g_border_on && pc == base_addr + OFF_BORDER) {
        regs->pc = regs->regs[30]; 
        goto out;
    }

    // 3. 副本秒过 (尾调用跳转)
    if (g_skip_on && pc == base_addr + OFF_SKIP) {
        regs->pc = base_addr + OFF_SKIP_JMP; 
        goto out;
    }

    // 4. Damage (无敌敌我识别)
    if (g_damage_on && pc == base_addr + OFF_DAMAGE) {
        uint32_t team_id = 0;
        void __user *ptr = (void __user *)(regs->regs[1] + 0x1c);
        
        if (regs->regs[1] != 0 && access_ok(ptr, 4)) {
            // [修复] 使用 GKI 推荐的 nofault 读取机制，天生安全，不需要配对 disable
            if (copy_from_user_nofault(&team_id, ptr, 4) == 0 && team_id == 0) {
                regs->regs[0] = 1; 
                regs->pc = regs->regs[30]; // 玩家无敌
            }
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
