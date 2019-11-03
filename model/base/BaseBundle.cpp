//
// Created by eleme on 2019/10/15.
//

#include "BaseBundle.h"
#include <iostream>
#include "../../library/judge_lib.h"
#include "../../library/json.hpp"

using nlohmann::detail::type_error;

using std::cerr;
using std::endl;
bool BaseBundle::setValue(const string &key, Pack val) {
    try {
        this->_map[key] = std::move(val);
    }
    catch (char *e) {
        cerr << "Bundle::setValue failed." << endl;
        return false;
    }
    return true;
}

Pack &BaseBundle::get(const string & key) {
    return this->_map[key];
}

bool BaseBundle::has(const string & key) {
    return this->_map.find(key) != this->_map.end();
}

string BaseBundle::checkUTF8Valid(string &str) {
    string copy_str(str);
    if (utf8_check_is_valid(copy_str)) {
        return trim(copy_str);
    } else {
        return "检测到非法UTF-8输出";
    }
}

string BaseBundle::trim(string &str) {
    if (str.length() > CODE_LENGTH_LIMIT) {
        auto diff_len = str.length() - CODE_LENGTH_LIMIT;
        string copy_str = str.substr(0, CODE_LENGTH_LIMIT);
        copy_str += "\n...\nOmit" + to_string(diff_len) + " character" + (diff_len > 1 ? "s" : "");
        return copy_str;
    } else {
        return str;
    }
}

void BaseBundle::clear() {
    init();
    if (has("wid") && get("wid").isInt()) {
        setValue("wid", Pack(get("wid").setInt(get("wid").getInt() + 1)));
    }
    else {
        setValue("wid", Pack(0));
    }
}

void BaseBundle::setSolutionId(int solution_id) {
    setValue("solution_id", Pack(solution_id));
}

string BaseBundle::toJSONString(const char * str) {
    return toJSONString(string(str));
}

string BaseBundle::toJSONString(string&& type) {
    json JSON;
    JSON["type"] = type;
    for(auto& i:_map) {
        try {
            switch (i.second.getType()) {
                case INT:
                    JSON["value"][i.first] = i.second.getInt();
                    break;
                case DOUBLE:
                    JSON["value"][i.first] = i.second.getFloat();
                    break;
                case STRING:
                    JSON["value"][i.first] = i.second.getString();
                    break;
                default:
                    JSON["value"][i.first] = false;
            }
            JSON.dump();
        }
        catch(type_error& e) {
            JSON["value"][i.first] = e.what();
        }
    }
    string jsonString = JSON.dump();
    cout << "jsonString: " << jsonString << endl;
    return jsonString;
}

string BaseBundle::toJSONString(string & key) {
    return toJSONString(std::forward<string>(key));
}
