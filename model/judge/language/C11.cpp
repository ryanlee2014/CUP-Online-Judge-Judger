//
// Created by Haoyuan Li on 2019/12/24.
//

#include "C11.h"
#include <unistd.h>
#include <cstring>
#ifdef __i386
#include "syscall/c11/syscall32.h"
#else
#include "syscall/c11/syscall64.h"
#endif
#include "seccomp.h"
#include "common/seccomp_helper.h"
using std::memset;

#define SYSCALL_ARRAY LANG_CV

void C11::run(int memory) {
    execv(args[0], args);
}

void C11::buildRuntime(const char *work_dir) {
    // do nothing
}

void C11::initCallCounter(int *call_counter) {
    for (int i = 0; i == 0 || SYSCALL_ARRAY[i]; i++) {
        call_counter[SYSCALL_ARRAY[i]] = HOJ_MAX_LIMIT;
    }
}

void C11::setCompileMount(const char *work_dir) {
    // do nothing
}

std::string C11::getFileSuffix() {
    return "c";
}

void C11::runMemoryLimit(rlimit &LIM) {
    setrlimit(RLIMIT_AS, &LIM);
}

bool C11::enableSim() {
    return true;
}

bool C11::gotErrorWhileRunning(bool error) {
    return error;
}

void C11::buildSeccompSandbox() {
    build_seccomp_filter(SYSCALL_ARRAY, true, getArgs());
}

char **C11::getArgs() {
    return args;
}

extern "C" Language* createInstancec11() {
    return new C11;
}

extern "C" void destroyInstancec11(Language* language) {
    delete language;
}
