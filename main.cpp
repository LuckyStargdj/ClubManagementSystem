#include "SystemManager.h"
#include <iostream>

int main() {
    SystemManager system;

    // ����Ƿ����й���Ա�˺�
    bool adminExists = false;
    for (const auto& userPair : system.getUsers()) {
        if (userPair.second->getRole() == "admin") {
            adminExists = true;
            break;
        }
    }

    // ���û�й���Ա�˺ţ�����Ĭ�Ϲ���Ա
    if (!adminExists) {
        Admin* admin = new Admin("admin", "admin123");
        system.addUser("admin", admin);
        system.saveData(); // ��������
    }

    while (true) {
        system.showMainMenu();
        int choice = system.getIntInput("��ѡ��: ", 1, 2);

        switch (choice) {
        case 1: { // ��¼
            std::string username, password;
            std::cout << "�û���: ";
            std::cin >> username;
            std::cout << "����: ";
            std::cin >> password;

            if (system.login(username, password)) {
                if (system.getCurrentUserRole() == "admin") {
                    system.showAdminMenu();
                }
                else {
                    system.showUserMenu();
                }
            }
            else {
                std::cout << "��¼ʧ��! �û������������\n";
                std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
                system.pressEnterToContinue();
            }
            break;
        }
        case 2: // �˳�ϵͳ
            return 0;
        }
    }
}