#ifndef USER_H
#define USER_H

#include <string>
#include <vector>
#include <stdexcept>

class User {
protected:
    std::string username;
    std::string password;
    std::string role; // "admin" or "user"

public:
    User(const std::string& uname, const std::string& pwd, const std::string& r);
    virtual ~User() = default;

    bool authenticate(const std::string& pwd) const;

    std::string getUsername() const;
    std::string getRole() const;
    std::string getPassword() const;

    virtual void displayInfo() const = 0;
};

class Admin : public User {
public:
    Admin(const std::string& uname, const std::string& pwd);
    void displayInfo() const override;
};

class OrdinaryUser : public User {
private:
    std::string studentId;
    std::string name;
    std::string college;
    std::string contact;
    std::vector<int> joinedClubs; // 加入的社团ID
    std::vector<int> participatedActivities; // 参与的活动ID

public:
    OrdinaryUser(const std::string& sid, const std::string& uname,
        const std::string& pwd, const std::string& n,
        const std::string& col, const std::string& ct);

    void updateContact(const std::string& newContact);
    bool joinClub(int clubId);
    bool leaveClub(int clubId);
    bool participateActivity(int activityId);

    std::string getStudentId() const;
    std::string getName() const;
    std::string getCollege() const;
    std::string getContact() const;
    const std::vector<int>& getJoinedClubs() const;
    const std::vector<int>& getParticipatedActivities() const;
    int getClubCount() const;

    void displayInfo() const override;
};

#endif // USER_H

