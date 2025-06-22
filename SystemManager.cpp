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
    std::cout << "\n��Enter������...";
    std::cin.get();
}

int SystemManager::getIntInput(const std::string& prompt, int min, int max) const {
    int value;
    while (true) {
        std::cout << prompt;
        if (std::cin >> value) {
            if (value >= min && value <= max) {
                std::cin.ignore(); // ������з�
                return value;
            }
        }
        else {
            std::cin.clear();
        }
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        std::cout << "������Ч��������" << min << "-" << max << "֮�������\n";
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
    // �����û�����
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
                    // ���ؼ��������
                    if (parts.size() > 7) {
                        auto clubIds = Utils::split(parts[7], ',');
                        for (const auto& idStr : clubIds) {
                            if (!idStr.empty()) {
                                int clubId = std::stoi(idStr);
                                dynamic_cast<OrdinaryUser*>(users[parts[1]])->joinClub(clubId);
                            }
                        }
                    }
                    // ���ز���Ļ
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

    // ������������
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
                clubs[id].setCreateTime(createTime); // ���ô���ʱ��
                // ���س�Ա
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

    // ���ػ����
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
                // ���ز�����
                if (parts.size() > 7) {
                    auto participantIds = Utils::split(parts[7], ',');
                    for (const auto& sid : participantIds) {
                        if (!sid.empty()) {
                            activities[id].addParticipant(sid);
                        }
                    }
                }
                // ���ط���
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
    // �����û�����
    std::ofstream ufile(userFile);
    if (std::filesystem::file_size(userFile) == 0) {
        ufile << "��ɫ|�û���|����|ѧ��|����|ѧԺ|��ϵ��ʽ" << "\n";
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
            // ������������
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
            // �������Ļ
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

    // ������������
    std::ofstream cfile(clubFile);
    for (const auto& pair : clubs) {
        const Club& club = pair.second;
        cfile << club.getId() << "|" << club.getName() << "|" << club.getType() << "|"
            << club.getDescription() << "|" << club.getLeader() << "|"
            << Utils::timeToString(club.getCreateTime()) << "|";
        // �����Ա
        const auto& memberIds = club.getMemberIds();
        for (size_t i = 0; i < memberIds.size(); ++i) {
            if (i > 0) cfile << ",";
            cfile << memberIds[i];
        }
        cfile << "\n";
    }

    // ��������
    std::ofstream afile(activityFile);
    for (const auto& pair : activities) {
        const Activity& activity = pair.second;
        afile << activity.getId() << "|" << activity.getClubId() << "|" << activity.getName() << "|"
            << Utils::timeToString(activity.getTime()) << "|" << activity.getLocation() << "|"
            << activity.getType() << "|" << activity.getDescription() << "|";
        // ���������
        const auto& participantIds = activity.getParticipantIds();
        for (size_t i = 0; i < participantIds.size(); ++i) {
            if (i > 0) afile << ",";
            afile << participantIds[i];
        }
        afile << "|";
        // ���淴��
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

// ����Ա����ʵ��
void SystemManager::addClub() {
    clearScreen();
    std::cout << "=== ��������� ===\n";

    std::string name = getStringInput("��������: ");
    std::string type;
    while (true) {
        std::cout << "�������� (1.ѧ�� 2.���� 3.����): ";
        int choice = getIntInput("", 1, 3);
        if (choice == 1) type = "ѧ��";
        else if (choice == 2) type = "����";
        else if (choice == 3) type = "����";
        break;
    }
    std::string desc = getStringInput("���ż��: ");
    std::string leader = getStringInput("������: ");

    Club newClub(nextClubId++, name, type, desc, leader);
    clubs[newClub.getId()] = newClub;

    std::cout << "\n������ӳɹ�! ID: " << newClub.getId() << "\n";
    pressEnterToContinue();
}

void SystemManager::removeClub() {
    clearScreen();
    std::cout << "=== ɾ������ ===\n";
    displayAllClubs();

    if (clubs.empty()) {
        std::cout << "\nû�п�ɾ��������\n";
        pressEnterToContinue();
        return;
    }

    int id = getIntInput("����Ҫɾ��������ID: ", 1, nextClubId - 1);
    if (clubs.find(id) != clubs.end()) {
        // ɾ����ػ
        for (auto it = activities.begin(); it != activities.end();) {
            if (it->second.getClubId() == id) {
                it = activities.erase(it);
            }
            else {
                ++it;
            }
        }

        // ɾ���û�����
        for (auto& userPair : users) {
            if (userPair.second->getRole() == "user") {
                OrdinaryUser* user = dynamic_cast<OrdinaryUser*>(userPair.second);
                user->leaveClub(id);
            }
        }

        // ɾ������
        clubs.erase(id);
        std::cout << "����ɾ���ɹ�!\n";
    }
    else {
        std::cout << "δ�ҵ�������!\n";
    }
    pressEnterToContinue();
}

void SystemManager::registerMember() {
    clearScreen();
    std::cout << "=== ע���³�Ա ===\n";

    std::string sid = getStringInput("ѧ��: ");
    std::string name = getStringInput("����: ");
    std::string college = getStringInput("ѧԺ: ");
    std::string contact = getStringInput("��ϵ��ʽ: ");
    std::string username = getStringInput("�û���: ");

    // ����û����Ƿ��Ѵ���
    if (users.find(username) != users.end()) {
        std::cout << "�û����Ѵ���!\n";
        pressEnterToContinue();
        return;
    }

    std::string password;
    while (true) {
        password = getStringInput("����: ");
        std::string confirm = getStringInput("ȷ������: ");
        if (password == confirm) break;
        std::cout << "������������벻һ�£�����������!\n";
    }

    OrdinaryUser* newUser = new OrdinaryUser(sid, username, password, name, college, contact);
    users[username] = newUser;

    std::cout << "\n��Աע��ɹ�!\n";
    pressEnterToContinue();
}

void SystemManager::viewAllMembers() {
    clearScreen();
    std::cout << "=== ���г�Ա ===\n\n";
    for (const auto& pair : users) {
        if (pair.second->getRole() == "user") {
            OrdinaryUser* user = dynamic_cast<OrdinaryUser*>(pair.second);
            std::cout << "�û���: " << user->getUsername()
                << " | ѧ��: " << user->getStudentId()
                << " | ����: " << user->getName()
                << " | ѧԺ: " << user->getCollege()
                << " | ��ϵ��ʽ: " << user->getContact() << "\n";
        }
    }
    pressEnterToContinue();
}

void SystemManager::publishActivity() {
    clearScreen();
    std::cout << "=== �����» ===\n";
    displayAllClubs();

    if (clubs.empty()) {
        std::cout << "\n���ȴ�������!\n";
        pressEnterToContinue();
        return;
    }

    int clubId = getIntInput("ѡ������ID: ", 1, nextClubId - 1);
    if (clubs.find(clubId) == clubs.end()) {
        std::cout << "��Ч������ID!\n";
        pressEnterToContinue();
        return;
    }

    std::string name = getStringInput("�����: ");
    std::string type = getStringInput("�����: ");
    std::string location = getStringInput("��ص�: ");
    std::string desc = getStringInput("�˵��: ");

    // ��ȡ�ʱ��
    std::string timeStr;
    while (true) {
        timeStr = getStringInput("�ʱ�� (��ʽ: YYYY-MM-DD HH:MM): ");
        if (Utils::stringToTime(timeStr) != -1) break;
        std::cout << "ʱ���ʽ������ʹ�� YYYY-MM-DD HH:MM ��ʽ!\n";
    }
    time_t time = Utils::stringToTime(timeStr);

    Activity newActivity(nextActivityId++, clubId, name, time, location, type, desc);
    activities[newActivity.getId()] = newActivity;

    std::cout << "\n������ɹ�! ID: " << newActivity.getId() << "\n";
    pressEnterToContinue();
}

void SystemManager::displayStatistics() const {
    clearScreen();
    std::cout << "=== ����ͳ����Ϣ ===\n\n";

    // ������ͳ��
    std::map<std::string, int> typeCount;
    std::map<std::string, std::vector<const Activity*>> typeActivities;

    for (const auto& clubPair : clubs) {
        const Club& club = clubPair.second;
        typeCount[club.getType()]++;

        // �ռ����Ż
        for (const auto& actPair : activities) {
            if (actPair.second.getClubId() == club.getId()) {
                typeActivities[club.getType()].push_back(&actPair.second);
            }
        }
    }

    // ��ʾͳ����Ϣ
    std::cout << "�������ͷֲ�:\n";
    for (const auto& pair : typeCount) {
        std::cout << "  " << pair.first << "��: " << pair.second << "������\n";
    }

    std::cout << "\n�����ų�Ա����:\n";
    for (const auto& clubPair : clubs) {
        const Club& club = clubPair.second;
        std::cout << "  " << club.getName() << ": " << club.getMemberCount() << "��\n";
    }

    std::cout << "\n��ɫ�չʾ:\n";
    for (const auto& pair : typeActivities) {
        std::cout << "  " << pair.first << "��������ɫ�:\n";
        for (const Activity* act : pair.second) {
            std::cout << "    - " << act->getName() << " (" << Utils::timeToString(act->getTime()) << ")\n";
        }
    }

    std::cout << "\n�������ͳ��:\n";
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

// ��ͨ�û�����ʵ��
void SystemManager::joinClub() {
    auto user = getCurrentOrdinaryUser();
    if (!user) return;

    clearScreen();
    std::cout << "=== �������� ===\n";
    displayAllClubs();

    if (clubs.empty()) {
        std::cout << "\n��ǰû�пɼ��������\n";
        pressEnterToContinue();
        return;
    }

    if (user->getClubCount() >= 3) {
        std::cout << "\n���Ѽ���3�����ţ��޷��������!\n";
        pressEnterToContinue();
        return;
    }

    int clubId = getIntInput("����Ҫ���������ID: ", 1, nextClubId - 1);
    if (clubs.find(clubId) != clubs.end()) {
        try {
            if (user->joinClub(clubId)) {
                clubs[clubId].addMember(user->getStudentId());
                std::cout << "�ɹ���������!\n";
            }
            else {
                std::cout << "���Ѽ��������!\n";
            }
        }
        catch (const std::runtime_error& e) {
            std::cout << "����ʧ��: " << e.what() << "\n";
        }
    }
    else {
        std::cout << "δ�ҵ�������!\n";
    }
    pressEnterToContinue();
}

void SystemManager::leaveClub() {
    auto user = getCurrentOrdinaryUser();
    if (!user) return;

    clearScreen();
    std::cout << "=== �˳����� ===\n";
    viewMyClubs();

    const auto& joinedClubs = user->getJoinedClubs();
    if (joinedClubs.empty()) {
        std::cout << "\n����δ�����κ�����\n";
        pressEnterToContinue();
        return;
    }

    int clubId = getIntInput("����Ҫ�˳�������ID: ", 1, nextClubId - 1);
    if (std::find(joinedClubs.begin(), joinedClubs.end(), clubId) != joinedClubs.end()) {
        if (clubs.find(clubId) != clubs.end()) {
            clubs[clubId].removeMember(user->getStudentId());
            user->leaveClub(clubId);
            std::cout << "�ɹ��˳�����!\n";
        }
    }
    else {
        std::cout << "��δ���������!\n";
    }
    pressEnterToContinue();
}

void SystemManager::participateActivity() {
    auto user = getCurrentOrdinaryUser();
    if (!user) return;

    clearScreen();
    std::cout << "=== �μӻ ===\n";
    displayAllActivities();

    if (activities.empty()) {
        std::cout << "\n��ǰû�пɲμӵĻ\n";
        pressEnterToContinue();
        return;
    }

    int activityId = getIntInput("����Ҫ�μӵĻID: ", 1, nextActivityId - 1);
    if (activities.find(activityId) != activities.end()) {
        if (user->participateActivity(activityId)) {
            activities[activityId].addParticipant(user->getStudentId());
            std::cout << "�ɹ��μӻ!\n";
        }
        else {
            std::cout << "���ѲμӸû!\n";
        }
    }
    else {
        std::cout << "δ�ҵ��û!\n";
    }
    pressEnterToContinue();
}

void SystemManager::submitFeedback() {
    auto user = getCurrentOrdinaryUser();
    if (!user) return;

    clearScreen();
    std::cout << "=== �ύ����� ===\n";
    viewMyActivities();

    const auto& participatedActivities = user->getParticipatedActivities();
    if (participatedActivities.empty()) {
        std::cout << "\n����δ�μ��κλ\n";
        pressEnterToContinue();
        return;
    }

    int activityId = getIntInput("����Ҫ�����ĻID: ", 1, nextActivityId - 1);
    if (activities.find(activityId) != activities.end()) {
        Activity& activity = activities[activityId];

        // ����Ƿ�μ��˸û
        if (std::find(participatedActivities.begin(), participatedActivities.end(), activityId) == participatedActivities.end()) {
            std::cout << "��δ�μӸû!\n";
            pressEnterToContinue();
            return;
        }

        // ����Ƿ��ѽ���
        if (activity.getTime() > time(nullptr)) {
            std::cout << "���δ�������޷��ύ����!\n";
            pressEnterToContinue();
            return;
        }

        // ����Ƿ����ύ����
        if (activity.getFeedback(user->getStudentId())) {
            std::cout << "�����ύ���û�ķ���!\n";
            pressEnterToContinue();
            return;
        }

        int rating = getIntInput("���� (1-5��): ", 1, 5);
        std::string comment = getStringInput("�������: ");

        if (activity.submitFeedback(user->getStudentId(), rating, comment)) {
            std::cout << "�����ύ�ɹ�!\n";
        }
        else {
            std::cout << "�����ύʧ��!\n";
        }
    }
    else {
        std::cout << "δ�ҵ��û!\n";
    }
    pressEnterToContinue();
}

void SystemManager::viewMyClubs() const {
    auto user = getCurrentOrdinaryUser();
    if (!user) return;

    clearScreen();
    std::cout << "=== �Ҽ�������� ===\n";

    const auto& clubIds = user->getJoinedClubs();
    if (clubIds.empty()) {
        std::cout << "\n����δ�����κ�����\n";
        pressEnterToContinue();
        return;
    }

    std::cout << "�Ѽ������� (" << clubIds.size() << "/3):\n";
    for (int id : clubIds) {
        if (clubs.find(id) != clubs.end()) {
            const Club& club = clubs.at(id);
            std::cout << "ID: " << id << " | ����: " << club.getName()
                << " | ����: " << club.getType() << "\n";
        }
    }

    pressEnterToContinue();
}

void SystemManager::viewMyActivities() const {
    auto user = getCurrentOrdinaryUser();
    if (!user) return;

    clearScreen();
    std::cout << "=== �ҲμӵĻ ===\n";

    const auto& activityIds = user->getParticipatedActivities();
    if (activityIds.empty()) {
        std::cout << "\n����δ�μ��κλ\n";
        pressEnterToContinue();
        return;
    }

    time_t now = time(nullptr);
    std::cout << "�Ѳμӻ:\n";
    for (int id : activityIds) {
        if (activities.find(id) != activities.end()) {
            const Activity& activity = activities.at(id);
            std::string status = (activity.getTime() > now) ? "���μ�" : "�����";
            std::cout << "ID: " << id << " | �: " << activity.getName()
                << " | ʱ��: " << Utils::timeToString(activity.getTime())
                << " | ״̬: " << status << "\n";
        }
    }

    pressEnterToContinue();
}

// ��������ʵ��

void SystemManager::displayAllClubs() const {
    if (clubs.empty()) {
        std::cout << "����������Ϣ\n";
        return;
    }

    std::cout << "�����б�:\n";
    for (const auto& pair : clubs) {
        const Club& club = pair.second;
        std::cout << "ID: " << club.getId() << " | ����: " << club.getName()
            << " | ����: " << club.getType() << " | ��Ա��: "
            << club.getMemberCount() << "\n";
    }
}

void SystemManager::viewAllClubs() const {
    clearScreen();
    std::cout << "=== �������� ===\n\n";
    displayAllClubs();
    pressEnterToContinue();
}

void SystemManager::displayAllActivities() const {
    if (activities.empty()) {
        std::cout << "���޻��Ϣ\n";
        return;
    }

    std::cout << "��б�:\n";
    for (const auto& pair : activities) {
        const Activity& activity = pair.second;
        std::string clubName = "δ֪����";
        if (clubs.find(activity.getClubId()) != clubs.end()) {
            clubName = clubs.at(activity.getClubId()).getName();
        }

        std::cout << "ID: " << activity.getId() << " | �: " << activity.getName()
            << " | ����: " << clubName << " | ʱ��: "
            << Utils::timeToString(activity.getTime()) << "\n";
    }
}

void SystemManager::viewAllActivities() const {
    clearScreen();
    std::cout << "=== ���л ===\n\n";
    displayAllActivities();
    pressEnterToContinue();
}

void SystemManager::viewClubDetails() const {
    clearScreen();
    std::cout << "=== �鿴�������� ===\n";
    displayAllClubs();

    if (clubs.empty()) {
        pressEnterToContinue();
        return;
    }

    int clubId = getIntInput("\n����Ҫ�鿴������ID: ", 1, nextClubId - 1);
    if (clubs.find(clubId) != clubs.end()) {
        clearScreen();
        const Club& club = clubs.at(clubId);
        club.displayInfo();

        // ��ʾ���Ż
        std::cout << "\n���Ż:\n";
        bool hasActivities = false;
        for (const auto& actPair : activities) {
            if (actPair.second.getClubId() == clubId) {
                hasActivities = true;
                std::cout << "  - " << actPair.second.getName()
                    << " (" << Utils::timeToString(actPair.second.getTime()) << ")\n";
            }
        }

        if (!hasActivities) {
            std::cout << "  ���޻\n";
        }

        // ��ʾ���ų�Ա
        std::cout << "\n���ų�Ա (" << club.getMemberCount() << "��):\n";
        const auto& memberIds = club.getMemberIds();
        for (const auto& sid : memberIds) {
            // ���ҳ�Ա����
            std::string memberName = "δ֪";
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
        std::cout << "δ�ҵ�������!\n";
    }
    pressEnterToContinue();
}

void SystemManager::viewActivityDetails() const {
    clearScreen();
    std::cout << "=== �鿴����� ===\n";
    displayAllActivities();

    if (activities.empty()) {
        pressEnterToContinue();
        return;
    }

    int activityId = getIntInput("\n����Ҫ�鿴�ĻID: ", 1, nextActivityId - 1);
    if (activities.find(activityId) != activities.end()) {
        clearScreen();
        const Activity& activity = activities.at(activityId);
        activity.displayInfo();

        // ��ʾ��������
        std::string clubName = "δ֪����";
        if (clubs.find(activity.getClubId()) != clubs.end()) {
            clubName = clubs.at(activity.getClubId()).getName();
        }
        std::cout << "��������: " << clubName << "\n";

        // ��ʾ�������
        std::cout << "�������: ";
        if (activity.getParticipantIds().empty()) {
            std::cout << "���޲�����\n";
        }
        else {
            std::cout << activity.getParticipantIds().size() << "�˲���\n";
        }

        // ��ʾ����
        std::cout << "\n�����:\n";
        bool hasFeedback = false;
        for (const auto& sid : activity.getParticipantIds()) {
            const auto* feedback = activity.getFeedback(sid);
            if (feedback) {
                hasFeedback = true;
                // ���ҳ�Ա����
                std::string memberName = "δ֪";
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
                std::cout << "    ����: " << feedback->rating << "��\n";
                std::cout << "    ����: " << feedback->comment << "\n\n";
            }
        }

        if (!hasFeedback) {
            std::cout << "  ���޷���\n";
        }
    }
    else {
        std::cout << "δ�ҵ��û!\n";
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


// �˵�ϵͳʵ��
void SystemManager::showMainMenu() {
    clearScreen();
    std::cout << "=== ��У���Ź���ϵͳ ===\n\n";
    std::cout << "1. ��¼\n";
    std::cout << "2. �˳�ϵͳ\n";
}

void SystemManager::showAdminMenu() {
    while (currentUser) {
        clearScreen();
        std::cout << "=== ����Ա�˵� (" << currentUser->getUsername() << ") ===\n\n";
        std::cout << "1. ���Ź���\n";
        std::cout << "2. ��Ա����\n";
        std::cout << "3. �����\n";
        std::cout << "4. ͳ��չʾ\n";
        std::cout << "5. �˳���¼\n";

        int choice = getIntInput("��ѡ��: ", 1, 5);

        switch (choice) {
        case 1: // ���Ź���
            clearScreen();
            std::cout << "=== ���Ź��� ===\n\n";
            std::cout << "1. �������\n";
            std::cout << "2. ɾ������\n";
            std::cout << "3. �鿴��������\n";
            std::cout << "4. �鿴��������\n";
            std::cout << "5. �����ϼ��˵�\n";

            switch (getIntInput("��ѡ��: ", 1, 5)) {
            case 1: addClub(); break;
            case 2: removeClub(); break;
            case 3: viewAllClubs(); break;
            case 4: viewClubDetails(); break;
            case 5: break; // ����
            }
            break;

        case 2: // ��Ա����
            clearScreen();
            std::cout << "=== ��Ա���� ===\n\n";
            std::cout << "1. ע���³�Ա\n";
            std::cout << "2. �鿴���г�Ա\n";
            std::cout << "3. �����ϼ��˵�\n";

            switch (getIntInput("��ѡ��: ", 1, 3)) {
            case 1: registerMember(); break;
            case 2: viewAllMembers(); break;
            case 3: break; // ����
            }
            break;

        case 3: // �����
            clearScreen();
            std::cout << "=== ����� ===\n\n";
            std::cout << "1. �����»\n";
            std::cout << "2. �鿴���л\n";
            std::cout << "3. �鿴�����\n";
            std::cout << "4. �����ϼ��˵�\n";

            switch (getIntInput("��ѡ��: ", 1, 4)) {
            case 1: publishActivity(); break;
            case 2: viewAllActivities(); break;
            case 3: viewActivityDetails(); break;
            case 4: break; // ����
            }
            break;

        case 4: // ͳ��չʾ
            displayStatistics();
            break;

        case 5: // �˳���¼
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
        std::cout << "=== �û��˵� (" << user->getName() << ") ===\n\n";
        std::cout << "1. �鿴������Ϣ\n";
        std::cout << "2. �����ҵ�����\n";
        std::cout << "3. �鿴���Ϣ\n";
        std::cout << "4. �����ҵĻ\n";
        std::cout << "5. �ύ�����\n";
        std::cout << "6. �˳���¼\n";

        int choice = getIntInput("��ѡ��: ", 1, 6);

        switch (choice) {
        case 1: // �鿴������Ϣ
            clearScreen();
            std::cout << "=== ������Ϣ ===\n\n";
            std::cout << "1. �鿴��������\n";
            std::cout << "2. ������ɸѡ����\n";
            std::cout << "3. �鿴��������\n";
            std::cout << "4. �����ϼ��˵�\n";

            switch (getIntInput("��ѡ��: ", 1, 4)) {
            case 1: viewAllClubs(); break;
            case 2:
                clearScreen();
                std::cout << "=== ������ɸѡ���� ===\n\n";
                std::cout << "1. ѧ����\n";
                std::cout << "2. ������\n";
                std::cout << "3. ������\n";
                std::cout << "4. ����\n";

                switch (getIntInput("��ѡ��: ", 1, 4)) {
                case 1:
                    clearScreen();
                    std::cout << "=== ѧ�������� ===\n\n";
                    for (const auto& pair : clubs) {
                        if (pair.second.getType() == "ѧ��") {
                            pair.second.displayInfo();
                            std::cout << "------------------------\n";
                        }
                    }
                    pressEnterToContinue();
                    break;
                case 2:
                    clearScreen();
                    std::cout << "=== ���������� ===\n\n";
                    for (const auto& pair : clubs) {
                        if (pair.second.getType() == "����") {
                            pair.second.displayInfo();
                            std::cout << "------------------------\n";
                        }
                    }
                    pressEnterToContinue();
                    break;
                case 3:
                    clearScreen();
                    std::cout << "=== ���������� ===\n\n";
                    for (const auto& pair : clubs) {
                        if (pair.second.getType() == "����") {
                            pair.second.displayInfo();
                            std::cout << "------------------------\n";
                        }
                    }
                    pressEnterToContinue();
                    break;
                case 4: break; // ����
                }
                break;
            case 3: viewClubDetails(); break;
            case 4: break; // ����
            }
            break;

        case 2: // �����ҵ�����
            clearScreen();
            std::cout << "=== �ҵ����Ź��� ===\n\n";
            std::cout << "1. ��������\n";
            std::cout << "2. �˳�����\n";
            std::cout << "3. �鿴�ҵ�����\n";
            std::cout << "4. �����ϼ��˵�\n";

            switch (getIntInput("��ѡ��: ", 1, 4)) {
            case 1: joinClub(); break;
            case 2: leaveClub(); break;
            case 3: viewMyClubs(); break;
            case 4: break; // ����
            }
            break;

        case 3: // �鿴���Ϣ
            clearScreen();
            std::cout << "=== ���Ϣ ===\n\n";
            std::cout << "1. �鿴���л\n";
            std::cout << "2. ��ʱ������\n";
            std::cout << "3. ������ɸѡ�\n";
            std::cout << "4. �鿴�����\n";
            std::cout << "5. �����ϼ��˵�\n";

            switch (getIntInput("��ѡ��: ", 1, 5)) {
            case 1:
                clearScreen();
                std::cout << "=== ���л ===\n\n";
                displayAllActivities();
                pressEnterToContinue();
                break;
            case 2:
                clearScreen();
                std::cout << "=== ��ʱ������ ===\n\n";
                {
                    // �����ָ���б�
                    std::vector<const Activity*> sortedActivities;
                    for (const auto& pair : activities) {
                        sortedActivities.push_back(&pair.second);
                    }

                    // ��ʱ������
                    std::sort(sortedActivities.begin(), sortedActivities.end(),
                        [](const Activity* a, const Activity* b) {
                            return a->getTime() < b->getTime();
                        });

                    // ��ʾ���
                    for (const Activity* act : sortedActivities) {
                        std::string clubName = "δ֪����";
                        if (clubs.find(act->getClubId()) != clubs.end()) {
                            clubName = clubs.at(act->getClubId()).getName();
                        }
                        std::cout << "ʱ��: " << Utils::timeToString(act->getTime())
                            << " | �: " << act->getName()
                            << " | ����: " << clubName << "\n";
                    }
                }
                pressEnterToContinue();
                break;
            case 3:
                clearScreen();
                std::cout << "=== ������ɸѡ� ===\n\n";
                displayAllClubs();
                if (!clubs.empty()) {
                    int clubId = getIntInput("\n��������ID: ", 1, nextClubId - 1);
                    if (clubs.find(clubId) != clubs.end()) {
                        clearScreen();
                        std::cout << "=== " << clubs.at(clubId).getName() << " �Ļ ===\n\n";
                        for (const auto& pair : activities) {
                            if (pair.second.getClubId() == clubId) {
                                std::cout << "�: " << pair.second.getName()
                                    << " | ʱ��: " << Utils::timeToString(pair.second.getTime())
                                    << " | �ص�: " << pair.second.getLocation() << "\n";
                            }
                        }
                    }
                    else {
                        std::cout << "δ�ҵ�������!\n";
                    }
                }
                pressEnterToContinue();
                break;
            case 4: viewActivityDetails(); break;
            case 5: break; // ����
            }
            break;

        case 4: // �����ҵĻ
            clearScreen();
            std::cout << "=== �ҵĻ���� ===\n\n";
            std::cout << "1. �μӻ\n";
            std::cout << "2. �鿴�ҵĻ\n";
            std::cout << "3. �����ϼ��˵�\n";

            switch (getIntInput("��ѡ��: ", 1, 3)) {
            case 1: participateActivity(); break;
            case 2: viewMyActivities(); break;
            case 3: break; // ����
            }
            break;

        case 5: // �ύ�����
            submitFeedback();
            break;

        case 6: // �˳���¼
            logout();
            return;
        }
    }
}

