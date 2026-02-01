#include "judge_client_report_helpers.h"

#include <filesystem>
#include <iostream>

#include "library/judge_lib.h"

using namespace std;

std::string build_test_run_output(int ACflg, double &usedtime, double timeLimit, int solution_id,
                                  const char *work_dir, bool debug_enabled) {
    string error_message;
    if (ACflg == TIME_LIMIT_EXCEEDED) {
        usedtime = timeLimit * 1000;
        error_message = "Time Limit Exceeded.Kill Process.\n";
    } else if (ACflg == RUNTIME_ERROR) {
        if (debug_enabled)
            printf("add RE info of %d..... \n", solution_id);
        error_message = "Runtime Error. Kill Process.\n";
    } else if (ACflg == MEMORY_LIMIT_EXCEEDED) {
        error_message = "Memory Limit Exceeded.Kill Process.\n";
    }
    string test_run_out;
    if (ACflg == ACCEPT) {
        test_run_out = getRuntimeInfoContents((std::filesystem::path(work_dir) / "user.out").string().c_str());
    } else {
        test_run_out = error_message;
    }
    if (test_run_out.length() > FOUR * ONE_KILOBYTE) {
        auto omit = to_string(test_run_out.length() - FOUR * ONE_KILOBYTE);
        test_run_out = test_run_out.substr(0, FOUR * ONE_KILOBYTE);
        test_run_out += "\n......Omit " + omit + " characters.";
    }
    if (debug_enabled) {
        cout << "test_run_out:" << endl << test_run_out << endl;
    }
    if (usedtime == timeLimit * 1000) {
        test_run_out += "\n娴嬭瘯杩愯涓彂鐢熻繍琛岃秴鏃讹紝绋嬪簭琚己鍒跺仠姝?;";
    }
    return test_run_out;
}

void send_test_run_bundle(int solution_id, double usedtime, int topmemory, const string &test_run_out,
                          const ResultSender &sender) {
    bundle.clear();
    bundle.setSolutionID(solution_id);
    bundle.setResult(TEST_RUN);
    bundle.setFinished(FINISHED);
    bundle.setUsedTime(usedtime);
    bundle.setMemoryUse(topmemory / ONE_KILOBYTE);
    bundle.setPassPoint(ZERO_PASSPOINT);
    bundle.setPassRate(ZERO_PASSRATE);
    bundle.setTestRunResult(test_run_out);
    sender(bundle.toJSONString());
}
