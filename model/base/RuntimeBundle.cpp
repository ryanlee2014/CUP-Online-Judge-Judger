//
// Created by eleme on 2019/10/15.
//

#include "RuntimeBundle.h"

RuntimeBundle &RuntimeBundle::setRuntimeInfo(string &runtime_info) {
    setValue("runtime_info", Pack(runtime_info));
    return *this;
}

RuntimeBundle &RuntimeBundle::setSolutionId(int solution_id) {
    BaseBundle::setSolutionId(solution_id);
    return *this;
}

void RuntimeBundle::init() {
    _map["runtime_info"] = Pack();
    _map["solution_id"] = Pack();
}

string RuntimeBundle::toJSONString() {
    return BaseBundle::toJSONString("runtime_info");
}

