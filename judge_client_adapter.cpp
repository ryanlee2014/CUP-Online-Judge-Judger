#include "judge_client_adapter.h"

#include "external/mysql/MySQLSubmissionAdapter.h"

MySQLSubmissionAdapterWrapper::MySQLSubmissionAdapterWrapper(MySQLSubmissionAdapter *adapter)
    : adapter_(adapter) {}

void MySQLSubmissionAdapterWrapper::getCustomInput(int solutionId, char *work_dir) {
    adapter_->getCustomInput(solutionId, work_dir);
}

void MySQLSubmissionAdapterWrapper::getProblemInfo(int p_id, double &time_lmt, int &mem_lmt, int &isspj) {
    adapter_->getProblemInfo(p_id, time_lmt, mem_lmt, isspj);
}

void MySQLSubmissionAdapterWrapper::getSolution(int solution_id, char *work_dir, int lang, char *usercode,
                                                const char *suffix, int debug) {
    adapter_->getSolution(solution_id, work_dir, lang, usercode, suffix, debug);
}

void MySQLSubmissionAdapterWrapper::getSolutionInfo(int solutionId, int &p_id, char *userId, int &lang) {
    adapter_->getSolutionInfo(solutionId, p_id, userId, lang);
}

ISubmissionAdapter &MySQLSubmissionAdapterWrapper::setPort(int port) {
    adapter_->setPort(port);
    return *this;
}

ISubmissionAdapter &MySQLSubmissionAdapterWrapper::setDBName(const std::string &db) {
    adapter_->setDBName(db);
    return *this;
}

ISubmissionAdapter &MySQLSubmissionAdapterWrapper::setUserName(const std::string &user_name) {
    adapter_->setUserName(user_name);
    return *this;
}

ISubmissionAdapter &MySQLSubmissionAdapterWrapper::setPassword(const std::string &password) {
    adapter_->setPassword(password);
    return *this;
}

ISubmissionAdapter &MySQLSubmissionAdapterWrapper::setHostName(const std::string &host_name) {
    adapter_->setHostName(host_name);
    return *this;
}

bool MySQLSubmissionAdapterWrapper::isConnected() {
    return adapter_->isConnected();
}

bool MySQLSubmissionAdapterWrapper::start() {
    return adapter_->start();
}
