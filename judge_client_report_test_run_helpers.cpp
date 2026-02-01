#include "judge_client_report_test_run_helpers.h"

#include <filesystem>
#include <iostream>

#include "header/static_var.h"
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
        test_run_out += "\n濞村鐦潻鎰攽娑擃厼褰傞悽鐔荤箥鐞涘矁绉撮弮璁圭礉缁嬪绨悮顐㈠繁閸掕泛浠犲?;";
    }
    return test_run_out;
}
