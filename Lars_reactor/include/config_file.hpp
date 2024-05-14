/**
 * @file config_file.hpp
 * @author qc
 * @brief 配置文件
 * @details
            [reactor]
            ip = 127.0.0.1
            port = 7777
            maxConn = 1024
            thredNum = 5
 * @version 0.1
 * @date 2024-05-07
 *
 * @copyright Copyright (c) 2024
 *
 */

#pragma once

#include <string>
#include <map>
#include <vector>
#include "singleton.hpp"

namespace qc {

/// @brief 单例模式

/// @brief 定义存放被指文件的信息的map, key -> 存放一个标题的session(string), value -> map(标题下所有的key-value键值对)
/// @details string -> map<string, string> -> map<session, map<key, value>>
typedef std::map<std::string, std::map<std::string, std::string> *> STR_MAP;
/// @brief 迭代器
typedef STR_MAP::iterator STR_MAP_ITER;

class config_file : public Singleton<config_file> {
friend class Singleton<config_file>;
public:

    ~config_file();

    // 获取字符串类型配置信息
    std::string GetString(const std::string& section, const std::string& key, const std::string& default_value = "");
    // 字符串集合配置信息
    std::vector<std::string> GetStringList(const std::string& section, const std::string& key);
    // 获取整型类型配置信息
    unsigned int GetNumber(const std::string& section, const std::string& key, unsigned int default_value = 0);
    // 获取bool类型配置信息
    bool GetBool(const std::string& section, const std::string& key, bool default_value = false);
    // 获取浮点类型配置信息
    float GetFloat(const std::string& section, const std::string& key,const float& default_value);
    // 设置配置文件所在路径
    static bool setPath(const std::string& path);
    // 获取单例
    // config_file::GetInstance();
private:
    /// @brief 构造函数私有化
    config_file() {}

    /// @brief 字符串配置文件解析基础方法
    bool isSection(std::string line, std::string& section);
    unsigned int parseNumber(const std::string& s);
    std::string trimLeft(const std::string& s);
    std::string trimRight(const std::string& s);
    std::string trim(const std::string& s);
    bool Load(const std::string& path);

    /// @brief 使用单例模式类之后就不需要再单独声明一个static* config了
    // static config_file *config;
    STR_MAP _map;
};
}