#include "judge_client_util_helpers.h"

#include <cstdlib>
#include <cstring>

#include "header/static_var.h"
#include "library/judge_lib.h"

using namespace std;

namespace judge_util_helpers {

bool parse_new_args(int argc, char **argv, int &solution_id, int &runner_id, string &judgerId) {
    bool error = false;
    for (int i = 1; i < argc; ++i) {
        int argType = detectArgType(argv[i]);
        if (argType == _ERROR) {
            error = true;
            break;
        } else if (argType == _DEBUG) {
            DEBUG = true;
        } else if (argType == _NO_RECORD) {
            NO_RECORD = 1;
        } else if (argType == _RECORD_CALL) {
            record_call = 1;
        } else if (argType == _ADMIN) {
            admin = true;
        } else if (argType == _NO_SIM) {
            no_sim = true;
        } else if (argType == _NO_MYSQL) {
            MYSQL_MODE = false;
        } else if (argType == _STDIN) {
            READ_FROM_STDIN = true;
        } else {
            ++i;
            if (i >= argc) {
                error = true;
                break;
            }
            switch (argType) {
                case _LANG_NAME:
                    strcpy(LANG_NAME, argv[i]);
                    break;
                case _DIR:
                    strcpy(oj_home, argv[i]);
                    break;
                case _SOLUTION_ID:
                    solution_id = atoi(argv[i]);
                    break;
                case _RUNNER_ID:
                    judger_number = runner_id = atoi(argv[i]);
                    break;
                case _JUDGER_ID:
                    judgerId = string(argv[i]);
                    break;
                default:
                    error = true;
                    break;
            }
        }
    }
    if (error) {
        return false;
    }
    return true;
}

}  // namespace judge_util_helpers
