#ifndef ACTIVITY_H
#define ACTIVITY_H

#include <string>
#include <vector>
#include <ctime>
#include <map>

class Activity {
public:
    struct Feedback {
        int rating; // 1-5
        std::string comment;
    };

private:
    int id;
    int clubId; // 发起活动的社团
    std::string name;
    time_t time;
    std::string location;
    std::string type;
    std::string description;
    std::vector<std::string> participantIds; // 参与者学号
    std::map<std::string, Feedback> feedbacks; // 学号到反馈的映射

public:
    Activity();
    Activity(int id, int cid, const std::string& n, time_t t,
        const std::string& loc, const std::string& type,
        const std::string& desc);
    ~Activity();

    bool addParticipant(const std::string& studentId);
    bool hasParticipant(const std::string& studentId) const;
    bool submitFeedback(const std::string& studentId, int rating, const std::string& comment);
    double participationRate(int totalMembers) const;

    int getId() const;
    int getClubId() const;
    std::string getName() const;
    time_t getTime() const;
    std::string getLocation() const;
    std::string getType() const;
    std::string getDescription() const;
    const std::vector<std::string>& getParticipantIds() const;
    const Feedback* getFeedback(const std::string& studentId) const;

    void displayInfo() const;
    void displayParticipants() const;
};

#endif // ACTIVITY_H

