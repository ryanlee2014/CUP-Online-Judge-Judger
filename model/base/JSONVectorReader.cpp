//
// Created by Ryan on 2018/10/18.
//

#include "JSONVectorReader.h"
#include "../../rapidjson/document.h"
#include "../../rapidjson/prettywriter.h"
#include <iostream>
#include <fstream>
#define ASSERT_VALID(x) assert(x)
using std::fstream;
using std::cout;
using namespace std;
JSONVectorReader::JSONVectorReader(const string &filePath) {
    loadFile(filePath);
}

bool JSONVectorReader::loadFile(const string &filePath) {
    fstream JSON_FILE(filePath.c_str());
    string s, line;
    while (getline(JSON_FILE, line)) {
        s += line;
    }
    JSON_FILE.close();
    return isLoad = !document.ParseInsitu((char *) s.c_str()).HasParseError();
}

bool JSONVectorReader::loadJSON(const string& jsonPlainText) {
    return !document.ParseInsitu((char *) jsonPlainText.c_str()).HasParseError();
}

vector<string> JSONVectorReader::GetArray(const string& key) {
    documentIsLoaded();
    vector<string> vec;
    auto findMember = document.FindMember(key.c_str());
    if(findMember == document.MemberEnd()) {
        return vec;
    }
    const Value& val = document[key.c_str()];
    ASSERT_VALID(val.IsArray());

    for(Value::ConstValueIterator iter = val.Begin();iter != val.End();++iter) {
        if(iter->IsString()) {
            vec.emplace_back(iter->GetString());
        }
    }
    return vec;
}

string JSONVectorReader::GetString(string key) {
    documentIsLoaded();
    auto findMember = document.FindMember(key.c_str());
    if(findMember == document.MemberEnd()) {
        return string();
    }
    auto& val = document[key.c_str()];
    ASSERT_VALID(val.IsString());
    return string(val.GetString());
}

int JSONVectorReader::GetInt(string key) {
    documentIsLoaded();
    auto findMember = document.FindMember(key.c_str());
    if(findMember == document.MemberEnd()) {
        return -1;
    }
    auto& val = document[key.c_str()];
    ASSERT_VALID(val.IsInt());
    return val.GetInt();
}

void JSONVectorReader::documentIsLoaded() {
    if (!isLoad) {
        throw "No JSON file is loaded";
    }
}

auto JSONVectorReader::GetObject(string key) {
    documentIsLoaded();
    auto findMember = document.FindMember(key.c_str());
    if (findMember == document.MemberEnd()) {
        //return ;
        return Value(kObjectType).GetObject();
    }
    auto& val = document[key.c_str()];
    ASSERT_VALID(val.IsObject());
    return val.GetObject();
}

double JSONVectorReader::GetDouble(string key) {
    documentIsLoaded();
    auto findMember = document.FindMember(key.c_str());
    if (findMember == document.MemberEnd()) {
        return 0;
    }
    auto& val = document[key.c_str()];
    ASSERT_VALID(val.IsDouble());
    return val.GetDouble();
}

float JSONVectorReader::GetFloat(string key) {
    documentIsLoaded();
    auto findMember = document.FindMember(key.c_str());
    if (findMember == document.MemberEnd()) {
        return 0;
    }
    auto& val = document[key.c_str()];
    ASSERT_VALID(val.IsFloat());
    return val.GetFloat();
}

void JSONVectorReader::GetCharString(string key, char *dest) {
    string response = this->GetString(key);
    strcpy(dest, response.c_str());
}


