#include "judge_client_run.h"

#include <iostream>
#include <memory>

#include "external/compare/Compare.h"
#include "header/static_var.h"
#include "judge_client_context.h"
#include "library/judge_lib.h"
#include "model/judge/language/Language.h"
#include "model/judge/policy/SpecialJudge.h"

using namespace std;

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
