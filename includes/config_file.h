//
// Created by TrueAbc on 2021/9/3.
//

#ifndef LARS_CONFIG_FILE_H
#define LARS_CONFIG_FILE_H
#include <string>
#include <map>
#include <vector>
// 定义存放配置信息的map
// key 是string 存放section
// value 是一个map 存放该section下的所有k-v
typedef std::map<std::string, std::map<std::string, std::string>* > STR_MAP;

typedef STR_MAP ::iterator STR_MAP_ITER;

// 单例模式
class config_file{
public:
    ~config_file();

    //获取字符串类型配置信息
    std::string GetString(const std::string& section, const std::string& key, const std::string& default_value="");
    // 字符串集合配置信息
    std::vector<std::string> GetStringList(const std::string& section, const std::string& key);
    //整型
    unsigned GetNumber(const std::string& section, const std::string& key, unsigned default_value=0);
    // bool
    bool GetBool(const std::string& section, const std::string& key, bool default_value = false);
    // float
    float GetFloat(const std::string& section, const std::string& key, const float& default_value);
    // 配置文件路径
    static bool setPath(const std::string& path);
    // 单例
    static config_file *instance();
private:
    config_file()= default;

//  字符串配置文件解析基础方法
    bool isSection(std::string line, std::string& section);
    unsigned parseNumber(const std::string& s);
    std::string trimLeft(const std::string& s);
    std::string trimRight(const std::string& s);
    std::string trim(const std::string& s);
    bool Load(const std::string& path);

    static config_file *config;

    STR_MAP _map;
};
#endif //LARS_CONFIG_FILE_H
