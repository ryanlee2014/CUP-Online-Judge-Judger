//
// Created by Haoyuan Li on 2019/12/24.
//

#include <sys/resource.h>
#include "Schema.h"
#include "unistd.h"
#include "util/util.h"
#include <cstring>
#include <cstdlib>
#ifdef __i386
#include "syscall/schema/syscall32.h"
#else
#include "syscall/schema/syscall64.h"
#endif

#include <seccomp.h>
#include "common/seccomp_helper.h"

#define SYSCALL_ARRAY LANG_SV

using std::memset;
void Schema::run(int memory) {
    // 沙箱里没有 /sys，libgc 并行标记线程会互等卡死；只用单个标记线程，并跳过本地化
    setenv("GC_MARKERS", "1", 1);
    setenv("GUILE_INSTALL_LOCALE", "0", 1);
    execv(args[0], args);
}

void Schema::setProcessLimit() {
    struct rlimit LIM;
    LIM.rlim_cur = LIM.rlim_max = 80;
    setrlimit(RLIMIT_NPROC, &LIM);
}

void Schema::buildRuntime(const char *work_dir) {
    Language::buildRuntime(work_dir);
    execute_cmd("/bin/mkdir -p %s/proc", work_dir);
    execute_cmd("/bin/mount -o bind /proc %s/proc", work_dir);
    execute_cmd("/bin/mkdir -p %s/usr/lib", work_dir);
    execute_cmd("/bin/mkdir -p %s/usr/share", work_dir);
    execute_cmd("/bin/cp -a /usr/share/guile %s/usr/share/", work_dir);
    execute_cmd("/bin/cp /usr/lib/libguile* %s/usr/lib/", work_dir);
    execute_cmd("/bin/cp /usr/lib/libgc* %s/usr/lib/", work_dir);
    execute_cmd("/bin/cp /usr/lib/i386-linux-gnu/libffi* %s/usr/lib/",
                work_dir);
    execute_cmd("/bin/cp /usr/lib/i386-linux-gnu/libunistring* %s/usr/lib/",
                work_dir);
    execute_cmd("/bin/cp /usr/lib/*/libgmp* %s/usr/lib/", work_dir);
    execute_cmd("/bin/cp /usr/lib/libgmp* %s/usr/lib/", work_dir);
    execute_cmd("/bin/cp /usr/lib/*/libltdl* %s/usr/lib/", work_dir);
    execute_cmd("/bin/cp /usr/lib/libltdl* %s/usr/lib/", work_dir);
    execute_cmd("/bin/cp /usr/bin/guile* %s/", work_dir);
    // CentOS 的预编译缓存在 /usr/lib64/guile，没有它 guile 每次都要现场编译标准库
    execute_cmd("/bin/mkdir -p %s/usr/lib64 && /bin/cp -a /usr/lib64/guile %s/usr/lib64/ 2>/dev/null", work_dir, work_dir);
    // guile 启动要做字符集转换（gconv）并读 /dev/urandom
    execute_cmd("/bin/cp -a /usr/lib64/gconv %s/usr/lib64/ 2>/dev/null; [ -d /usr/lib/x86_64-linux-gnu/gconv ] && /bin/mkdir -p %s/usr/lib/x86_64-linux-gnu && /bin/cp -a /usr/lib/x86_64-linux-gnu/gconv %s/usr/lib/x86_64-linux-gnu/; true", work_dir, work_dir, work_dir);
    execute_cmd("/bin/mkdir -p %s/dev", work_dir);
    execute_cmd("/bin/mount -o bind /dev %s/dev", work_dir);
}

double Schema::buildTimeLimit(double timeLimit, double bonus) {
    return BonusLimit::buildBonusTimeLimit(timeLimit, bonus);
}

int Schema::buildMemoryLimit(int memoryLimit, int bonus) {
    return BonusLimit::buildBonusMemoryLimit(memoryLimit, bonus);
}

void Schema::initCallCounter(int *call_counter) {
    memset(call_counter, 0, call_array_size);
    for (int i = 0; i == 0 || SYSCALL_ARRAY[i]; ++i) {
        call_counter[SYSCALL_ARRAY[i]] = HOJ_MAX_LIMIT;
    }
}

std::string Schema::getFileSuffix() {
    return "scm";
}

int Schema::getMemory(rusage ruse, pid_t pid) {
    // guile 会预留很大的虚拟地址空间，VmPeak 远大于实际占用，按物理内存峰值 VmHWM 计
    const char* mark = "VmHWM:";
    const unsigned BUFFER_SIZE = 5 * (1 << 10);
    FILE *pf;
    char fn[BUFFER_SIZE], buf[BUFFER_SIZE];
    int ret = 0;
    sprintf(fn, "/proc/%d/status", pid);
    pf = fopen(fn, "re");
    auto m = static_cast<int>(strlen(mark));
    while (pf && fgets(buf, BUFFER_SIZE - 1, pf)) {

        buf[strlen(buf) - 1] = 0;
        if (strncmp(buf, mark, m) == 0) {
            sscanf(buf + m + 1, "%d", &ret);
        }
    }
    if (pf)
        fclose(pf);
    return ret << 10;
}

void Schema::buildSeccompSandbox() {
    scmp_filter_ctx ctx;
    ctx = seccomp_init(SCMP_ACT_TRAP);
    for (int i = 0; i == 0 || SYSCALL_ARRAY[i]; i++) {
        if (SYSCALL_ARRAY[i] == 59) {
            continue;
        }
        seccomp_rule_add(ctx, SCMP_ACT_ALLOW, SYSCALL_ARRAY[i], 0);
    }
    // execve 只放行启动解释器的那一次（与 Lua/Ruby 一致）
    seccomp_rule_add(ctx, SCMP_ACT_ALLOW, SCMP_SYS(execve), 1, SCMP_A1(SCMP_CMP_EQ, (scmp_datum_t)(getArgs())));
    if (install_helper()) {
        printf("install helper failed");
        exit(1);
    }
    seccomp_load(ctx);
}

char **Schema::getArgs() {
    return args;
}

extlang createInstanceschema () {
    return new Schema;
}

deslang destroyInstanceschema (Language* language) {
    delete language;
}