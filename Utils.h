#ifndef UTILS_H
#define UTILS_H

#include <string>
#include <vector>

namespace Utils {

    // 字符串操作
    std::string trim(const std::string& str);
    std::vector<std::string> split(const std::string& str, char delimiter);
    std::string join(const std::vector<std::string>& vec, char delimiter);

    // 时间操作
    std::string timeToString(time_t time);
    time_t stringToTime(const std::string& str);

    // 输入验证
    bool isValidEmail(const std::string& email);
    bool isValidPhone(const std::string& phone);
    bool isNumeric(const std::string& str);

    void createDirectoryForFile(const std::string& filepath);
}

#endif // UTILS_H
