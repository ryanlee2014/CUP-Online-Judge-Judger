//
// Created by eleme on 2019/10/15.
//

#ifndef JUDGE_CLIENT_BASEBUNDLE_H
#define JUDGE_CLIENT_BASEBUNDLE_H

#include <string>
#include <unordered_map>
#include "Pack.h"
using std::unordered_map;
using std::string;

class BaseBundle {
public:
    void clear();
protected:
    void setSolutionId(int);
    unordered_map<string, Pack> _map;
    const unsigned CODE_LENGTH_LIMIT = 1u << 13;
    bool setValue(const string& key, Pack val);
    virtual void init() = 0;
    Pack &get(const string&);
    bool has(const string&);
    string checkUTF8Valid(string &);
    string trim(string &);
    string toJSONString(string&);
    string toJSONString(string&&);
    string toJSONString(const char*);
};


#endif //JUDGE_CLIENT_BASEBUNDLE_H
