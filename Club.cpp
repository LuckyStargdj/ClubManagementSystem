#include "Club.h"
#include "Utils.h"
#include <iostream>
#include <algorithm>

Club::Club() {}

Club::Club(int id, const std::string& n, const std::string& t,
    const std::string& desc, const std::string& l)
    : id(id), name(n), type(t), description(desc), leader(l) {
    createTime = time(nullptr);
}

Club::~Club() {}

bool Club::addMember(const std::string& studentId) {
    if (std::find(memberIds.begin(), memberIds.end(), studentId) != memberIds.end()) {
        return false; // 已存在
    }
    memberIds.push_back(studentId);
    return true;
}

bool Club::removeMember(const std::string& studentId) {
    auto it = std::find(memberIds.begin(), memberIds.end(), studentId);
    if (it != memberIds.end()) {
        memberIds.erase(it);
        return true;
    }
    return false;
}

bool Club::hasMember(const std::string& studentId) const {
    return std::find(memberIds.begin(), memberIds.end(), studentId) != memberIds.end();
}

int Club::getId() const { 
    return id; 
}

void Club::setId(int new_id) {
    id = new_id;
}

std::string Club::getName() const { 
    return name; 
}

void Club::setName(const std::string& new_name) {
    name = new_name;
}

std::string Club::getType() const { 
    return type; 
}

void Club::setType(const std::string& new_type) {
    type = new_type;
}

std::string Club::getDescription() const { 
    return description; 
}

void Club::setDescription(const std::string& new_description) {
    description = new_description;
}

time_t Club::getCreateTime() const { 
    return createTime; 
}

void Club::setCreateTime(time_t time) {
    createTime = time;
}

std::string Club::getLeader() const { 
    return leader; 
}

void Club::setLeader(const std::string& new_leader) {
    leader = new_leader;
}

int Club::getMemberCount() const { return static_cast<int>(memberIds.size()); }
const std::vector<std::string>& Club::getMemberIds() const { return memberIds; }


void Club::displayInfo() const {
    std::cout << "社团ID: " << id << "\n";
    std::cout << "名称: " << name << " (" << type << "类)\n";
    std::cout << "创建时间: " << Utils::timeToString(createTime) << "\n";
    std::cout << "负责人: " << leader << "\n";
    std::cout << "简介: " << description << "\n";
    std::cout << "成员数量: " << memberIds.size() << "\n";
}
