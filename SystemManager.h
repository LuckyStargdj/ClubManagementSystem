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
    std::map<std::string, User*> users; // �û���->�û�����
    std::map<int, Club> clubs; // ����ID->���Ŷ���
    std::map<int, Activity> activities; // �ID->�����
    User* currentUser = nullptr;

    // �ļ��洢·��
    std::string userFile = "data/users.dat";  // �û������ļ�
    std::string clubFile = "data/clubs.dat";  // ���������ļ�
    std::string activityFile = "data/activities.dat";  // ������ļ�

    // ID������
    int nextClubId = 1;
    int nextActivityId = 1;

public:
    SystemManager();
    ~SystemManager();

    // ��¼ϵͳ
    bool login(const std::string& username, const std::string& password);
    void logout();

    // ���ݳ־û�
    void loadData();
    void saveData();

    // ����Ա����
    void addClub();
    void removeClub();
    void registerMember();
    void viewAllMembers();
    void publishActivity();
    void displayStatistics() const;

    // ��ͨ�û�����
    void joinClub();
    void leaveClub();
    void participateActivity();
    void submitFeedback();
    void viewMyClubs() const;
    void viewMyActivities() const;

    // ��������
    void displayAllClubs() const;
    void viewAllClubs() const;
    void displayAllActivities() const;
    void viewAllActivities() const;
    void viewClubDetails() const;
    void viewActivityDetails() const;
    const std::map<std::string, User*>& getUsers() const;
    void addUser(const std::string& username, User* user);
    void removeUser(const std::string& username);

    // ��������
    OrdinaryUser* getCurrentOrdinaryUser() const;
    std::string getCurrentUserRole() const;
    void clearScreen() const;
    void pressEnterToContinue() const;
    int getIntInput(const std::string& prompt, int min, int max) const;
    std::string getStringInput(const std::string& prompt);

    // �˵�ϵͳ
    void showMainMenu();
    void showAdminMenu();
    void showUserMenu();
};

#endif // SYSTEM_MANAGER_H
