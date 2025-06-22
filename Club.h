#ifndef CLUB_H
#define CLUB_H

#include <string>
#include <vector>
#include <ctime>

class Club {
private:
    int id;
    std::string name;
    std::string type; // 学术/文体/公益
    std::string description;
    time_t createTime;
    std::string leader;
    std::vector<std::string> memberIds; // 成员学号

public:
    Club();
    Club(int id, const std::string& n, const std::string& t,
        const std::string& desc, const std::string& l);
    ~Club();
    bool addMember(const std::string& studentId);
    bool removeMember(const std::string& studentId);
    bool hasMember(const std::string& studentId) const;

    int getId() const;
    void setId(int new_id);
    std::string getName() const;
    void setName(const std::string& new_name);
    std::string getType() const;
    void setType(const std::string& new_type);
    std::string getDescription() const;
    void setDescription(const std::string& new_description);
    time_t getCreateTime() const;
    void setCreateTime(time_t time);
    std::string getLeader() const;
    void setLeader(const std::string& new_leader);
    int getMemberCount() const;
    const std::vector<std::string>& getMemberIds() const;

    void displayInfo() const;
};

#endif // CLUB_H
