//
// Created by eleme on 2019/10/15.
//

#ifndef JUDGE_CLIENT_RUNTIMEBUNDLE_H
#define JUDGE_CLIENT_RUNTIMEBUNDLE_H

#include <string>
#include <unordered_map>
#include "Pack.h"
#include "BaseBundle.h"

using std::string;

class RuntimeBundle: public BaseBundle {
public:
    RuntimeBundle();

    ~RuntimeBundle() = default;

    RuntimeBundle& setRuntimeInfo(string&);

    RuntimeBundle& setSolutionId(int);

    string toJSONString();

private:
    void init() override;
};


#endif //JUDGE_CLIENT_RUNTIMEBUNDLE_H
