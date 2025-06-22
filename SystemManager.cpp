#include "SystemManager.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <filesystem>
#include <iomanip>
#include <algorithm>
#include <cctype>
#include <limits>
#include <vector>
#include <map>

#ifdef _WIN32
#include <conio.h>
#else
#include <termios.h>
#include <unistd.h>
#endif

SystemManager::SystemManager() {
    loadData();
}

SystemManager::~SystemManager() {
    saveData();
    for (auto& pair : users) {
        delete pair.second;
        pair.second = nullptr;
    }
}

void SystemManager::clearScreen() const {
#ifdef _WIN32
    system("cls");
#else
    system("clear");
#endif
}

void SystemManager::pressEnterToContinue() const {
    std::cout << "\n按Enter键继续...";
    std::cin.get();
}

int SystemManager::getIntInput(const std::string& prompt, int min, int max) const {
    int value;
    while (true) {
        std::cout << prompt;
        if (std::cin >> value) {
            if (value >= min && value <= max) {
                std::cin.ignore(); // 清除换行符
                return value;
            }
        }
        else {
            std::cin.clear();
        }
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        std::cout << "输入无效，请输入" << min << "-" << max << "之间的整数\n";
    }
}

std::string SystemManager::getStringInput(const std::string& prompt) {
    std::string input;
    std::cout << prompt;
    std::getline(std::cin, input);
    return Utils::trim(input);
}

bool SystemManager::login(const std::string& username, const std::string& password) {
    auto it = users.find(username);
    if (it != users.end() && it->second->authenticate(password)) {
        currentUser = it->second;
        return true;
    }
    return false;
}

void SystemManager::logout() {
    currentUser = nullptr;
}

OrdinaryUser* SystemManager::getCurrentOrdinaryUser() const {
    if (currentUser && currentUser->getRole() == "user") {
        return dynamic_cast<OrdinaryUser*>(currentUser);
    }
    return nullptr;
}

void SystemManager::loadData() {
    // 加载用户数据
    Utils::createDirectoryForFile(userFile);
    std::ifstream ufile(userFile);
    if (ufile) {
        std::string line;
        while (std::getline(ufile, line)) {
            auto parts = Utils::split(line, '|');
            if (parts.size() >= 3) {
                if (parts[0] == "admin") {
                    users[parts[1]] = new Admin(parts[1], parts[2]);
                }
                else if (parts[0] == "user" && parts.size() >= 7) {
                    users[parts[1]] = new OrdinaryUser(parts[3], parts[1], parts[2],
                        parts[4], parts[5], parts[6]);
                    // 加载加入的社团
                    if (parts.size() > 7) {
                        auto clubIds = Utils::split(parts[7], ',');
                        for (const auto& idStr : clubIds) {
                            if (!idStr.empty()) {
                                int clubId = std::stoi(idStr);
                                dynamic_cast<OrdinaryUser*>(users[parts[1]])->joinClub(clubId);
                            }
                        }
                    }
                    // 加载参与的活动
                    if (parts.size() > 8) {
                        auto activityIds = Utils::split(parts[8], ',');
                        for (const auto& idStr : activityIds) {
                            if (!idStr.empty()) {
                                int activityId = std::stoi(idStr);
                                dynamic_cast<OrdinaryUser*>(users[parts[1]])->participateActivity(activityId);
                            }
                        }
                    }
                }
            }
        }
    }

    // 加载社团数据
    std::ifstream cfile(clubFile);
    if (cfile) {
        std::string line;
        while (std::getline(cfile, line)) {
            auto parts = Utils::split(line, '|');
            if (parts.size() >= 6) {
                int id = std::stoi(parts[0]);
                time_t createTime = Utils::stringToTime(parts[5]);
                clubs[id] = Club(id, parts[1], parts[2], parts[3], parts[4]);
                clubs[id] = Club(id, parts[1], parts[2], parts[3], parts[4]);
                clubs[id].setCreateTime(createTime); // 设置创建时间
                // 加载成员
                if (parts.size() > 6) {
                    auto memberIds = Utils::split(parts[6], ',');
                    for (const auto& sid : memberIds) {
                        if (!sid.empty()) {
                            clubs[id].addMember(sid);
                        }
                    }
                }
                if (id >= nextClubId) nextClubId = id + 1;
            }
        }
    }

    // 加载活动数据
    std::ifstream afile(activityFile);
    if (afile) {
        std::string line;
        while (std::getline(afile, line)) {
            auto parts = Utils::split(line, '|');
            if (parts.size() >= 7) {
                int id = std::stoi(parts[0]);
                int clubId = std::stoi(parts[1]);
                time_t time = Utils::stringToTime(parts[3]);
                activities[id] = Activity(id, clubId, parts[2], time,
                    parts[4], parts[5], parts[6]);
                // 加载参与者
                if (parts.size() > 7) {
                    auto participantIds = Utils::split(parts[7], ',');
                    for (const auto& sid : participantIds) {
                        if (!sid.empty()) {
                            activities[id].addParticipant(sid);
                        }
                    }
                }
                // 加载反馈
                if (parts.size() > 8) {
                    auto feedbacks = Utils::split(parts[8], ';');
                    for (const auto& fb : feedbacks) {
                        if (!fb.empty()) {
                            auto fbParts = Utils::split(fb, ':');
                            if (fbParts.size() == 3) {
                                int rating = std::stoi(fbParts[1]);
                                activities[id].submitFeedback(fbParts[0], rating, fbParts[2]);
                            }
                        }
                    }
                }
                if (id >= nextActivityId) nextActivityId = id + 1;
            }
        }
    }
}

void SystemManager::saveData() {
    // 保存用户数据
    std::ofstream ufile(userFile);
    if (std::filesystem::file_size(userFile) == 0) {
        ufile << "角色|用户名|密码|学号|姓名|学院|联系方式" << "\n";
    }
    for (const auto& pair : users) {
        User* user = pair.second;
        if (!user) {
            return;
        }
        if (user->getRole() == "admin") {
            ufile << "admin|" << user->getUsername() << "|" << user->getPassword() << "\n";
        }
        else if (user->getRole() == "user") {
            OrdinaryUser* ou = dynamic_cast<OrdinaryUser*>(user);
            ufile << "user|" << ou->getUsername() << "|" << ou->getPassword() << "|"
                << ou->getStudentId() << "|" << ou->getName() << "|"
                << ou->getCollege() << "|" << ou->getContact();
            // 保存加入的社团
            const auto& clubIds = ou->getJoinedClubs();
            if (!clubIds.empty()) {
                ufile << "|";
                for (size_t i = 0; i < clubIds.size(); ++i) {
                    if (i > 0) ufile << ",";
                    ufile << clubIds[i];
                }
            }
            else {
                ufile << "|";
            }
            // 保存参与的活动
            const auto& activityIds = ou->getParticipatedActivities();
            if (!activityIds.empty()) {
                ufile << "|";
                for (size_t i = 0; i < activityIds.size(); ++i) {
                    if (i > 0) ufile << ",";
                    ufile << activityIds[i];
                }
            }
            ufile << "\n";
        }
    }

    // 保存社团数据
    std::ofstream cfile(clubFile);
    for (const auto& pair : clubs) {
        const Club& club = pair.second;
        cfile << club.getId() << "|" << club.getName() << "|" << club.getType() << "|"
            << club.getDescription() << "|" << club.getLeader() << "|"
            << Utils::timeToString(club.getCreateTime()) << "|";
        // 保存成员
        const auto& memberIds = club.getMemberIds();
        for (size_t i = 0; i < memberIds.size(); ++i) {
            if (i > 0) cfile << ",";
            cfile << memberIds[i];
        }
        cfile << "\n";
    }

    // 保存活动数据
    std::ofstream afile(activityFile);
    for (const auto& pair : activities) {
        const Activity& activity = pair.second;
        afile << activity.getId() << "|" << activity.getClubId() << "|" << activity.getName() << "|"
            << Utils::timeToString(activity.getTime()) << "|" << activity.getLocation() << "|"
            << activity.getType() << "|" << activity.getDescription() << "|";
        // 保存参与者
        const auto& participantIds = activity.getParticipantIds();
        for (size_t i = 0; i < participantIds.size(); ++i) {
            if (i > 0) afile << ",";
            afile << participantIds[i];
        }
        afile << "|";
        // 保存反馈
        bool firstFeedback = true;
        for (const auto& p : participantIds) {
            const auto* fb = activity.getFeedback(p);
            if (fb) {
                if (!firstFeedback) afile << ";";
                afile << p << ":" << fb->rating << ":" << fb->comment;
                firstFeedback = false;
            }
        }
        afile << "\n";
    }
}

// 管理员功能实现
void SystemManager::addClub() {
    clearScreen();
    std::cout << "=== 添加新社团 ===\n";

    std::string name = getStringInput("社团名称: ");
    std::string type;
    while (true) {
        std::cout << "社团类型 (1.学术 2.文体 3.公益): ";
        int choice = getIntInput("", 1, 3);
        if (choice == 1) type = "学术";
        else if (choice == 2) type = "文体";
        else if (choice == 3) type = "公益";
        break;
    }
    std::string desc = getStringInput("社团简介: ");
    std::string leader = getStringInput("负责人: ");

    Club newClub(nextClubId++, name, type, desc, leader);
    clubs[newClub.getId()] = newClub;

    std::cout << "\n社团添加成功! ID: " << newClub.getId() << "\n";
    pressEnterToContinue();
}

void SystemManager::removeClub() {
    clearScreen();
    std::cout << "=== 删除社团 ===\n";
    displayAllClubs();

    if (clubs.empty()) {
        std::cout << "\n没有可删除的社团\n";
        pressEnterToContinue();
        return;
    }

    int id = getIntInput("输入要删除的社团ID: ", 1, nextClubId - 1);
    if (clubs.find(id) != clubs.end()) {
        // 删除相关活动
        for (auto it = activities.begin(); it != activities.end();) {
            if (it->second.getClubId() == id) {
                it = activities.erase(it);
            }
            else {
                ++it;
            }
        }

        // 删除用户关联
        for (auto& userPair : users) {
            if (userPair.second->getRole() == "user") {
                OrdinaryUser* user = dynamic_cast<OrdinaryUser*>(userPair.second);
                user->leaveClub(id);
            }
        }

        // 删除社团
        clubs.erase(id);
        std::cout << "社团删除成功!\n";
    }
    else {
        std::cout << "未找到该社团!\n";
    }
    pressEnterToContinue();
}

void SystemManager::registerMember() {
    clearScreen();
    std::cout << "=== 注册新成员 ===\n";

    std::string sid = getStringInput("学号: ");
    std::string name = getStringInput("姓名: ");
    std::string college = getStringInput("学院: ");
    std::string contact = getStringInput("联系方式: ");
    std::string username = getStringInput("用户名: ");

    // 检查用户名是否已存在
    if (users.find(username) != users.end()) {
        std::cout << "用户名已存在!\n";
        pressEnterToContinue();
        return;
    }

    std::string password;
    while (true) {
        password = getStringInput("密码: ");
        std::string confirm = getStringInput("确认密码: ");
        if (password == confirm) break;
        std::cout << "两次输入的密码不一致，请重新输入!\n";
    }

    OrdinaryUser* newUser = new OrdinaryUser(sid, username, password, name, college, contact);
    users[username] = newUser;

    std::cout << "\n成员注册成功!\n";
    pressEnterToContinue();
}

void SystemManager::viewAllMembers() {
    clearScreen();
    std::cout << "=== 所有成员 ===\n\n";
    for (const auto& pair : users) {
        if (pair.second->getRole() == "user") {
            OrdinaryUser* user = dynamic_cast<OrdinaryUser*>(pair.second);
            std::cout << "用户名: " << user->getUsername()
                << " | 学号: " << user->getStudentId()
                << " | 姓名: " << user->getName()
                << " | 学院: " << user->getCollege()
                << " | 联系方式: " << user->getContact() << "\n";
        }
    }
    pressEnterToContinue();
}

void SystemManager::publishActivity() {
    clearScreen();
    std::cout << "=== 发布新活动 ===\n";
    displayAllClubs();

    if (clubs.empty()) {
        std::cout << "\n请先创建社团!\n";
        pressEnterToContinue();
        return;
    }

    int clubId = getIntInput("选择社团ID: ", 1, nextClubId - 1);
    if (clubs.find(clubId) == clubs.end()) {
        std::cout << "无效的社团ID!\n";
        pressEnterToContinue();
        return;
    }

    std::string name = getStringInput("活动名称: ");
    std::string type = getStringInput("活动类型: ");
    std::string location = getStringInput("活动地点: ");
    std::string desc = getStringInput("活动说明: ");

    // 获取活动时间
    std::string timeStr;
    while (true) {
        timeStr = getStringInput("活动时间 (格式: YYYY-MM-DD HH:MM): ");
        if (Utils::stringToTime(timeStr) != -1) break;
        std::cout << "时间格式错误，请使用 YYYY-MM-DD HH:MM 格式!\n";
    }
    time_t time = Utils::stringToTime(timeStr);

    Activity newActivity(nextActivityId++, clubId, name, time, location, type, desc);
    activities[newActivity.getId()] = newActivity;

    std::cout << "\n活动发布成功! ID: " << newActivity.getId() << "\n";
    pressEnterToContinue();
}

void SystemManager::displayStatistics() const {
    clearScreen();
    std::cout << "=== 社团统计信息 ===\n\n";

    // 按类型统计
    std::map<std::string, int> typeCount;
    std::map<std::string, std::vector<const Activity*>> typeActivities;

    for (const auto& clubPair : clubs) {
        const Club& club = clubPair.second;
        typeCount[club.getType()]++;

        // 收集社团活动
        for (const auto& actPair : activities) {
            if (actPair.second.getClubId() == club.getId()) {
                typeActivities[club.getType()].push_back(&actPair.second);
            }
        }
    }

    // 显示统计信息
    std::cout << "社团类型分布:\n";
    for (const auto& pair : typeCount) {
        std::cout << "  " << pair.first << "类: " << pair.second << "个社团\n";
    }

    std::cout << "\n各社团成员数量:\n";
    for (const auto& clubPair : clubs) {
        const Club& club = clubPair.second;
        std::cout << "  " << club.getName() << ": " << club.getMemberCount() << "人\n";
    }

    std::cout << "\n特色活动展示:\n";
    for (const auto& pair : typeActivities) {
        std::cout << "  " << pair.first << "类社团特色活动:\n";
        for (const Activity* act : pair.second) {
            std::cout << "    - " << act->getName() << " (" << Utils::timeToString(act->getTime()) << ")\n";
        }
    }

    std::cout << "\n活动参与率统计:\n";
    for (const auto& clubPair : clubs) {
        const Club& club = clubPair.second;
        int memberCount = club.getMemberCount();
        if (memberCount == 0) continue;

        int totalActivities = 0;
        int totalParticipants = 0;

        for (const auto& actPair : activities) {
            if (actPair.second.getClubId() == club.getId()) {
                totalActivities++;
                int cnt = static_cast<int>(actPair.second.getParticipantIds().size());
                totalParticipants += cnt;
            }
        }

        double avgParticipation = totalActivities > 0 ?
            (static_cast<double>(totalParticipants) / (memberCount * totalActivities)) * 100 : 0.0;

        std::cout << "  " << club.getName() << ": "
            << std::fixed << std::setprecision(1) << avgParticipation << "%\n";
    }

    pressEnterToContinue();
}

// 普通用户功能实现
void SystemManager::joinClub() {
    auto user = getCurrentOrdinaryUser();
    if (!user) return;

    clearScreen();
    std::cout << "=== 加入社团 ===\n";
    displayAllClubs();

    if (clubs.empty()) {
        std::cout << "\n当前没有可加入的社团\n";
        pressEnterToContinue();
        return;
    }

    if (user->getClubCount() >= 3) {
        std::cout << "\n您已加入3个社团，无法加入更多!\n";
        pressEnterToContinue();
        return;
    }

    int clubId = getIntInput("输入要加入的社团ID: ", 1, nextClubId - 1);
    if (clubs.find(clubId) != clubs.end()) {
        try {
            if (user->joinClub(clubId)) {
                clubs[clubId].addMember(user->getStudentId());
                std::cout << "成功加入社团!\n";
            }
            else {
                std::cout << "您已加入该社团!\n";
            }
        }
        catch (const std::runtime_error& e) {
            std::cout << "操作失败: " << e.what() << "\n";
        }
    }
    else {
        std::cout << "未找到该社团!\n";
    }
    pressEnterToContinue();
}

void SystemManager::leaveClub() {
    auto user = getCurrentOrdinaryUser();
    if (!user) return;

    clearScreen();
    std::cout << "=== 退出社团 ===\n";
    viewMyClubs();

    const auto& joinedClubs = user->getJoinedClubs();
    if (joinedClubs.empty()) {
        std::cout << "\n您尚未加入任何社团\n";
        pressEnterToContinue();
        return;
    }

    int clubId = getIntInput("输入要退出的社团ID: ", 1, nextClubId - 1);
    if (std::find(joinedClubs.begin(), joinedClubs.end(), clubId) != joinedClubs.end()) {
        if (clubs.find(clubId) != clubs.end()) {
            clubs[clubId].removeMember(user->getStudentId());
            user->leaveClub(clubId);
            std::cout << "成功退出社团!\n";
        }
    }
    else {
        std::cout << "您未加入该社团!\n";
    }
    pressEnterToContinue();
}

void SystemManager::participateActivity() {
    auto user = getCurrentOrdinaryUser();
    if (!user) return;

    clearScreen();
    std::cout << "=== 参加活动 ===\n";
    displayAllActivities();

    if (activities.empty()) {
        std::cout << "\n当前没有可参加的活动\n";
        pressEnterToContinue();
        return;
    }

    int activityId = getIntInput("输入要参加的活动ID: ", 1, nextActivityId - 1);
    if (activities.find(activityId) != activities.end()) {
        if (user->participateActivity(activityId)) {
            activities[activityId].addParticipant(user->getStudentId());
            std::cout << "成功参加活动!\n";
        }
        else {
            std::cout << "您已参加该活动!\n";
        }
    }
    else {
        std::cout << "未找到该活动!\n";
    }
    pressEnterToContinue();
}

void SystemManager::submitFeedback() {
    auto user = getCurrentOrdinaryUser();
    if (!user) return;

    clearScreen();
    std::cout << "=== 提交活动反馈 ===\n";
    viewMyActivities();

    const auto& participatedActivities = user->getParticipatedActivities();
    if (participatedActivities.empty()) {
        std::cout << "\n您尚未参加任何活动\n";
        pressEnterToContinue();
        return;
    }

    int activityId = getIntInput("输入要反馈的活动ID: ", 1, nextActivityId - 1);
    if (activities.find(activityId) != activities.end()) {
        Activity& activity = activities[activityId];

        // 检查是否参加了该活动
        if (std::find(participatedActivities.begin(), participatedActivities.end(), activityId) == participatedActivities.end()) {
            std::cout << "您未参加该活动!\n";
            pressEnterToContinue();
            return;
        }

        // 检查活动是否已结束
        if (activity.getTime() > time(nullptr)) {
            std::cout << "活动尚未结束，无法提交反馈!\n";
            pressEnterToContinue();
            return;
        }

        // 检查是否已提交反馈
        if (activity.getFeedback(user->getStudentId())) {
            std::cout << "您已提交过该活动的反馈!\n";
            pressEnterToContinue();
            return;
        }

        int rating = getIntInput("评分 (1-5星): ", 1, 5);
        std::string comment = getStringInput("反馈意见: ");

        if (activity.submitFeedback(user->getStudentId(), rating, comment)) {
            std::cout << "反馈提交成功!\n";
        }
        else {
            std::cout << "反馈提交失败!\n";
        }
    }
    else {
        std::cout << "未找到该活动!\n";
    }
    pressEnterToContinue();
}

void SystemManager::viewMyClubs() const {
    auto user = getCurrentOrdinaryUser();
    if (!user) return;

    clearScreen();
    std::cout << "=== 我加入的社团 ===\n";

    const auto& clubIds = user->getJoinedClubs();
    if (clubIds.empty()) {
        std::cout << "\n您尚未加入任何社团\n";
        pressEnterToContinue();
        return;
    }

    std::cout << "已加入社团 (" << clubIds.size() << "/3):\n";
    for (int id : clubIds) {
        if (clubs.find(id) != clubs.end()) {
            const Club& club = clubs.at(id);
            std::cout << "ID: " << id << " | 名称: " << club.getName()
                << " | 类型: " << club.getType() << "\n";
        }
    }

    pressEnterToContinue();
}

void SystemManager::viewMyActivities() const {
    auto user = getCurrentOrdinaryUser();
    if (!user) return;

    clearScreen();
    std::cout << "=== 我参加的活动 ===\n";

    const auto& activityIds = user->getParticipatedActivities();
    if (activityIds.empty()) {
        std::cout << "\n您尚未参加任何活动\n";
        pressEnterToContinue();
        return;
    }

    time_t now = time(nullptr);
    std::cout << "已参加活动:\n";
    for (int id : activityIds) {
        if (activities.find(id) != activities.end()) {
            const Activity& activity = activities.at(id);
            std::string status = (activity.getTime() > now) ? "待参加" : "已完成";
            std::cout << "ID: " << id << " | 活动: " << activity.getName()
                << " | 时间: " << Utils::timeToString(activity.getTime())
                << " | 状态: " << status << "\n";
        }
    }

    pressEnterToContinue();
}

// 公共功能实现

void SystemManager::displayAllClubs() const {
    if (clubs.empty()) {
        std::cout << "暂无社团信息\n";
        return;
    }

    std::cout << "社团列表:\n";
    for (const auto& pair : clubs) {
        const Club& club = pair.second;
        std::cout << "ID: " << club.getId() << " | 名称: " << club.getName()
            << " | 类型: " << club.getType() << " | 成员数: "
            << club.getMemberCount() << "\n";
    }
}

void SystemManager::viewAllClubs() const {
    clearScreen();
    std::cout << "=== 所有社团 ===\n\n";
    displayAllClubs();
    pressEnterToContinue();
}

void SystemManager::displayAllActivities() const {
    if (activities.empty()) {
        std::cout << "暂无活动信息\n";
        return;
    }

    std::cout << "活动列表:\n";
    for (const auto& pair : activities) {
        const Activity& activity = pair.second;
        std::string clubName = "未知社团";
        if (clubs.find(activity.getClubId()) != clubs.end()) {
            clubName = clubs.at(activity.getClubId()).getName();
        }

        std::cout << "ID: " << activity.getId() << " | 活动: " << activity.getName()
            << " | 社团: " << clubName << " | 时间: "
            << Utils::timeToString(activity.getTime()) << "\n";
    }
}

void SystemManager::viewAllActivities() const {
    clearScreen();
    std::cout << "=== 所有活动 ===\n\n";
    displayAllActivities();
    pressEnterToContinue();
}

void SystemManager::viewClubDetails() const {
    clearScreen();
    std::cout << "=== 查看社团详情 ===\n";
    displayAllClubs();

    if (clubs.empty()) {
        pressEnterToContinue();
        return;
    }

    int clubId = getIntInput("\n输入要查看的社团ID: ", 1, nextClubId - 1);
    if (clubs.find(clubId) != clubs.end()) {
        clearScreen();
        const Club& club = clubs.at(clubId);
        club.displayInfo();

        // 显示社团活动
        std::cout << "\n社团活动:\n";
        bool hasActivities = false;
        for (const auto& actPair : activities) {
            if (actPair.second.getClubId() == clubId) {
                hasActivities = true;
                std::cout << "  - " << actPair.second.getName()
                    << " (" << Utils::timeToString(actPair.second.getTime()) << ")\n";
            }
        }

        if (!hasActivities) {
            std::cout << "  暂无活动\n";
        }

        // 显示社团成员
        std::cout << "\n社团成员 (" << club.getMemberCount() << "人):\n";
        const auto& memberIds = club.getMemberIds();
        for (const auto& sid : memberIds) {
            // 查找成员姓名
            std::string memberName = "未知";
            for (const auto& userPair : users) {
                if (userPair.second->getRole() == "user") {
                    OrdinaryUser* user = dynamic_cast<OrdinaryUser*>(userPair.second);
                    if (user->getStudentId() == sid) {
                        memberName = user->getName();
                        break;
                    }
                }
            }
            std::cout << "  - " << sid << " (" << memberName << ")\n";
        }
    }
    else {
        std::cout << "未找到该社团!\n";
    }
    pressEnterToContinue();
}

void SystemManager::viewActivityDetails() const {
    clearScreen();
    std::cout << "=== 查看活动详情 ===\n";
    displayAllActivities();

    if (activities.empty()) {
        pressEnterToContinue();
        return;
    }

    int activityId = getIntInput("\n输入要查看的活动ID: ", 1, nextActivityId - 1);
    if (activities.find(activityId) != activities.end()) {
        clearScreen();
        const Activity& activity = activities.at(activityId);
        activity.displayInfo();

        // 显示所属社团
        std::string clubName = "未知社团";
        if (clubs.find(activity.getClubId()) != clubs.end()) {
            clubName = clubs.at(activity.getClubId()).getName();
        }
        std::cout << "所属社团: " << clubName << "\n";

        // 显示参与情况
        std::cout << "参与情况: ";
        if (activity.getParticipantIds().empty()) {
            std::cout << "暂无参与者\n";
        }
        else {
            std::cout << activity.getParticipantIds().size() << "人参与\n";
        }

        // 显示反馈
        std::cout << "\n活动反馈:\n";
        bool hasFeedback = false;
        for (const auto& sid : activity.getParticipantIds()) {
            const auto* feedback = activity.getFeedback(sid);
            if (feedback) {
                hasFeedback = true;
                // 查找成员姓名
                std::string memberName = "未知";
                for (const auto& userPair : users) {
                    if (userPair.second->getRole() == "user") {
                        OrdinaryUser* user = dynamic_cast<OrdinaryUser*>(userPair.second);
                        if (user->getStudentId() == sid) {
                            memberName = user->getName();
                            break;
                        }
                    }
                }

                std::cout << "  - " << memberName << " (" << sid << ")\n";
                std::cout << "    评分: " << feedback->rating << "星\n";
                std::cout << "    反馈: " << feedback->comment << "\n\n";
            }
        }

        if (!hasFeedback) {
            std::cout << "  暂无反馈\n";
        }
    }
    else {
        std::cout << "未找到该活动!\n";
    }
    pressEnterToContinue();
}

const std::map<std::string, User*>& SystemManager::getUsers() const {
    return users;
}

void SystemManager::addUser(const std::string& username, User* user) {
    users[username] = user;
}

void SystemManager::removeUser(const std::string& username) {
    auto it = users.find(username);
    if (it != users.end()) {
        users.erase(username);
    }
}

std::string SystemManager::getCurrentUserRole() const {
    if (currentUser) {
        return currentUser->getRole();
    }
    return "";
}


// 菜单系统实现
void SystemManager::showMainMenu() {
    clearScreen();
    std::cout << "=== 高校社团管理系统 ===\n\n";
    std::cout << "1. 登录\n";
    std::cout << "2. 退出系统\n";
}

void SystemManager::showAdminMenu() {
    while (currentUser) {
        clearScreen();
        std::cout << "=== 管理员菜单 (" << currentUser->getUsername() << ") ===\n\n";
        std::cout << "1. 社团管理\n";
        std::cout << "2. 成员管理\n";
        std::cout << "3. 活动管理\n";
        std::cout << "4. 统计展示\n";
        std::cout << "5. 退出登录\n";

        int choice = getIntInput("请选择: ", 1, 5);

        switch (choice) {
        case 1: // 社团管理
            clearScreen();
            std::cout << "=== 社团管理 ===\n\n";
            std::cout << "1. 添加社团\n";
            std::cout << "2. 删除社团\n";
            std::cout << "3. 查看所有社团\n";
            std::cout << "4. 查看社团详情\n";
            std::cout << "5. 返回上级菜单\n";

            switch (getIntInput("请选择: ", 1, 5)) {
            case 1: addClub(); break;
            case 2: removeClub(); break;
            case 3: viewAllClubs(); break;
            case 4: viewClubDetails(); break;
            case 5: break; // 返回
            }
            break;

        case 2: // 成员管理
            clearScreen();
            std::cout << "=== 成员管理 ===\n\n";
            std::cout << "1. 注册新成员\n";
            std::cout << "2. 查看所有成员\n";
            std::cout << "3. 返回上级菜单\n";

            switch (getIntInput("请选择: ", 1, 3)) {
            case 1: registerMember(); break;
            case 2: viewAllMembers(); break;
            case 3: break; // 返回
            }
            break;

        case 3: // 活动管理
            clearScreen();
            std::cout << "=== 活动管理 ===\n\n";
            std::cout << "1. 发布新活动\n";
            std::cout << "2. 查看所有活动\n";
            std::cout << "3. 查看活动详情\n";
            std::cout << "4. 返回上级菜单\n";

            switch (getIntInput("请选择: ", 1, 4)) {
            case 1: publishActivity(); break;
            case 2: viewAllActivities(); break;
            case 3: viewActivityDetails(); break;
            case 4: break; // 返回
            }
            break;

        case 4: // 统计展示
            displayStatistics();
            break;

        case 5: // 退出登录
            logout();
            return;
        }
    }
}

void SystemManager::showUserMenu() {
    auto user = getCurrentOrdinaryUser();
    if (!user) return;

    while (currentUser) {
        clearScreen();
        std::cout << "=== 用户菜单 (" << user->getName() << ") ===\n\n";
        std::cout << "1. 查看社团信息\n";
        std::cout << "2. 管理我的社团\n";
        std::cout << "3. 查看活动信息\n";
        std::cout << "4. 管理我的活动\n";
        std::cout << "5. 提交活动反馈\n";
        std::cout << "6. 退出登录\n";

        int choice = getIntInput("请选择: ", 1, 6);

        switch (choice) {
        case 1: // 查看社团信息
            clearScreen();
            std::cout << "=== 社团信息 ===\n\n";
            std::cout << "1. 查看所有社团\n";
            std::cout << "2. 按类型筛选社团\n";
            std::cout << "3. 查看社团详情\n";
            std::cout << "4. 返回上级菜单\n";

            switch (getIntInput("请选择: ", 1, 4)) {
            case 1: viewAllClubs(); break;
            case 2:
                clearScreen();
                std::cout << "=== 按类型筛选社团 ===\n\n";
                std::cout << "1. 学术类\n";
                std::cout << "2. 文体类\n";
                std::cout << "3. 公益类\n";
                std::cout << "4. 返回\n";

                switch (getIntInput("请选择: ", 1, 4)) {
                case 1:
                    clearScreen();
                    std::cout << "=== 学术类社团 ===\n\n";
                    for (const auto& pair : clubs) {
                        if (pair.second.getType() == "学术") {
                            pair.second.displayInfo();
                            std::cout << "------------------------\n";
                        }
                    }
                    pressEnterToContinue();
                    break;
                case 2:
                    clearScreen();
                    std::cout << "=== 文体类社团 ===\n\n";
                    for (const auto& pair : clubs) {
                        if (pair.second.getType() == "文体") {
                            pair.second.displayInfo();
                            std::cout << "------------------------\n";
                        }
                    }
                    pressEnterToContinue();
                    break;
                case 3:
                    clearScreen();
                    std::cout << "=== 公益类社团 ===\n\n";
                    for (const auto& pair : clubs) {
                        if (pair.second.getType() == "公益") {
                            pair.second.displayInfo();
                            std::cout << "------------------------\n";
                        }
                    }
                    pressEnterToContinue();
                    break;
                case 4: break; // 返回
                }
                break;
            case 3: viewClubDetails(); break;
            case 4: break; // 返回
            }
            break;

        case 2: // 管理我的社团
            clearScreen();
            std::cout << "=== 我的社团管理 ===\n\n";
            std::cout << "1. 加入社团\n";
            std::cout << "2. 退出社团\n";
            std::cout << "3. 查看我的社团\n";
            std::cout << "4. 返回上级菜单\n";

            switch (getIntInput("请选择: ", 1, 4)) {
            case 1: joinClub(); break;
            case 2: leaveClub(); break;
            case 3: viewMyClubs(); break;
            case 4: break; // 返回
            }
            break;

        case 3: // 查看活动信息
            clearScreen();
            std::cout << "=== 活动信息 ===\n\n";
            std::cout << "1. 查看所有活动\n";
            std::cout << "2. 按时间排序活动\n";
            std::cout << "3. 按社团筛选活动\n";
            std::cout << "4. 查看活动详情\n";
            std::cout << "5. 返回上级菜单\n";

            switch (getIntInput("请选择: ", 1, 5)) {
            case 1:
                clearScreen();
                std::cout << "=== 所有活动 ===\n\n";
                displayAllActivities();
                pressEnterToContinue();
                break;
            case 2:
                clearScreen();
                std::cout << "=== 按时间排序活动 ===\n\n";
                {
                    // 创建活动指针列表
                    std::vector<const Activity*> sortedActivities;
                    for (const auto& pair : activities) {
                        sortedActivities.push_back(&pair.second);
                    }

                    // 按时间排序
                    std::sort(sortedActivities.begin(), sortedActivities.end(),
                        [](const Activity* a, const Activity* b) {
                            return a->getTime() < b->getTime();
                        });

                    // 显示结果
                    for (const Activity* act : sortedActivities) {
                        std::string clubName = "未知社团";
                        if (clubs.find(act->getClubId()) != clubs.end()) {
                            clubName = clubs.at(act->getClubId()).getName();
                        }
                        std::cout << "时间: " << Utils::timeToString(act->getTime())
                            << " | 活动: " << act->getName()
                            << " | 社团: " << clubName << "\n";
                    }
                }
                pressEnterToContinue();
                break;
            case 3:
                clearScreen();
                std::cout << "=== 按社团筛选活动 ===\n\n";
                displayAllClubs();
                if (!clubs.empty()) {
                    int clubId = getIntInput("\n输入社团ID: ", 1, nextClubId - 1);
                    if (clubs.find(clubId) != clubs.end()) {
                        clearScreen();
                        std::cout << "=== " << clubs.at(clubId).getName() << " 的活动 ===\n\n";
                        for (const auto& pair : activities) {
                            if (pair.second.getClubId() == clubId) {
                                std::cout << "活动: " << pair.second.getName()
                                    << " | 时间: " << Utils::timeToString(pair.second.getTime())
                                    << " | 地点: " << pair.second.getLocation() << "\n";
                            }
                        }
                    }
                    else {
                        std::cout << "未找到该社团!\n";
                    }
                }
                pressEnterToContinue();
                break;
            case 4: viewActivityDetails(); break;
            case 5: break; // 返回
            }
            break;

        case 4: // 管理我的活动
            clearScreen();
            std::cout << "=== 我的活动管理 ===\n\n";
            std::cout << "1. 参加活动\n";
            std::cout << "2. 查看我的活动\n";
            std::cout << "3. 返回上级菜单\n";

            switch (getIntInput("请选择: ", 1, 3)) {
            case 1: participateActivity(); break;
            case 2: viewMyActivities(); break;
            case 3: break; // 返回
            }
            break;

        case 5: // 提交活动反馈
            submitFeedback();
            break;

        case 6: // 退出登录
            logout();
            return;
        }
    }
}

