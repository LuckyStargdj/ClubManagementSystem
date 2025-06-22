#ifndef SYSTEM_MANAGER_H
#define SYSTEM_MANAGER_H

#include "Utils.h"
#include "User.h"
#include "Club.h"
#include "Activity.h"
#include <map>
#include <vector>
#include <string>
#include <memory>
#include <fstream>

class SystemManager {
private:
    std::map<std::string, User*> users; // 用户名->用户对象
    std::map<int, Club> clubs; // 社团ID->社团对象
    std::map<int, Activity> activities; // 活动ID->活动对象
    User* currentUser = nullptr;

    // 文件存储路径
    std::string userFile = "data/users.dat";  // 用户数据文件
    std::string clubFile = "data/clubs.dat";  // 社团数据文件
    std::string activityFile = "data/activities.dat";  // 活动数据文件

    // ID生成器
    int nextClubId = 1;
    int nextActivityId = 1;

public:
    SystemManager();
    ~SystemManager();

    // 登录系统
    bool login(const std::string& username, const std::string& password);
    void logout();

    // 数据持久化
    void loadData();
    void saveData();

    // 管理员功能
    void addClub();
    void removeClub();
    void registerMember();
    void viewAllMembers();
    void publishActivity();
    void displayStatistics() const;

    // 普通用户功能
    void joinClub();
    void leaveClub();
    void participateActivity();
    void submitFeedback();
    void viewMyClubs() const;
    void viewMyActivities() const;

    // 公共功能
    void displayAllClubs() const;
    void viewAllClubs() const;
    void displayAllActivities() const;
    void viewAllActivities() const;
    void viewClubDetails() const;
    void viewActivityDetails() const;
    const std::map<std::string, User*>& getUsers() const;
    void addUser(const std::string& username, User* user);
    void removeUser(const std::string& username);

    // 辅助函数
    OrdinaryUser* getCurrentOrdinaryUser() const;
    std::string getCurrentUserRole() const;
    void clearScreen() const;
    void pressEnterToContinue() const;
    int getIntInput(const std::string& prompt, int min, int max) const;
    std::string getStringInput(const std::string& prompt);

    // 菜单系统
    void showMainMenu();
    void showAdminMenu();
    void showUserMenu();
};

#endif // SYSTEM_MANAGER_H
