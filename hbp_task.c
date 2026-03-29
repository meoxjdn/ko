#include "include/hbp.h"

struct task_struct *find_task_by_name(const char *name)
{
    struct task_struct *task;

    for_each_process(task) {
        if (strstr(task->comm, name)) {
            return task;
        }
    }
    return NULL;
}

/* 内核版 maps 查找 */
unsigned long get_module_base(struct task_struct *task, const char *module)
{
    struct mm_struct *mm;
    struct vm_area_struct *vma;

    if (!task || !task->mm)
        return 0;

    mm = task->mm;

    for (vma = mm->mmap; vma; vma = vma->vm_next) {
        if (vma->vm_file) {
            char path[256];
            char *tmp = d_path(&vma->vm_file->f_path, path, sizeof(path));
            if (!IS_ERR(tmp)) {
                if (strstr(tmp, module)) {
                    return vma->vm_start;
                }
            }
        }
    }

    return 0;
}