//
// Created by Ryan on 2018-12-18.
//

#ifndef JUDGE_CLIENT_BUNDLE_H
#define JUDGE_CLIENT_BUNDLE_H

#include <string>
#include <algorithm>
#include <unordered_map>

#include "Pack.h"
#include "BaseBundle.h"

using std::unordered_map;
using std::string;
using std::to_string;


class Bundle: public BaseBundle{
public:
    Bundle();

    ~Bundle() = default;

    Bundle& setSolutionId(int);

    bool setResult(int);

    bool setFinished(int);

    bool setUsedTime(double);

    bool setMemoryUse(int);

    bool setPassPoint(int);

    bool setPassRate(double);

    bool setJudger(char *);

    bool setJudger(string &);

    bool setTestRunResult(string &);

    bool setCompileInfo(string &);

    bool setSim(int);

    bool setSimSource(int);

    bool setTotalPoint(int);

    bool setRuntimeInfo(string &);

    string toJSONString();

    operator string();

private:
    void init();
};


#endif //JUDGE_CLIENT_BUNDLE_H
