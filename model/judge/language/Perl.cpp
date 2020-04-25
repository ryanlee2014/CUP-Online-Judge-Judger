//
// Created by Haoyuan Li on 2019/12/24.
//

#include "Perl.h"
#include "unistd.h"
#include "util/util.h"
#include <cstring>
#ifdef __i386
#include "syscall/perl/syscall32.h"
#else
#include "syscall/perl/syscall64.h"
#endif

#include <seccomp.h>
#include "common/seccomp_helper.h"

#define SYSCALL_ARRAY LANG_PLV

using std::memset;

void Perl::run(int memory) {
    execv(args[0], args);
}

void Perl::buildRuntime(const char *work_dir) {
    Language::buildRuntime(work_dir);
    execute_cmd("/bin/mkdir %s/usr", work_dir);
    execute_cmd("/bin/mkdir %s/usr/lib", work_dir);
    execute_cmd("/bin/cp /usr/lib/libperl* %s/usr/lib/", work_dir);
    execute_cmd("/bin/cp /usr/bin/perl* %s/", work_dir);
}

double Perl::buildTimeLimit(double timeLimit, double bonus) {
    return BonusLimit::buildBonusTimeLimit(timeLimit, bonus);
}

int Perl::buildMemoryLimit(int memoryLimit, int bonus) {
    return BonusLimit::buildBonusMemoryLimit(memoryLimit, bonus);
}

void Perl::initCallCounter(int *call_counter) {
    memset(call_counter, 0, call_array_size);
    for (int i = 0; i == 0 || SYSCALL_ARRAY[i]; i++)
        call_counter[SYSCALL_ARRAY[i]] = HOJ_MAX_LIMIT;
}

std::string Perl::getFileSuffix() {
    return "pl";
}

int Perl::getMemory(rusage ruse, pid_t pid) {
    const char* mark = "VmPeak:";
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

void Perl::buildSeccompSandbox() {
    scmp_filter_ctx ctx;
    ctx = seccomp_init(SCMP_ACT_KILL);
    for (int i = 0; i == 0 || SYSCALL_ARRAY[i]; i++) {
        if (SYSCALL_ARRAY[i] == 59) {
            continue;
        }
        seccomp_rule_add(ctx, SCMP_ACT_ALLOW, SYSCALL_ARRAY[i], 0);
    }
    seccomp_rule_add(ctx, SCMP_ACT_ALLOW, SCMP_SYS(execve), 1, SCMP_A1(SCMP_CMP_EQ, (scmp_datum_t)(getArgs())));
    if (install_helper()) {
        printf("install helper failed");
        exit(1);
    }
    seccomp_load(ctx);
}

char **Perl::getArgs() {
    return args;
}

extlang createInstanceperl () {
    return new Perl;
}

deslang destroyInstanceperl (Language* language) {
    delete language;
}
