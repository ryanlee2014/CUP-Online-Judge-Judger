#include "judge_client_watch_metrics.h"

#include <algorithm>
#include <cstdio>
#include <sys/ptrace.h>

#include "model/judge/language/Language.h"

namespace judge_watch_helpers {

void update_watch_metrics(WatchMetricsInput &input) {
    if (!input.ruse || !input.topmemory || !input.ACflg || !input.config || !input.languageModel) {
        return;
    }
    int tempmemory = input.languageModel->getMemory(*input.ruse, input.pidApp);
    *input.topmemory = std::max(tempmemory, *input.topmemory);
    if (*input.topmemory <= input.mem_lmt * STD_MB) {
        return;
    }
    if (input.debug_enabled) {
        printf("out of memory %d\n", *input.topmemory);
    }
    if (*input.ACflg == ACCEPT) {
        *input.ACflg = MEMORY_LIMIT_EXCEEDED;
    }
    if (input.config->use_ptrace) {
        ptrace(PTRACE_KILL, input.pidApp, NULL, NULL);
    }
    input.stop = true;
}

void add_watch_usedtime(double &usedtime, const struct rusage &ruse) {
    usedtime += (ruse.ru_utime.tv_sec * 1000 + ruse.ru_utime.tv_usec / 1000);
    usedtime += (ruse.ru_stime.tv_sec * 1000 + ruse.ru_stime.tv_usec / 1000);
}

}  // namespace judge_watch_helpers
