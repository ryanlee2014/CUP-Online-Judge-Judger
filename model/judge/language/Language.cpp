//
// Created by Haoyuan Li on 2019/12/24.
//

#include <sys/resource.h>
#include "Language.h"
#include "util/util.h"
#include <iostream>
#include <unistd.h>

const int COMPILE_STD_MB = 1 << 20;

void Language::setProcessLimit() {
    struct rlimit LIM;
    LIM.rlim_cur = LIM.rlim_max = 1;
    setrlimit(RLIMIT_NPROC, &LIM);
}

void Language::setCompileProcessLimit() {
    this->setCPULimit();
    this->setAlarm();
    this->setFSizeLimit();
    this->setASLimit();
}

void Language::setCPULimit() {
    struct rlimit LIM;
    LIM.rlim_max = 60;
    LIM.rlim_cur = 60;
    setrlimit(RLIMIT_CPU, &LIM);
}

void Language::setFSizeLimit() {
    struct rlimit LIM;
    LIM.rlim_max = static_cast<rlim_t>(10 * COMPILE_STD_MB);
    LIM.rlim_cur = static_cast<rlim_t>(10 * COMPILE_STD_MB);
    setrlimit(RLIMIT_FSIZE, &LIM);
}

void Language::setASLimit() {
    struct rlimit LIM;
    LIM.rlim_max = static_cast<rlim_t>(COMPILE_STD_MB * 256);
    LIM.rlim_cur = static_cast<rlim_t>(COMPILE_STD_MB * 256);
    setrlimit(RLIMIT_AS, &LIM);
}

void Language::setAlarm() {
    alarm(10);
}

void Language::compile(std::vector<std::string>& _args, const char* java_xms, const char* java_xmx) {
    if (_args.empty()) {
        std::cout << "Nothing to do" << std::endl;
        exit(0);
    }
    int len = static_cast<int>(_args.size());
    const char *args[len + 5];
    for (int i = 0; i < len; ++i) {
        args[i] = _args[i].c_str();
    }
    args[len] = nullptr;
    std::cout << args[0] << std::endl;
    for (int i = 1; i < len; ++i) {
        std::cout << args[i] << " ";
    }
    std::cout << std::endl;
    execvp(args[0], (char *const *) args);
}

void Language::buildRuntime(const char* work_dir) {
    execute_cmd("/bin/mkdir %s/lib", work_dir);
    execute_cmd("/bin/mkdir %s/lib64", work_dir);
    execute_cmd("/bin/mkdir %s/bin", work_dir);
    //  execute_cmd("/bin/cp /lib/* %s/lib/", work_dir);
    //  execute_cmd("/bin/cp -a /lib/i386-linux-gnu %s/lib/", work_dir);
    //  execute_cmd("/bin/cp -a /usr/lib/i386-linux-gnu %s/lib/", work_dir);
    execute_cmd("/bin/cp -a /lib/x86_64-linux-gnu %s/lib/", work_dir);
    execute_cmd("/bin/cp /lib64/* %s/lib64/", work_dir);
}

