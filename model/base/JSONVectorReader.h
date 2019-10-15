//
// Created by Ryan on 2018/10/18.
//

#ifndef CPP_COMPILERARGSREADER_H
#define CPP_COMPILERARGSREADER_H

#include "../../rapidjson/document.h"
#include "../../rapidjson/prettywriter.h"
#include <vector>
#include <iostream>
#include <string>
using std::cout;
using std::string;
using std::vector;
using namespace rapidjson;
using rapidjson::Document;
using rapidjson::Value;
class JSONVectorReader {
public:
    JSONVectorReader(const string &filePath);

    JSONVectorReader() : isLoad(false) {}

    ~JSONVectorReader() = default;

    bool loadFile(const string &filePath);

    bool loadJSON(const string& jsonPlainText);

    vector<string> GetArray(const string& key);

    string GetString(string key);

    int GetInt(string key);

    double GetDouble(string key);

    float GetFloat(string key);

    void GetCharString(string key, char* dest);

    auto GetObject(string key);

    void documentIsLoaded();

private:
    bool isLoad;
    Document document;
};


#endif //CPP_COMPILERARGSREADER_H
