/**
 * @file config_file.cc

 * @author qc
 * @brief 配置文件
 * @version 0.1
 * @date 2024-05-07
 * 
 * @copyright Copyright (c) 2024
 * 
 */
#include <lars_reactor/config_file.hpp>
// #include "config_file.hpp"

#include <strings.h>

#include <fstream>
#include <iostream>
#include <map>
#include <sstream>

// #include "qc.hpp"
#include <lars_reactor/qc.hpp>

namespace qc {

// 直接获取单例模式,不需要在config_file中单独设置一个单例(饿汉式)
static config_file* config = nullptr;

config_file::~config_file() {
    for (auto it = _map.begin(); it != _map.end(); ++it) {
        delete it->second;
    }
}

/// @brief 获取字符串类型键值对
std::string config_file::GetString(const std::string& section, const std::string& key, const std::string& default_value) {
    auto it = _map.find(section);
    if (it != _map.end()) {
        std::map<std::string, std::string>::const_iterator it1 = it->second->find(key);
        if (it1 != it->second->end()) {
            return it1->second; // return value
        }
    }
    return default_value;
}

float config_file::GetFloat(const std::string& section, const std::string& key, const float& default_value) {
    std::ostringstream convert1;
    convert1 << default_value;
    // 将浮点类型转换为字符串,然后按照字符串业务来处理
    std::string default_value_str = convert1.str();
    // 获取字符串类型键值对 -> value
    std::string text = GetString(section, key, default_value_str);
    // stringstream中类型会对应匹配
    std::istringstream convert2(text);

    float fresult;
    // 转换失败的时候fresult会置为0
    if (!(convert2 >> fresult)) fresult = 0;
    return fresult;
}

// 载入配置文件
bool config_file::Load(const std::string& path) {
    std::ifstream ifs(path.c_str());
    if (!ifs.good()) return false;

    std::string line;
    std::map<std::string, std::string> *m = nullptr;

    while (!ifs.eof() && ifs.good()) {
        getline(ifs, line);
        std::string section;
        // 如果当前行是section就创建对应的value
        if (isSection(line, section)) {
            STR_MAP_ITER it = _map.find(section);
            if (it == _map.end()) {
                m = new std::map<std::string, std::string> ();
                // _map.insert(STR_MAP::value_type(section, m));
                _map.insert({section, m});
            } else m = it->second;
            continue;
        }

        size_t equ_pos = line.find('=');
        // 没找到,继续处理下一行
        if (equ_pos == std::string::npos) continue;
        // ip
        std::string key = line.substr(0, equ_pos);
        // 127.0.0.1
        std::string value = line.substr(equ_pos + 1);
        // ip_ -> ip
        key = trim(key);
        // _127.0.0.1 -> 127.0.0.1
        value = trim(value);

        if (key.empty()) continue;
        if (key[0] == '#' || key[0] == ':') continue;
        std::map<std::string, std::string>::iterator it1 = m->find(key);
        if (it1 != m->end()) it1->second = value;
        else m->insert({key, value});
    }
    ifs.close();
    return true;
}

std::vector<std::string> config_file::GetStringList(const std::string& section, const std::string& key) {
    std::vector<std::string> v;
    std::string str = GetString(section, key, "");
    std::string sep = ", \t";
    std::string substr;
    std::string::size_type start = 0;
    std::string::size_type index;

    while ((index = str.find_first_of(sep, start)) != std::string::npos) {
        substr = str.substr(start, index - start);
        v.push_back(substr);

        start = str.find_first_not_of(sep, index);
        if (start == std::string::npos) return v;
    }
    substr = str.substr(start);
    v.push_back(substr);
    return v;
}

// 获取整型类型键值对
unsigned int config_file::GetNumber(const std::string& section, const std::string& key, unsigned default_value) {
    auto it = _map.find(section);
    if (it != _map.end()) {
        std::map<std::string, std::string>::const_iterator it1 = it->second->find(key);
        if (it1 != it->second->end()) return parseNumber(it1->second);
    }
    return default_value;
}

// 获取布尔类型键值对
bool config_file::GetBool(const std::string& section, const std::string& key, bool default_value) {
    auto it = _map.find(section);
    if (it != _map.end()) {
        std::map<std::string, std::string>::const_iterator it1 = it->second->find(key);
        if (it1 != it->second->end())
            if (strcasecmp(it1->second.c_str(), "true") == 0) return true;
    }
    return default_value;
}

// [section]
bool config_file::isSection(std::string line, std::string& section) {
    section = trim(line);
    if (section.empty() || section.length() <= 2) return false;
    // 使用at更安全
    if (section[0] != '[' || section.at(section.length() - 1) != ']') return false;

    section = section.substr(1, section.length() - 2);
    section = trim(section);

    return true;
}

/// @brief string -> long long 
unsigned config_file::parseNumber(const std::string& s) {
    std::istringstream iss(s);
    long long v = 0;
    iss >> v;
    return v;
}


/// @brief 将s左边的空格都去除
/// @details s = "ip = 127.0.0.1" 
std::string config_file::trimLeft(const std::string& s) {
    size_t first_not_empty = 0;

    std::string::const_iterator beg = s.begin();

    while (beg != s.end()) {
        // 判断*beg是不是空白字符串
        // 空格 /t /n /v ...
        if (!isspace(*beg)) break;
        first_not_empty++;
        beg++;
    }
    return s.substr(first_not_empty);
}

/// @brief 将s右边的空白字符都去掉
std::string config_file::trimRight(const std::string& s) {
    size_t last_not_empty = s.size();

    std::string::const_iterator end = s.end();

    while (end != s.begin()) {
        end--;
        if (!isspace(*end)) break;
        last_not_empty--;
    }
    return s.substr(0, last_not_empty);
}

/// @brief 将s两边的空白字符都去掉
std::string config_file::trim(const std::string& s) {
    return trimLeft(trimRight(s));
}

/// @brief 设置配置文件的路径,路径设置好之后就应该直接载入
bool config_file::setPath(const std::string& path) {
    qc_assert(config == nullptr);
    config = config_file_instance::GetInstance();
    return config->Load(path);
}

void config_file::get_all_info(config_file* instance) {
    std::cout << "get_all_info begin" << std::endl;

    std::cout << "instance->_map.size() = " << instance->_map.size() << std::endl;
    for (auto it = instance->_map.begin(); it != instance->_map.end(); ++it) {
        std::cout << '[' << it->first << ']' << std::endl;
        for (auto itt = it->second->begin(); itt != it->second->end(); ++itt) {
            std::cout << itt->first << "  " << itt->second << std::endl;
        }
    }
    std::cout << "get_all_info end" << std::endl;
}
}