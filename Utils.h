#ifndef UTILS_H
#define UTILS_H

#include <string>
#include <vector>

namespace Utils {

    // �ַ�������
    std::string trim(const std::string& str);
    std::vector<std::string> split(const std::string& str, char delimiter);
    std::string join(const std::vector<std::string>& vec, char delimiter);

    // ʱ�����
    std::string timeToString(time_t time);
    time_t stringToTime(const std::string& str);

    // ������֤
    bool isValidEmail(const std::string& email);
    bool isValidPhone(const std::string& phone);
    bool isNumeric(const std::string& str);

    void createDirectoryForFile(const std::string& filepath);
}

#endif // UTILS_H
