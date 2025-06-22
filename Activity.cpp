#include "Activity.h"
#include "Utils.h"
#include <iostream>
#include <algorithm>

Activity::Activity() {}

Activity::Activity(int id, int cid, const std::string& n, time_t t,
    const std::string& loc, const std::string& type,
    const std::string& desc)
    : id(id), clubId(cid), name(n), time(t), location(loc), type(type), description(desc) {}

Activity::~Activity() {}

bool Activity::addParticipant(const std::string& studentId) {
    if (std::find(participantIds.begin(), participantIds.end(), studentId) != participantIds.end()) {
        return false; // 已存在
    }
    participantIds.push_back(studentId);
    return true;
}

bool Activity::hasParticipant(const std::string& studentId) const {
    return std::find(participantIds.begin(), participantIds.end(), studentId) != participantIds.end();
}

bool Activity::submitFeedback(const std::string& studentId, int rating, const std::string& comment) {
    if (rating < 1 || rating > 5) {
        throw std::invalid_argument("评分必须在1-5之间");
    }

    if (!hasParticipant(studentId)) {
        return false;
    }

    if (feedbacks.find(studentId) != feedbacks.end()) {
        return false; // 已提交反馈
    }

    feedbacks[studentId] = Feedback{ rating, comment };
    return true;
}

double Activity::participationRate(int totalMembers) const {
    if (totalMembers == 0) return 0.0;
    return (static_cast<double>(participantIds.size()) / totalMembers) * 100;
}

int Activity::getId() const { return id; }
int Activity::getClubId() const { return clubId; }
std::string Activity::getName() const { return name; }
time_t Activity::getTime() const { return time; }
std::string Activity::getLocation() const { return location; }
std::string Activity::getType() const { return type; }
std::string Activity::getDescription() const { return description; }
const std::vector<std::string>& Activity::getParticipantIds() const { return participantIds; }

const Activity::Feedback* Activity::getFeedback(const std::string& studentId) const {
    auto it = feedbacks.find(studentId);
    return (it != feedbacks.end()) ? &it->second : nullptr;
}

void Activity::displayInfo() const {
    std::cout << "活动ID: " << id << "\n";
    std::cout << "名称: " << name << " (" << type << ")\n";
    std::cout << "时间: " << Utils::timeToString(time) << "\n";
    std::cout << "地点: " << location << "\n";
    std::cout << "描述: " << description << "\n";
    std::cout << "参与人数: " << participantIds.size() << "\n";
}

void Activity::displayParticipants() const {
    std::cout << "参与者列表 (" << participantIds.size() << "人):\n";
    for (const auto& id : participantIds) {
        std::cout << "- " << id << "\n";
    }
}
