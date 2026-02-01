#include "judge_client_flow_syscall_helpers.h"

#include "header/static_var.h"
#include "manager/syscall/InitManager.h"

void prepare_syscall_template(const JudgeContext &ctx, std::vector<int> &syscall_template,
                              const int *&syscall_template_ptr) {
    if (!ctx.flags.record_call) {
        InitManager::initSyscallLimits(ctx.lang, syscall_template.data(), ctx.flags.record_call, call_array_size);
        syscall_template_ptr = syscall_template.data();
    }
}
