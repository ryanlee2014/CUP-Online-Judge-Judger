//
// Created by ryan on 19-4-11.
//

#include "MySQLAutoPointer.h"
#include "../library/judge_lib.h"
MySQLAutoPointer::~MySQLAutoPointer() {
    if(connected) {
        mysql_close(conn);
    }
}
MySQLAutoPointer::operator MYSQL*() {
    return conn;
}

bool MySQLAutoPointer::start() {
    conn = mysql_init(nullptr);
    //mysql_real_connect(conn,host_name,user_name,password,db_name,port_number,0,0);
    const char timeout = 30;
    mysql_options(conn, MYSQL_OPT_CONNECT_TIMEOUT, &timeout);

    if (!mysql_real_connect(conn, host_name.c_str(), user_name.c_str(), password.c_str(), db_name.c_str(),
                            static_cast<unsigned int>(port_number), 0, 0)) {
        write_log("%s", mysql_error(conn));
        if (DEBUG) {
            cerr << "MYSQL daemon service is down." << endl;
            cerr << "MYSQL error log: " << mysql_error(conn) << endl;
        }
        return false;
    }
    const char *utf8sql = "set names utf8";
    if (mysql_real_query(conn, utf8sql, strlen(utf8sql))) {
        write_log("%s", mysql_error(conn));
        if (DEBUG) {
            cerr << "MYSQL daemon service is down." << endl;
            cerr << "MYSQL error log: " << mysql_error(conn) << endl;
        }
        return false;
    }
    connected = true;
    return true;
}

bool MySQLAutoPointer::setDebugMode(bool state) {
    DEBUG = state;
    return DEBUG;
}

bool MySQLAutoPointer::setDBName(string db) {
    if(db.empty()) {
        return false;
    }
    db_name = db;
    return true;
}

bool MySQLAutoPointer::setHostName(string h) {
    if(h.empty()) {
        return false;
    }
    host_name = h;
    return true;
}

bool MySQLAutoPointer::setPassword(string p) {
    if(p.empty()) {
        return false;
    }
    password = p;
    return true;
}

bool MySQLAutoPointer::setUserName(string u) {
    if(u.empty()) {
        return false;
    }
    user_name = u;
    return true;
}

bool MySQLAutoPointer::setPort(int p) {
    if(p == 0) {
        return false;
    }
    port_number = p;
    return true;
}

bool MySQLAutoPointer::isConnected() {
    return connected;
}