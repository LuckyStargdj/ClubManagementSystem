#include "SystemManager.h"
#include <iostream>

int main() {
    SystemManager system;

    // 检查是否已有管理员账号
    bool adminExists = false;
    for (const auto& userPair : system.getUsers()) {
        if (userPair.second->getRole() == "admin") {
            adminExists = true;
            break;
        }
    }

    // 如果没有管理员账号，创建默认管理员
    if (!adminExists) {
        Admin* admin = new Admin("admin", "admin123");
        system.addUser("admin", admin);
        system.saveData(); // 立即保存
    }

    while (true) {
        system.showMainMenu();
        int choice = system.getIntInput("请选择: ", 1, 2);

        switch (choice) {
        case 1: { // 登录
            std::string username, password;
            std::cout << "用户名: ";
            std::cin >> username;
            std::cout << "密码: ";
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
                std::cout << "登录失败! 用户名或密码错误\n";
                std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
                system.pressEnterToContinue();
            }
            break;
        }
        case 2: // 退出系统
            return 0;
        }
    }
}