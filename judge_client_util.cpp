#include "judge_client_util.h"

#include <cstdio>
#include <cstdlib>
#include <cstring>

#include "header/static_var.h"
#include "library/judge_lib.h"

using namespace std;

extern int call_counter[call_array_size];

static bool parse_new_args(int argc, char **argv, int &solution_id, int &runner_id, string &judgerId) {
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

void init_parameters(int argc, char **argv, int &solution_id,
                     int &runner_id, string &judgerId) {
    if (argc < 3) {
        fprintf(stderr, "Usage:%s solution_id runner_id.\n", argv[0]);
        fprintf(stderr, "Multi:%s solution_id runner_id judge_base_path.\n",
                argv[0]);
        fprintf(stderr,
                "Debug:%s solution_id runner_id judge_base_path debug.\n",
                argv[0]);
        exit(1);
    }
    if (parse_new_args(argc, argv, solution_id, runner_id, judgerId)) {
        return;
    }
    DEBUG = (argc > 4);
    if (argc > 5 && !strcmp(argv[5], "DEBUG")) {
        NO_RECORD = 1;
    } else {
        record_call = (argc > 5);
    }
    if (argc > 5) {
        strcpy(LANG_NAME, argv[5]);
    }
    if (argc > 3)
        strcpy(oj_home, argv[3]);
    else
        strcpy(oj_home, "/home/judge");

    solution_id = atoi(argv[1]);
    runner_id = atoi(argv[2]);
    judger_number = runner_id;
}

void print_call_array() {
    printf("int LANG_%sV[256]={", LANG_NAME);
    int i = 0;
    for (i = 0; i < call_array_size; i++) {
        if (call_counter[i]) {
            printf("%d,", i);
        }
    }
    printf("0};\n");

    printf("int LANG_%sC[256]={", LANG_NAME);
    for (i = 0; i < call_array_size; i++) {
        if (call_counter[i]) {
            printf("HOJ_MAX_LIMIT,");
        }
    }
    printf("0};\n");
}
