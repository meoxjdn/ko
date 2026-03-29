#include "include/hbp.h"
#include "include/offsets.h"
#include <linux/uaccess.h>
#include <linux/percpu.h>
#include <linux/thread_info.h>
#include <asm/fpsimd.h>

// ==========================================
// [核心防御] Per-CPU 递归保护锁
// 防止多线程并发触发断点导致内核栈溢出或死机
// ==========================================
DEFINE_PER_CPU(int, hbp_active);

void hbp_handler(struct perf_event *bp,
                 struct perf_sample_data *data,
                 struct pt_regs *regs)
{
    int *active;
    unsigned long pc = regs->pc;

    // 获取当前 CPU 的锁，如果是重入则直接放行，防止死循环
    active = this_cpu_ptr(&hbp_active);
    if (*active) return;
    *active = 1;

    // ==========================================
    // 1. FOV (广角)
    // ==========================================
    if (g_fov_on && pc == base_addr + OFF_FOV) {
        /*
         * 适配注意：如果是较低版本内核，可能是 &current->thread.fpsimd_state
         * 高版本内核通常带有 .uw 结构
         */
        struct user_fpsimd_state *fpsimd = &current->thread.uw.fpsimd_state;

        // [修复] 128-bit 寄存器脏数据问题
        // 将 vregs[0] 强转为 32位指针，只覆盖最低的 32 bit，防止破坏高位浮点数据
        __u32 *vreg0_32 = (__u32 *)&fpsimd->vregs[0];
        vreg0_32[0] = 0x4089999A; // 4.3f

        // 强制 CPU 在返回用户态时重新从 task_struct 加载浮点寄存器
        set_tsk_thread_flag(current, TIF_FOREIGN_FPSTATE);

        // 你的原始汇编注入是 FMOV S0, W0 然后 RET。这里直接返回即可。
        regs->pc = regs->regs[30];
        goto out;
    }

    // ==========================================
    // 2. 去黑边
    // ==========================================
    if (g_border_on && pc == base_addr + OFF_BORDER) {
        // 你的原始汇编是写了个 0xD65F03C0 (RET)
        regs->pc = regs->regs[30];
        goto out;
    }

    // ==========================================
    // 3. 副本秒过
    // ==========================================
    if (g_skip_on && pc == base_addr + OFF_SKIP) {
        regs->pc = base_addr + OFF_SKIP_JMP;
        goto out;
    }

    // ==========================================
    // 4. 敌我识别无敌 (Damage)
    // Hook 点在函数绝对起点 (AC4B4: STP X23, X22, ...)
    // ==========================================
    if (g_damage_on && pc == base_addr + OFF_DAMAGE) {
        // [判空保护] 对齐汇编的 CBZ X1, +20，防止内核读野指针
        if (regs->regs[1] == 0) {
            goto out;
        }

        uint64_t target_addr = regs->regs[1] + 0x1c;
        uint32_t flag = 0;
        int read_status;

        // 原子态安全读取内存，防止目标内存被 swap out 导致 Kernel Panic
        pagefault_disable();
        read_status = __copy_from_user_inatomic(&flag, (void __user *)target_addr, sizeof(flag));
        pagefault_enable();

        // [对齐汇编 CBNZ W16, +12]
        // 汇编逻辑：为 0 是玩家 (走修改并RET)，非 0 是敌人 (走原指令)
        if (read_status == 0 && flag == 0) {
            // 【玩家受击】
            regs->regs[0] = 1;         // 强制将受到伤害改为 1 (MOV W0, #1)
            regs->pc = regs->regs[30]; // 栈未变动，绝对安全的 RET
            goto out;
        }

        // 【敌人受击】或读取失败
        // 绝招：什么都不改，直接释放锁！
        // 内核会自动步过 (Step-over) 这个断点，执行 STP 压栈，敌人正常掉血
        goto out;
    }

    // ==========================================
    // 5. MAX HP
    // ==========================================
    if (g_maxhp_on && pc == base_addr + OFF_MAXHP) {
        regs->regs[0] = 1;
        regs->pc = regs->regs[30];
        goto out;
    }

out:
    // 释放锁，允许下一次中断响应
    *active = 0;
}
