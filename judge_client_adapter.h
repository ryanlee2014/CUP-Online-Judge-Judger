#pragma once

#include <string>

class ISubmissionAdapter {
public:
    virtual ~ISubmissionAdapter() = default;

    virtual void getCustomInput(int solutionId, char *work_dir) = 0;
    virtual void getProblemInfo(int p_id, double &time_lmt, int &mem_lmt, int &isspj) = 0;
    virtual void getSolution(int solution_id, char *work_dir, int lang, char *usercode, const char *suffix,
                             int debug) = 0;
    virtual void getSolutionInfo(int solutionId, int &p_id, char *userId, int &lang) = 0;

    virtual ISubmissionAdapter &setPort(int port) = 0;
    virtual ISubmissionAdapter &setDBName(const std::string &db) = 0;
    virtual ISubmissionAdapter &setUserName(const std::string &user_name) = 0;
    virtual ISubmissionAdapter &setPassword(const std::string &password) = 0;
    virtual ISubmissionAdapter &setHostName(const std::string &host_name) = 0;

    virtual bool isConnected() = 0;
    virtual bool start() = 0;
};

class MySQLSubmissionAdapter;

class MySQLSubmissionAdapterWrapper final : public ISubmissionAdapter {
public:
    explicit MySQLSubmissionAdapterWrapper(MySQLSubmissionAdapter *adapter);

    void getCustomInput(int solutionId, char *work_dir) override;
    void getProblemInfo(int p_id, double &time_lmt, int &mem_lmt, int &isspj) override;
    void getSolution(int solution_id, char *work_dir, int lang, char *usercode, const char *suffix,
                     int debug) override;
    void getSolutionInfo(int solutionId, int &p_id, char *userId, int &lang) override;

    ISubmissionAdapter &setPort(int port) override;
    ISubmissionAdapter &setDBName(const std::string &db) override;
    ISubmissionAdapter &setUserName(const std::string &user_name) override;
    ISubmissionAdapter &setPassword(const std::string &password) override;
    ISubmissionAdapter &setHostName(const std::string &host_name) override;

    bool isConnected() override;
    bool start() override;

private:
    MySQLSubmissionAdapter *adapter_;
};
