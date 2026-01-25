#pragma once

#include <stdint.h>

typedef void *scmp_filter_ctx;
typedef unsigned long long scmp_datum_t;

#define SCMP_ACT_KILL 0
#define SCMP_ACT_ALLOW 1
#define SCMP_ACT_TRAP 2

#define SCMP_SYS(x) 0
#define SCMP_A1(...) 0
#define SCMP_CMP_EQ 0

static inline scmp_filter_ctx seccomp_init(uint32_t action) {
    (void)action;
    return (void *)1;
}

static inline int seccomp_rule_add(scmp_filter_ctx ctx, uint32_t action,
                                   int syscall, unsigned arg_cnt, ...) {
    (void)ctx;
    (void)action;
    (void)syscall;
    (void)arg_cnt;
    return 0;
}

static inline int seccomp_load(scmp_filter_ctx ctx) {
    (void)ctx;
    return 0;
}
