#ifndef JUDGE_CLIENT_FLOW_SYSCALL_HELPERS_H
#define JUDGE_CLIENT_FLOW_SYSCALL_HELPERS_H

#include <vector>

#include "judge_client_context.h"

void prepare_syscall_template(const JudgeContext &ctx, std::vector<int> &syscall_template,
                              const int *&syscall_template_ptr);

#endif
