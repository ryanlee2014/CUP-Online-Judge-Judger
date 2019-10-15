#include <utility>
//
// Created by Ryan on 2018-12-18.
//
#include <iostream>

using std::cerr;
using std::endl;
#include "Bundle.h"

bool Bundle::setResult(int state) {
    return setValue("state", Pack(state));
}

bool Bundle::setFinished(int finished) {
    return setValue("finish", Pack(finished));
}

bool Bundle::setUsedTime(double time) {
    return setValue("time", Pack(time));
}

bool Bundle::setMemoryUse(int memory) {
    return setValue("memory", Pack(memory));
}

bool Bundle::setPassPoint(int pass_point) {
    return setValue("pass_point", Pack(pass_point));
}

bool Bundle::setPassRate(double pass_rate) {
    return setValue("pass_rate", Pack(pass_rate));
}

bool Bundle::setJudger(char * str) {
    return setValue("judger", Pack(string(str)));
}

bool Bundle::setJudger(string& str) {
    return setValue("judger", Pack(string(str)));
}


bool Bundle::setTestRunResult(string &test_run_result) {
    return setValue("test_run_result", Pack(checkUTF8Valid(test_run_result)));
}

bool Bundle::setCompileInfo(string &compile_info) {
    return setValue("compile_info", Pack(checkUTF8Valid(compile_info)));
}

bool Bundle::setSim(int sim) {
    return setValue("sim", Pack(sim));
}

bool Bundle::setSimSource(int sim_s_id) {
    return setValue("sim_s_id", Pack(sim_s_id));
}

bool Bundle::setTotalPoint(int total_point) {
    return setValue("total_point", Pack(total_point));
}

string Bundle::toJSONString() {
    return BaseBundle::toJSONString("judger");
}

void Bundle::init() {
    _map["solution_id"] = Pack();
    _map["state"] = Pack();
    _map["finish"] = Pack();
    _map["time"] = Pack();
    _map["memory"] = Pack();
    _map["pass_point"] = Pack();
    _map["pass_rate"] = Pack();
    _map["test_run_result"] = Pack("");
    _map["compile_info"] = Pack("");
    _map["sim"] = Pack();
    _map["sim_s_id"] = Pack();
    _map["total_point"] = Pack(0);
}

Bundle::Bundle() {
    init();
}

Bundle::operator string() {
    return toJSONString();
}

Bundle &Bundle::setSolutionId(int solution_id) {
    BaseBundle::setSolutionId(solution_id);
    return *this;
}
