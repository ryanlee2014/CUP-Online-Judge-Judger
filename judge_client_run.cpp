#include "judge_client_run.h"

#include <algorithm>
#include <iostream>
#include <memory>
#include <string>
#include <sys/types.h>

#include "external/compare/Compare.h"
#include "header/static_var.h"
#include "judge_client_context.h"
#include "judge_client_run_helpers.h"
#include "judge_client_watch.h"
#include "library/judge_lib.h"
#include "model/base/ThreadPool.h"
#include "model/judge/language/Language.h"
#include "model/judge/policy/SpecialJudge.h"

using namespace std;
using namespace judge_run_helpers;

void run_solution_parallel(int &lang, char *work_dir, double &time_lmt, double &usedtime,
                           int &mem_lmt, int fileId, const JudgeConfigSnapshot &config) {
    char input[BUFFER_SIZE], userOutput[BUFFER_SIZE], errorOutput[BUFFER_SIZE];
    build_parallel_io_names(fileId, input, userOutput, errorOutput);
    run_solution_common(lang, work_dir, time_lmt, usedtime, mem_lmt, input, userOutput, errorOutput, 1.0, 1.0,
                        config);
}

void run_solution(int &lang, char *work_dir, double &time_lmt, double &usedtime,
                  int &mem_lmt, const JudgeConfigSnapshot &config) {
    run_solution_common(lang, work_dir, time_lmt, usedtime, mem_lmt, "data.in", "user.out", "error.out", 1.5, 2.0,
                        config);
}

void run_solution(int &lang, char *work_dir, const double &time_lmt, const double &usedtime,
                  const int &mem_lmt, const JudgeConfigSnapshot &config) {
    run_solution_common(lang, work_dir, time_lmt, usedtime, mem_lmt, "data.in", "user.out", "error.out", 1.5, 2.0,
                        config);
}

JudgeResult judge_solution(int &ACflg, double &usedtime, double time_lmt, int isspj,
                           int p_id, char *infile, char *outfile, char *userfile, char *usercode, int &PEflg,
                           int lang, char *work_dir, int &topmemory, int mem_lmt,
                           int solution_id, int num_of_test, string &global_work_dir,
                           const JudgeConfigSnapshot &config) {
    shared_ptr<Language> languageModel(getLanguageModel(lang));
    cout << "Used time" << endl;
    cout << usedtime << endl;
    cout << time_lmt * 1000 * (config.use_max_time ? 1 : num_of_test) << endl;
    cout << "judge solution: infile: " << infile << " outfile: " << outfile << " userfile: " << userfile << endl;
    int comp_res;
    if (!config.all_test_mode) {
        num_of_test = static_cast<int>(1.0);
    }
    if (ACflg == ACCEPT && usedtime > time_lmt * 1000 * (config.use_max_time ? 1 : num_of_test)) {
        cout << "Time Limit Exceeded" << endl;
        usedtime = time_lmt * 1000;
        ACflg = TIME_LIMIT_EXCEEDED;
    }
    if (topmemory > mem_lmt * STD_MB)
        ACflg = MEMORY_LIMIT_EXCEEDED;
    languageModel->fixACFlag(ACflg);
    if (ACflg == ACCEPT) {
        if (isspj) {
            comp_res = SpecialJudge::newInstance().setDebug(DEBUG).run(oj_home, p_id, infile, outfile, userfile,
                                                                       usercode, global_work_dir);
        } else {
            shared_ptr<Compare::Compare> compare(getCompareModel());
            compare->setDebug(DEBUG);
            comp_res = compare->compare(outfile, userfile);
        }
        if (comp_res == WRONG_ANSWER) {
            ACflg = WRONG_ANSWER;
            if (DEBUG)
                printf("fail test %s\n", infile);
        } else if (comp_res == PRESENTATION_ERROR) {
            PEflg = PRESENTATION_ERROR;
        }
        ACflg = comp_res;
    }
    languageModel->fixFlagWithVMIssue(work_dir, ACflg, topmemory, mem_lmt);
    return {ACflg, usedtime, topmemory, 0};
}

JudgeResult runJudgeTask(int runner_id, int language, char *work_dir, pair<string, int> &infilePair, int ACflg,
                         int SPECIAL_JUDGE, int solution_id, double timeLimit, double usedtime, int memoryLimit,
                         int problemId, char *usercode, int num_of_test, string &global_work_dir,
                         const JudgeConfigSnapshot &config, const int *syscall_template) {
    int call_counter_local[call_array_size], PEflg;
    char infile[BUFFER_SIZE], outfile[BUFFER_SIZE], userfile[BUFFER_SIZE];
    int topmemory = 0;
    prepare_run_files_with_id(language, runner_id, infilePair, problemId, work_dir, num_of_test, call_counter_local,
                              infile, outfile, userfile, syscall_template);
    auto pid = fork_and_run_child([&]() {
        run_solution_parallel(language, work_dir, timeLimit, usedtime, memoryLimit, num_of_test, config);
    });
    if (pid == CHILD_PROCESS) {
        return {};
    } else {
        return finish_run_with_id(pid, ACflg, SPECIAL_JUDGE, solution_id, language, topmemory, memoryLimit, usedtime,
                                  timeLimit, problemId, PEflg, work_dir, num_of_test, call_counter_local, infile,
                                  outfile, userfile, usercode, global_work_dir, config);
    }
}

JudgeResult runJudgeTask(int runner_id, int language, char *work_dir, pair<string, int> &infilePair, int ACflg,
                         int SPECIAL_JUDGE, int solution_id, double timeLimit, double usedtime, int memoryLimit,
                         int problemId, char *usercode, int num_of_test, string &global_work_dir,
                         const JudgeConfigSnapshot &config) {
    return runJudgeTask(runner_id, language, work_dir, infilePair, ACflg, SPECIAL_JUDGE,
                        solution_id, timeLimit, usedtime, memoryLimit,
                        problemId, usercode, num_of_test, global_work_dir, config, nullptr);
}

JudgeSeriesResult runParallelJudge(int runner_id, int language, char *work_dir, char *usercode, int timeLimit,
                                   int usedtime, int memoryLimit,
                                   vector<pair<string, int>> &inFileList,
                                   int &ACflg, int SPECIAL_JUDGE, string &global_work_dir,
                                   SubmissionInfo &submissionInfo, const JudgeConfigSnapshot &config,
                                   const int *syscall_template) {
    struct ChunkResult {
        vector<JudgeResult> results;
    };
    int workers = max(int(std::thread::hardware_concurrency()), 1);
    size_t total = inFileList.size();
    if (total == 0) {
        return {3, 0, 0, 0, 0};
    }
    int env_workers = read_env_int("JUDGE_PARALLEL_WORKERS");
    if (env_workers > 0) {
        workers = env_workers;
    }
    if (total > 0 && static_cast<size_t>(workers) > total) {
        workers = static_cast<int>(total);
    }
    ThreadPool pool(workers);
    vector<future<ChunkResult>> result;
    size_t chunk_size = (total + workers - 1) / workers;
    int env_chunk = read_env_int("JUDGE_PARALLEL_CHUNK");
    if (env_chunk > 0) {
        chunk_size = static_cast<size_t>(env_chunk);
    }
    chunk_size = max<size_t>(1, min(chunk_size, total));
    const int base_ac = ACflg;
    for (size_t start = 0; start < total; start += chunk_size) {
        size_t end = min(start + chunk_size, total);
        result.emplace_back(pool.enqueue([&, start, end]() {
            ChunkResult chunk;
            chunk.results.reserve(end - start);
            for (size_t i = start; i < end; ++i) {
                double usedtime_local = 0;
                auto &infilePair = inFileList[i];
                int local_ac = base_ac;
                JudgeResult r = runJudgeTask(runner_id, submissionInfo.getLanguage(), work_dir, infilePair,
                                             local_ac, SPECIAL_JUDGE, submissionInfo.getSolutionId(),
                                             submissionInfo.getTimeLimit(), usedtime_local,
                                             submissionInfo.getMemoryLimit(), submissionInfo.getProblemId(),
                                             usercode, int(i), global_work_dir, config, syscall_template);
                chunk.results.push_back(r);
            }
            return chunk;
        }));
    }
    JudgeSeriesResult finalResult = {3, 0, 0, 0, 0};
    for (auto &res: result) {
        ChunkResult chunk = res.get();
        for (auto &r: chunk.results) {
            update_series_result(finalResult, r);
        }
    }
    return finalResult;
}

JudgeSeriesResult runParallelJudge(int runner_id, int language, char *work_dir, char *usercode, int timeLimit,
                                   int usedtime, int memoryLimit,
                                   vector<pair<string, int>> &inFileList,
                                   int &ACflg, int SPECIAL_JUDGE, string &global_work_dir,
                                   SubmissionInfo &submissionInfo, const JudgeConfigSnapshot &config) {
    return runParallelJudge(runner_id, language, work_dir, usercode, timeLimit, usedtime, memoryLimit, inFileList,
                            ACflg, SPECIAL_JUDGE, global_work_dir, submissionInfo, config, nullptr);
}
