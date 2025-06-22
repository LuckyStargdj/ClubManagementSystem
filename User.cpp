#include "User.h"
#include "Utils.h"
#include <iostream>

User::User(const std::string& uname, const std::string& pwd, const std::string& r)
    : username(uname), password(pwd), role(r) {}

// 验证密码是否正确
bool User::authenticate(const std::string& pwd) const {
    return password == pwd;
}

std::string User::getUsername() const { return username; }
std::string User::getRole() const { return role; }
std::string User::getPassword() const { return password; }

Admin::Admin(const std::string& uname, const std::string& pwd)
    : User(uname, pwd, "admin") {}

void Admin::displayInfo() const {
    std::cout << "管理员: " << username << "\n";
}

OrdinaryUser::OrdinaryUser(const std::string& sid, const std::string& uname,
    const std::string& pwd, const std::string& n,
    const std::string& col, const std::string& ct)
    : User(uname, pwd, "user"), studentId(sid), name(n), college(col), contact(ct) {}

void OrdinaryUser::updateContact(const std::string& newContact) {
    contact = newContact;
}

bool OrdinaryUser::joinClub(int clubId) {
    if (getClubCount() >= 3) {
        throw std::runtime_error("已达到最多加入3个社团的限制");
    }
    for (int id : joinedClubs) {
        if (id == clubId) {
            return false; // 已加入
        }
    }
    joinedClubs.push_back(clubId);
    return true;
}

bool OrdinaryUser::leaveClub(int clubId) {
    for (auto it = joinedClubs.begin(); it != joinedClubs.end(); ++it) {
        if (*it == clubId) {
            joinedClubs.erase(it);
            return true;
        }
    }
    return false;
}

bool OrdinaryUser::participateActivity(int activityId) {
    for (int id : participatedActivities) {
        if (id == activityId) {
            return false; // 已参加
        }
    }
    participatedActivities.push_back(activityId);
    return true;
}

std::string OrdinaryUser::getStudentId() const { return studentId; }
std::string OrdinaryUser::getName() const { return name; }
std::string OrdinaryUser::getCollege() const { return college; }
std::string OrdinaryUser::getContact() const { return contact; }
const std::vector<int>& OrdinaryUser::getJoinedClubs() const { return joinedClubs; }
const std::vector<int>& OrdinaryUser::getParticipatedActivities() const { return participatedActivities; }
int OrdinaryUser::getClubCount() const { return static_cast<int>(joinedClubs.size()); }

void OrdinaryUser::displayInfo() const {
    std::cout << "学生: " << name << " (" << studentId << ")\n";
    std::cout << "学院: " << college << ", 联系方式: " << contact << "\n";
    std::cout << "已加入社团: " << getClubCount() << "/3\n";
}