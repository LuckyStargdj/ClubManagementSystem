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

// ȥ���ַ�����ͷ�ͽ�β�����пհ��ַ�
std::string Utils::trim(const std::string& str) {
    // ���ڿ��ַ���ֱ�ӷ���
    if (str.empty()) {
        return "";
    }

    // ʹ��UTF-8��ȫ�Ĳ��ҷ���
    auto isSpace = [](char c) -> bool {
        // ������ASCII�հ��ַ�������Ӱ����ֽ��ַ�
        return (c == ' ' || c == '\t' || c == '\n' || c == '\r' || c == '\v' || c == '\f');
    };

    // ���ҵ�һ���ǿհ��ַ�
    auto start = str.begin();
    while (start != str.end() && isSpace(*start)) {
        ++start;
    }

    // �������һ���ǿհ��ַ�
    auto end = str.end();
    while (end != start && isSpace(*(end - 1))) {
        --end;
    }

    // ����ȥ���հ׺�����ַ���
    return std::string(start, end);
}

// ���ַ�����ָ���ָ�����ֳɶ�����ַ���
std::vector<std::string> Utils::split(const std::string& str, char delimiter) {
    std::vector<std::string> tokens;
    std::string token;
    std::istringstream tokenStream(str);
    while (std::getline(tokenStream, token, delimiter)) {
        tokens.push_back(trim(token));
    }
    return tokens;
}

// ���ַ��������е�Ԫ�ذ�ָ���ָ������ӳ�һ�����ַ���
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

    // ʹ�� localtime_s ��ȫ�汾
    if (localtime_s(&timeInfo, &time) == 0) {
        // ��ʽ��ʱ��Ϊ�ַ���
        std::strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", &timeInfo);
        return std::string(buffer);
    }
    else {
        // ����������Ĭ��ֵ���׳��쳣
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
    // ��ȡ�ļ�����Ŀ¼
    std::filesystem::path path(filepath);
    std::filesystem::path dirpath = path.parent_path();

    // ���Ŀ¼Ϊ�գ�˵���ǵ�ǰĿ¼������ֱ�ӷ���
    if (dirpath.empty()) {
        return;
    }

    // ����Ŀ¼
    if (!std::filesystem::exists(dirpath)) {
        std::filesystem::create_directories(dirpath);
    }
}



