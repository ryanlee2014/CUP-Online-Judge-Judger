//
// Created by Haoyuan Li on 2019/12/24.
//

#ifndef JUDGE_CLIENT_FREEBASIC_H
#define JUDGE_CLIENT_FREEBASIC_H


#include "C11.h"

class FreeBasic : public C11 {
public:
    void buildRuntime(const char* work_dir);
};


#endif //JUDGE_CLIENT_FREEBASIC_H
