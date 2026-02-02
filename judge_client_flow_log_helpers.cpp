#include "judge_client_flow_log_helpers.h"

#include <cstdio>

void log_limits(bool debug_enabled, double time_limit, int memory_limit) {
    if (!debug_enabled) {
        return;
    }
    printf("time: %f mem: %d\n", time_limit, memory_limit);
}
