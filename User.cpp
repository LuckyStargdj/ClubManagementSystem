#include "User.h"
#include "Utils.h"
#include <iostream>

User::User(const std::string& uname, const std::string& pwd, const std::string& r)
    : username(uname), password(pwd), role(r) {}

// ��֤�����Ƿ���ȷ
bool User::authenticate(const std::string& pwd) const {
    return password == pwd;
}

std::string User::getUsername() const { return username; }
std::string User::getRole() const { return role; }
std::string User::getPassword() const { return password; }

Admin::Admin(const std::string& uname, const std::string& pwd)
    : User(uname, pwd, "admin") {}

void Admin::displayInfo() const {
    std::cout << "����Ա: " << username << "\n";
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
        throw std::runtime_error("�Ѵﵽ������3�����ŵ�����");
    }
    for (int id : joinedClubs) {
        if (id == clubId) {
            return false; // �Ѽ���
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
            return false; // �Ѳμ�
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
    std::cout << "ѧ��: " << name << " (" << studentId << ")\n";
    std::cout << "ѧԺ: " << college << ", ��ϵ��ʽ: " << contact << "\n";
    std::cout << "�Ѽ�������: " << getClubCount() << "/3\n";
}