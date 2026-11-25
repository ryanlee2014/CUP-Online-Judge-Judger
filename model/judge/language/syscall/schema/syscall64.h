//
// Created by Haoyuan Li on 2020/3/16.
//

#ifndef JUDGE_LIBRARY_SYSCALL64_H
#define JUDGE_LIBRARY_SYSCALL64_H
#include <sys/syscall.h>
int LANG_SV[256] = {0, 60, 7, 15, 24, 25, 28, 35, 39, 186, 234, 17, 56, 257, 302, 2, 3, 4, 5, 9, 10, 11, 12, 14, 21, 59, 72, 82, 83, 90, 95, 97, 158, 202, 204, 218, 228, 229, 231, 273, 293, 1, 23, 100, 61, 22, 6, 33, 8, 13, 16, 111, 110, 39, 79, SYS_fcntl, SYS_getdents64, SYS_getrlimit, SYS_rt_sigprocmask, SYS_futex, SYS_mmap, SYS_stat, SYS_open, SYS_close, SYS_execve, SYS_access, SYS_brk, SYS_readlink, SYS_munmap, SYS_close, SYS_uname, SYS_clone, SYS_uname, SYS_mprotect, SYS_rt_sigaction, SYS_getrlimit, SYS_fstat, SYS_getuid, SYS_getgid, SYS_geteuid, SYS_getegid, SYS_set_thread_area, SYS_set_tid_address, SYS_set_robust_list, SYS_exit_group, 158, 0};
#endif //JUDGE_LIBRARY_SYSCALL64_H
