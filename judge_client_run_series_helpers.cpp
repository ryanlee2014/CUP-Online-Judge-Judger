#include "judge_client_run_series_helpers.h"

#include <algorithm>
#include <iostream>

namespace judge_run_helpers {

void update_series_result(JudgeSeriesResult &finalResult, const JudgeResult &r) {
    std::cout << "Flag " << r.ACflg << "Memory " << r.topMemory
              << "UsedTime " << r.usedTime << "Num " << r.num << std::endl;
    finalResult.ACflg = std::max(finalResult.ACflg, r.ACflg);
    finalResult.topMemory = std::max(finalResult.topMemory, r.topMemory);
    finalResult.usedTime = std::max(finalResult.usedTime, r.usedTime);
    finalResult.pass_point += r.ACflg == ACCEPT;
}

}  // namespace judge_run_helpers
