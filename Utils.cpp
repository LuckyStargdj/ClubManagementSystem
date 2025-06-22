#include "Utils.h"
#include <algorithm>
#include <filesystem>
#include <sstream>
#include <ctime>
#include <regex>
#include <cctype>
#include <sys/stat.h>

#ifdef _WIN32
#include <direct.h>
#else
#include <unistd.h>
#endif

// 去除字符串开头和结尾的所有空白字符
std::string Utils::trim(const std::string& str) {
    // 对于空字符串直接返回
    if (str.empty()) {
        return "";
    }

    // 使用UTF-8安全的查找方法
    auto isSpace = [](char c) -> bool {
        // 仅处理ASCII空白字符，避免影响多字节字符
        return (c == ' ' || c == '\t' || c == '\n' || c == '\r' || c == '\v' || c == '\f');
    };

    // 查找第一个非空白字符
    auto start = str.begin();
    while (start != str.end() && isSpace(*start)) {
        ++start;
    }

    // 查找最后一个非空白字符
    auto end = str.end();
    while (end != start && isSpace(*(end - 1))) {
        --end;
    }

    // 返回去除空白后的子字符串
    return std::string(start, end);
}

// 将字符串按指定分隔符拆分成多个子字符串
std::vector<std::string> Utils::split(const std::string& str, char delimiter) {
    std::vector<std::string> tokens;
    std::string token;
    std::istringstream tokenStream(str);
    while (std::getline(tokenStream, token, delimiter)) {
        tokens.push_back(trim(token));
    }
    return tokens;
}

// 将字符串向量中的元素按指定分隔符连接成一个新字符串
std::string Utils::join(const std::vector<std::string>& vec, char delimiter) {
    std::string result;
    for (size_t i = 0; i < vec.size(); ++i) {
        if (i != 0) result += delimiter;
        result += vec[i];
    }
    return result;
}

std::string Utils::timeToString(time_t time) {
    char buffer[20];
    struct tm timeInfo;

    // 使用 localtime_s 安全版本
    if (localtime_s(&timeInfo, &time) == 0) {
        // 格式化时间为字符串
        std::strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", &timeInfo);
        return std::string(buffer);
    }
    else {
        // 错误处理，返回默认值或抛出异常
        return "1970-01-01 00:00:00";
    }
}

time_t Utils::stringToTime(const std::string& str) {
    std::tm t = {};
    std::istringstream ss(str);
    ss >> std::get_time(&t, "%Y-%m-%d %H:%M:%S");
    return mktime(&t);
}

bool Utils::isValidEmail(const std::string& email) {
    const std::regex pattern(R"([a-zA-Z0-9._%+-]+@[a-zA-Z0-9.-]+\.[a-zA-Z]{2,})");
    return std::regex_match(email, pattern);
    return true;
}

bool Utils::isValidPhone(const std::string& phone) {
    const std::regex pattern(R"(\d{3}-\d{4}-\d{4})");
    return std::regex_match(phone, pattern);
    return true;
}

bool Utils::isNumeric(const std::string& str) {
    return !str.empty() && std::all_of(str.begin(), str.end(), ::isdigit);
}

void Utils::createDirectoryForFile(const std::string& filepath) {
    // 获取文件所在目录
    std::filesystem::path path(filepath);
    std::filesystem::path dirpath = path.parent_path();

    // 如果目录为空（说明是当前目录），则直接返回
    if (dirpath.empty()) {
        return;
    }

    // 创建目录
    if (!std::filesystem::exists(dirpath)) {
        std::filesystem::create_directories(dirpath);
    }
}



