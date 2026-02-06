#ifndef JUDGE_CLIENT_WATCH_METRICS_H
#define JUDGE_CLIENT_WATCH_METRICS_H

#include <memory>
#include <sys/resource.h>
#include <sys/types.h>

#include "judge_client_context.h"

class Language;

namespace judge_watch_helpers {

struct WatchMetricsInput {
    std::shared_ptr<Language> languageModel;
    const struct rusage *ruse = nullptr;
    pid_t pidApp = 0;
    int mem_lmt = 0;
    int *topmemory = nullptr;
    int *ACflg = nullptr;
    bool debug_enabled = false;
    const JudgeConfigSnapshot *config = nullptr;
    bool stop = false;
};

void update_watch_metrics(WatchMetricsInput &input);
void add_watch_usedtime(double &usedtime, const struct rusage &ruse);

}  // namespace judge_watch_helpers

#endif
