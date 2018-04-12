//
// Created by admin on 2017/10/18.
//

//
#define IGNORE_ESOL
// File:   main.cc
// Author: sempr
// refacted by zhblue
/*
 * Copyright 2008 sempr <iamsempr@gmail.com>
 *
 * Refacted and modified by zhblue<newsclan@gmail.com>
 * Bug report email newsclan@gmail.com
 *
 *
 * This file is part of HUSTOJ.
 *
 * HUSTOJ is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * HUSTOJ is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with HUSTOJ. if not, see <http://www.gnu.org/licenses/>.
 */

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <dirent.h>
#include <unistd.h>
#include <cstdarg>
#include <cctype>
#include <sys/wait.h>
#include <sys/ptrace.h>
#include <sys/user.h>
#include <sys/resource.h>
//#include <sys/types.h>
#include <sys/stat.h>
#include <mysql/mysql.h>
#include "okcalls.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include "websocket.h"

using namespace std;
using json = nlohmann::json;
//
#define STD_MB (int)(1048576*1.5)
//#define STD_T_LIM 2
#define STD_F_LIM (STD_MB<<5)
//#define STD_M_LIM (STD_MB<<7)
#define BUFFER_SIZE 5120


/*copy from ZOJ
 http://code.google.com/p/zoj/source/browse/trunk/judge_client/client/tracer.cc?spec=svn367&r=367#39
 */
#ifdef __i386
#define REG_SYSCALL orig_eax
#define REG_RET eax
#define REG_ARG0 ebx
#define REG_ARG1 ecx
#else
#define REG_SYSCALL orig_rax
#define REG_RET rax
#define REG_ARG0 rdi
#define REG_ARG1 rsi

#endif

static int DEBUG = 0;
static char host_name[BUFFER_SIZE];
static char user_name[BUFFER_SIZE];
static char password[BUFFER_SIZE];
static char db_name[BUFFER_SIZE];
static char oj_home[BUFFER_SIZE];
static char data_list[BUFFER_SIZE][BUFFER_SIZE];
static int data_list_len = 0;

static int port_number;
static int max_running;
static int sleep_time;
static int java_time_bonus = 5;
static int java_memory_bonus = 512;
static char java_xms[BUFFER_SIZE];
static char java_xmx[BUFFER_SIZE];
static int sim_enable = 0;
static int oi_mode = 0;
static int full_diff = 0;
static int use_max_time = 0;

static int http_judge = 0;
static char http_baseurl[BUFFER_SIZE];

static char http_username[BUFFER_SIZE];
static char http_password[BUFFER_SIZE];
static int shm_run = 0;

static char record_call = 0;
static int use_ptrace = 1;
static int judger_number = 0;
int solution_id;
//static int sleep_tmp;
#define ZOJ_COM
MYSQL *conn;

websocket webSocket;
char *global_work_dir;
static char lang_ext[22][8] = {"c", "cc", "pas", "java", "rb", "sh", "py",
                               "php", "pl", "cs", "m", "bas", "scm", "c", "cc", "lua", "js", "go", "py", "cc", "cc",
                               "c"};
enum language {
    C11 = 0,
    CPP17,
    PASCAL,
    JAVA,
    RUBY,
    BASH,
    PYTHON2,
    PHP,
    PERL,
    CSHARP,
    OBJC,
    FREEBASIC,
    SCHEMA,
    CLANG,
    CLANGPP,
    LUA,
    JAVASCRIPT,
    GO,
    PYTHON3,
    CPP11,
    CPP98,
    C99,
    OTHER
};
enum status {
    OJ_WT0 = 0,
    OJ_WT1,
    OJ_CI,
    OJ_RI,
    OJ_AC,
    OJ_PE,
    OJ_WA,
    OJ_TL,
    OJ_ML,
    OJ_OL,
    OJ_RE,
    OJ_CE,
    OJ_CO,
    OJ_TR
};


long get_file_size(const char *filename) {
    struct stat f_stat{};

    if (stat(filename, &f_stat) == -1) {
        return 0;
    }

    return (long) f_stat.st_size;
}

void write_log(const char *_fmt, ...) {
    va_list ap;
    char fmt[4096];
    strncpy(fmt, _fmt, 4096);
    char buffer[4096];
    //      time_t          t = time(NULL);
    //int l;
    sprintf(buffer, "%s/log/client.log", oj_home);
    FILE *fp = fopen(buffer, "ae+");
    if (fp == nullptr) {
        fprintf(stderr, "openfile error!\n");
        system("pwd");
    }
    va_start(ap, _fmt);
    //l =
    vsprintf(buffer, fmt, ap);
    fprintf(fp, "%s\n", buffer);
    if (DEBUG)
        printf("%s\n", buffer);
    va_end(ap);
    fclose(fp);

}

int execute_cmd(const char *fmt, ...) {
    char cmd[BUFFER_SIZE];

    int ret = 0;
    va_list ap;

    va_start(ap, fmt);
    vsprintf(cmd, fmt, ap);
    ret = system(cmd);
    va_end(ap);
    return ret;
}

string ws_send(const int &solution_id, const int &state, const int &finished, const double &time,
               const int &memory, const int &pass_point, const double &pass_rate, const string &test_run_result = "",
               const string &compile_info = "") {
    json send_msg;
    int len = (int) test_run_result.length();
    string ntest_run_result;
    for (int i = 0; i < len; ++i) {
        if (test_run_result[i] >= 128)
            ntest_run_result += '?';
        else
            ntest_run_result += test_run_result[i];
    }
    send_msg["type"] = "judger";
    send_msg["value"]["judger"] = judger_number;
    send_msg["value"]["solution_id"] = solution_id;
    send_msg["value"]["state"] = state;
    send_msg["value"]["finish"] = finished;
    send_msg["value"]["time"] = time;
    send_msg["value"]["memory"] = memory;
    send_msg["value"]["pass_rate"] = pass_rate;
    send_msg["value"]["pass_point"] = pass_point;
    if (test_run_result.length())
        send_msg["value"]["test_run_result"] = ntest_run_result;
    if (compile_info.length())
        send_msg["value"]["compile_info"] = compile_info;
    return send_msg.dump();
    /*
    string s="{\"solution_id\":"+to_string(solution_id)+",\"state\":"+to_string(state)+",\"finish\":"+to_string(finished);
    s+=",\"time\":"+to_string(time)+",\"memory\":"+to_string(memory)+",\"pass_point\":"+to_string(pass_point)+"";
    s+="}";
    return s;
     */
}

const int call_array_size = 512;
int call_counter[call_array_size];
static char LANG_NAME[BUFFER_SIZE];

void init_syscalls_limits(int lang) {
    int i;
    memset(call_counter, 0, sizeof(call_counter));
    if (DEBUG)
        write_log("init_call_counter:%d", lang);
    if (record_call) { // C & C++
        for (i = 0; i < call_array_size; i++) {
            call_counter[i] = 0;
        }
    } else if (lang == C11 || lang == CPP17 || lang == CLANG || lang == CLANGPP || lang == C99 || lang == CPP11 ||
               lang == CPP98) { // C & C++
        for (i = 0; i == 0 || LANG_CV[i]; i++) {
            call_counter[LANG_CV[i]] = HOJ_MAX_LIMIT;
        }
    } else if (lang == PASCAL) { // Pascal
        for (i = 0; i == 0 || LANG_PV[i]; i++)
            call_counter[LANG_PV[i]] = HOJ_MAX_LIMIT;
    } else if (lang == JAVA) { // Java
        for (i = 0; i == 0 || LANG_JV[i]; i++)
            call_counter[LANG_JV[i]] = HOJ_MAX_LIMIT;
    } else if (lang == RUBY) { // Ruby
        for (i = 0; i == 0 || LANG_RV[i]; i++)
            call_counter[LANG_RV[i]] = HOJ_MAX_LIMIT;
    } else if (lang == BASH) { // Bash
        for (i = 0; i == 0 || LANG_BV[i]; i++)
            call_counter[LANG_BV[i]] = HOJ_MAX_LIMIT;
    } else if (lang == PYTHON2 || lang == PYTHON3) { // Python
        for (i = 0; i == 0 || LANG_YV[i]; i++)
            call_counter[LANG_YV[i]] = HOJ_MAX_LIMIT;
    } else if (lang == PHP) { // php
        for (i = 0; i == 0 || LANG_PHV[i]; i++)
            call_counter[LANG_PHV[i]] = HOJ_MAX_LIMIT;
    } else if (lang == PERL) { // perl
        for (i = 0; i == 0 || LANG_PLV[i]; i++)
            call_counter[LANG_PLV[i]] = HOJ_MAX_LIMIT;
    } else if (lang == CSHARP) { // mono c#
        for (i = 0; i == 0 || LANG_CSV[i]; i++)
            call_counter[LANG_CSV[i]] = HOJ_MAX_LIMIT;
    } else if (lang == OBJC) { //objective c
        for (i = 0; i == 0 || LANG_OV[i]; i++)
            call_counter[LANG_OV[i]] = HOJ_MAX_LIMIT;
    } else if (lang == FREEBASIC) { //free basic
        for (i = 0; i == 0 || LANG_BASICV[i]; i++)
            call_counter[LANG_BASICV[i]] = HOJ_MAX_LIMIT;
    } else if (lang == SCHEMA) { //scheme guile
        for (i = 0; i == 0 || LANG_SV[i]; i++)
            call_counter[LANG_SV[i]] = HOJ_MAX_LIMIT;
    } else if (lang == LUA) { //lua
        for (i = 0; i == 0 || LANG_LUAV[i]; i++)
            call_counter[LANG_LUAV[i]] = HOJ_MAX_LIMIT;
    } else if (lang == JAVASCRIPT) { //nodejs
        for (i = 0; i == 0 || LANG_JSV[i]; i++)
            call_counter[LANG_JSV[i]] = HOJ_MAX_LIMIT;
    } else if (lang == GO) { //go
        for (i = 0; i == 0 || LANG_GOV[i]; i++)
            call_counter[LANG_GOV[i]] = HOJ_MAX_LIMIT;
    }
}

int after_equal(const char *c) {
    int i = 0;
    for (; c[i] && c[i] != '='; i++);
    return ++i;
}

void trim(char *c) {
    char buf[BUFFER_SIZE];
    char *start, *end;
    strcpy(buf, c);
    start = buf;
    while (isspace(*start))
        ++start;
    end = start;
    while (!isspace(*end))
        ++end;
    *end = '\0';
    strcpy(c, start);
}

bool read_buf(char *buf, const char *key, char *value) {
    if (strncmp(buf, key, strlen(key)) == 0) {
        strcpy(value, buf + after_equal(buf));
        trim(value);
        if (DEBUG)
            printf("%s\n", value);
        return true;
    }
    return false;
}

void read_int(char *buf, const char *key, int &value) {
    char buf2[BUFFER_SIZE];
    if (read_buf(buf, key, buf2))
        value = atoi(buf2);
//      sscanf(buf2, "%d", value);

}

// read the configue file
void init_mysql_conf() {
    FILE *fp = nullptr;
    char buf[BUFFER_SIZE];
    host_name[0] = 0;
    user_name[0] = 0;
    password[0] = 0;
    db_name[0] = 0;
    port_number = 3306;
    max_running = 3;
    sleep_time = 3;
    strcpy(java_xms, "-Xms32m");
    strcpy(java_xmx, "-Xmx256m");
    sprintf(buf, "%s/etc/judge.conf", oj_home);
    fp = fopen("./etc/judge.conf", "re");
    if (fp != nullptr) {
        while (fgets(buf, BUFFER_SIZE - 1, fp)) {
            read_buf(buf, "OJ_HOST_NAME", host_name);
            read_buf(buf, "OJ_USER_NAME", user_name);
            read_buf(buf, "OJ_PASSWORD", password);
            read_buf(buf, "OJ_DB_NAME", db_name);
            read_int(buf, "OJ_PORT_NUMBER", port_number);
            read_int(buf, "OJ_JAVA_TIME_BONUS", java_time_bonus);
            read_int(buf, "OJ_JAVA_MEMORY_BONUS", java_memory_bonus);
            read_int(buf, "OJ_SIM_ENABLE", sim_enable);
            read_buf(buf, "OJ_JAVA_XMS", java_xms);
            read_buf(buf, "OJ_JAVA_XMX", java_xmx);
            read_int(buf, "OJ_HTTP_JUDGE", http_judge);
            read_buf(buf, "OJ_HTTP_BASEURL", http_baseurl);
            read_buf(buf, "OJ_HTTP_USERNAME", http_username);
            read_buf(buf, "OJ_HTTP_PASSWORD", http_password);
            read_int(buf, "OJ_OI_MODE", oi_mode);
            read_int(buf, "OJ_FULL_DIFF", full_diff);
            read_int(buf, "OJ_SHM_RUN", shm_run);
            read_int(buf, "OJ_USE_MAX_TIME", use_max_time);
            read_int(buf, "OJ_USE_PTRACE", use_ptrace);

        }
        //fclose(fp);
    }
    //  fclose(fp);
}

int isInFile(const char fname[]) {
    auto l = static_cast<int>(strlen(fname));
    if (l <= 3 || strcmp(fname + l - 3, ".in") != 0)
        return 0;
    else
        return l - 3;
}

void find_next_nonspace(int &c1, int &c2, FILE *&f1, FILE *&f2, int &ret) {
    // Find the next non-space character or \n.
    while ((isspace(c1)) || (isspace(c2))) {
        if (c1 != c2) {
            if (c2 == EOF) {
                do {
                    c1 = fgetc(f1);
                } while (isspace(c1));
                continue;
            } else if (c1 == EOF) {
                do {
                    c2 = fgetc(f2);
                } while (isspace(c2));
                continue;
#ifdef IGNORE_ESOL
            } else if (isspace(c1) && isspace(c2)) {
                while (c2 == '\n' && isspace(c1) && c1 != '\n')
                    c1 = fgetc(f1);
                while (c1 == '\n' && isspace(c2) && c2 != '\n')
                    c2 = fgetc(f2);

#else
                } else if ((c1 == '\r' && c2 == '\n')) {
                c1 = fgetc(f1);
            } else if ((c2 == '\r' && c1 == '\n')) {
                c2 = fgetc(f2);
#endif
            } else {
                if (DEBUG)
                    printf("%d=%c\t%d=%c", c1, c1, c2, c2);;
                ret = OJ_PE;
            }
        }
        if (isspace(c1)) {
            c1 = fgetc(f1);
        }
        if (isspace(c2)) {
            c2 = fgetc(f2);
        }
    }
}

/***
 int compare_diff(const char *file1,const char *file2){
 char diff[1024];
 sprintf(diff,"diff -q -B -b -w --strip-trailing-cr %s %s",file1,file2);
 int d=system(diff);
 if (d) return OJ_WA;
 sprintf(diff,"diff -q -B --strip-trailing-cr %s %s",file1,file2);
 int p=system(diff);
 if (p) return OJ_PE;
 else return OJ_AC;

 }
 */
const char *getFileNameFromPath(const char *path) {
    for (auto i = static_cast<int>(strlen(path)); i >= 0; i--) {
        if (path[i] == '/')
            return &path[i + 1];
    }
    return path;
}

void make_diff_out_full(FILE *f1, FILE *f2, int c1, int c2, const char *path) {

    execute_cmd("echo '========[%s]========='>>diff.out", getFileNameFromPath(path));
    execute_cmd("echo '------test in top 100 lines------'>>diff.out");
    execute_cmd("head -100 data.in>>diff.out");
    execute_cmd("echo '------test out top 100 lines-----'>>diff.out");
    execute_cmd("head -100 '%s'>>diff.out", path);
    execute_cmd("echo '------user out top 100 lines-----'>>diff.out");
    execute_cmd("head -100 user.out>>diff.out");
    execute_cmd("echo '------diff out 200 lines-----'>>diff.out");
    execute_cmd("diff '%s' user.out|head -200>>diff.out", path);
    execute_cmd("echo '=============================='>>diff.out");

}

void make_diff_out_simple(FILE *f1, FILE *f2, int c1, int c2, const char *path) {
    execute_cmd("echo '========[%s]========='>>diff.out", getFileNameFromPath(path));
    execute_cmd("echo '=======diff out 100 lines====='>>diff.out");
    execute_cmd("diff '%s' user.out|head -100>>diff.out", path);
    execute_cmd("echo '=============================='>>diff.out");
}

/*
 * translated from ZOJ judger r367
 * http://code.google.com/p/zoj/source/browse/trunk/judge_client/client/text_checker.cc#25
 *
 */
int compare_zoj(const char *file1, const char *file2) {
    if (DEBUG && false) {
        fstream user_out(file1), error_file(file2);
        string tmp;
        cout << "Display user file?(0/1)" << endl;
        int choose = 0;
        cin >> choose;
        if (choose) {
            cout << "user running file" << endl;
            while (getline(user_out, tmp))
                cout << tmp << endl;
        }
        choose = 0;
        cout << "Display error file?(0/1)" << endl;
        cin >> choose;
        if (choose) {
            cout << "error msg" << endl;
            while (getline(error_file, tmp))
                cerr << tmp << endl;
        }
    }
    int ret = OJ_AC;
    int c1, c2;
    FILE *f1, *f2;
    f1 = fopen(file1, "re");
    f2 = fopen(file2, "re");
    if (!f1 || !f2) {
        ret = OJ_RE;
    } else
        for (;;) {
            // Find the first non-space character at the beginning of line.
            // Blank lines are skipped.
            c1 = fgetc(f1);
            c2 = fgetc(f2);
            find_next_nonspace(c1, c2, f1, f2, ret);
            // Compare the current line.
            for (;;) {
                // Read until 2 files return a space or 0 together.
                while ((!isspace(c1) && c1) || (!isspace(c2) && c2)) {
                    if (c1 == EOF && c2 == EOF) {
                        goto end;
                    }
                    if (c1 == EOF || c2 == EOF) {
                        break;
                    }
                    if (c1 != c2) {
                        // Consecutive non-space characters should be all exactly the same
                        ret = OJ_WA;
                        goto end;
                    }
                    c1 = fgetc(f1);
                    c2 = fgetc(f2);
                }
                find_next_nonspace(c1, c2, f1, f2, ret);
                if (c1 == EOF && c2 == EOF) {
                    goto end;
                }
                if (c1 == EOF || c2 == EOF) {
                    ret = OJ_WA;
                    goto end;
                }

                if ((c1 == '\n' || !c1) && (c2 == '\n' || !c2)) {
                    break;
                }
            }
        }
    end:
    if (ret == OJ_WA || ret == OJ_PE) {
        if (full_diff)
            make_diff_out_full(f1, f2, c1, c2, file1);
        else
            make_diff_out_simple(f1, f2, c1, c2, file1);
    }
    if (f1)
        fclose(f1);
    if (f2)
        fclose(f2);
    return ret;
}

void delnextline(char s[]) {
    int L;
    L = static_cast<int>(strlen(s));
    while (L > 0 && (s[L - 1] == '\n' || s[L - 1] == '\r'))
        s[--L] = 0;
}

int compare(const char *file1, const char *file2) {
#ifdef ZOJ_COM
    //compare ported and improved from zoj don't limit file size
    return compare_zoj(file1, file2);
#endif
#ifndef ZOJ_COM
    //the original compare from the first version of hustoj has file size limit
    //and waste memory
    FILE *f1,*f2;
    char *s1,*s2,*p1,*p2;
    int PEflg;
    s1=new char[STD_F_LIM+512];
    s2=new char[STD_F_LIM+512];
    if (!(f1=fopen(file1,"re")))
        return OJ_AC;
    for (p1=s1;EOF!=fscanf(f1,"%s",p1);)
        while (*p1) p1++;
    fclose(f1);
    if (!(f2=fopen(file2,"re")))
        return OJ_RE;
    for (p2=s2;EOF!=fscanf(f2,"%s",p2);)
        while (*p2) p2++;
    fclose(f2);
    if (strcmp(s1,s2)!=0) {
        //              printf("A:%s\nB:%s\n",s1,s2);
        delete[] s1;
        delete[] s2;

        return OJ_WA;
    } else {
        f1=fopen(file1,"re");
        f2=fopen(file2,"re");
        PEflg=0;
        while (PEflg==0 && fgets(s1,STD_F_LIM,f1) && fgets(s2,STD_F_LIM,f2)) {
            delnextline(s1);
            delnextline(s2);
            if (strcmp(s1,s2)==0) continue;
            else PEflg=1;
        }
        delete [] s1;
        delete [] s2;
        fclose(f1);fclose(f2);
        if (PEflg) return OJ_PE;
        else return OJ_AC;
    }
#endif
}

FILE *read_cmd_output(const char *fmt, ...) {
    char cmd[BUFFER_SIZE];

    FILE *ret = nullptr;
    va_list ap;

    va_start(ap, fmt);
    vsprintf(cmd, fmt, ap);
    va_end(ap);
    if (DEBUG)
        printf("%s\n", cmd);
    ret = popen(cmd, "r");

    return ret;
}

/* write result back to database */
void _update_solution_mysql(int solution_id, int result, double time, int memory,
                            int sim, int sim_s_id, double pass_rate) {
    char sql[BUFFER_SIZE];
    if (oi_mode) {
        sprintf(sql,
                "UPDATE solution SET result=%d,time=%f,memory=%d,pass_rate=%f,judger='%s' WHERE solution_id=%d LIMIT 1%c",
                result, time, memory, pass_rate, http_username, solution_id, 0);
    } else {
        sprintf(sql,
                "UPDATE solution SET result=%d,time=%f,memory=%d,judger='%s' WHERE solution_id=%d LIMIT 1%c",
                result, time, memory, http_username, solution_id, 0);
    }
    //      printf("sql= %s\n",sql);
    if (mysql_real_query(conn, sql, strlen(sql))) {
        //              printf("..update failed! %s\n",mysql_error(conn));
    }
    if (sim) {
        sprintf(sql,
                "insert into sim(s_id,sim_s_id,sim) values(%d,%d,%d) on duplicate key update  sim_s_id=%d,sim=%d",
                solution_id, sim_s_id, sim, sim_s_id, sim);
        //      printf("sql= %s\n",sql);
        if (mysql_real_query(conn, sql, strlen(sql))) {
            //              printf("..update failed! %s\n",mysql_error(conn));
        }

    }

}

void update_solution(int solution_id, int result, double time, int memory, int sim,
                     int sim_s_id, double pass_rate) {
    if (result == OJ_TL && memory == 0)
        result = OJ_ML;
    _update_solution_mysql(solution_id, result, time, memory, sim, sim_s_id,
                           pass_rate);
}

/* write compile error message back to database */
void _addceinfo_mysql(int solution_id) {
    char sql[(1 << 16)], *end;
    char ceinfo[(1 << 16)], *cend;
    FILE *fp = fopen("ce.txt", "re");
    snprintf(sql, (1 << 16) - 1, "DELETE FROM compileinfo WHERE solution_id=%d",
             solution_id);
    mysql_real_query(conn, sql, strlen(sql));
    cend = ceinfo;
    while (fgets(cend, 1024, fp)) {
        cend += strlen(cend);
        if (cend - ceinfo > 4096)
            break;
    }
    cend = nullptr;
    end = sql;
    strcpy(end, "INSERT INTO compileinfo VALUES(");
    end += strlen(sql);
    *end++ = '\'';
    end += sprintf(end, "%d", solution_id);
    *end++ = '\'';
    *end++ = ',';
    *end++ = '\'';
    end += mysql_real_escape_string(conn, end, ceinfo, strlen(ceinfo));
    *end++ = '\'';
    *end++ = ')';
    *end = 0;
    //      printf("%s\n",ceinfo);
    if (mysql_real_query(conn, sql, end - sql))
        printf("%s\n", mysql_error(conn));
    fclose(fp);
}
// urlencoded function copied from http://www.geekhideout.com/urlcode.shtml
/* Converts a hex character to its integer value */
char from_hex(char ch) {
    return static_cast<char>(isdigit(ch) ? ch - '0' : tolower(ch) - 'a' + 10);
}

/* Converts an integer value to its hex character*/
char to_hex(char code) {
    static char hex[] = "0123456789abcdef";
    return hex[code & 15];
}

/* Returns a url-encoded version of str */
/* IMPORTANT: be sure to free() the returned string after use */
char *url_encode(char *str) {
    char *pstr = str, *buf = (char *) malloc(strlen(str) * 3 + 1), *pbuf = buf;
    while (*pstr) {
        if (isalnum(*pstr) || *pstr == '-' || *pstr == '_' || *pstr == '.'
            || *pstr == '~')
            *pbuf++ = *pstr;
        else if (*pstr == ' ')
            *pbuf++ = '+';
        else
            *pbuf++ = '%', *pbuf++ = to_hex(*pstr >> 4), *pbuf++ = to_hex(
                    static_cast<char>(*pstr & 15));
        pstr++;
    }
    *pbuf = '\0';
    return buf;
}

void addceinfo(int solution_id) {
    _addceinfo_mysql(solution_id);
}

/* write runtime error message back to database */
void _addreinfo_mysql(int solution_id, const char *filename) {
    char sql[(1 << 16)], *end;
    char reinfo[(1 << 16)], *rend;
    FILE *fp = fopen(filename, "re");
    snprintf(sql, (1 << 16) - 1, "DELETE FROM runtimeinfo WHERE solution_id=%d",
             solution_id);
    mysql_real_query(conn, sql, strlen(sql));
    rend = reinfo;
    while (fgets(rend, 1024, fp)) {
        rend += strlen(rend);
        if (rend - reinfo > 4096)
            break;
    }
    rend = nullptr;
    end = sql;
    strcpy(end, "INSERT INTO runtimeinfo VALUES(");
    end += strlen(sql);
    *end++ = '\'';
    end += sprintf(end, "%d", solution_id);
    *end++ = '\'';
    *end++ = ',';
    *end++ = '\'';
    end += mysql_real_escape_string(conn, end, reinfo, strlen(reinfo));
    *end++ = '\'';
    *end++ = ')';
    *end = 0;
    //      printf("%s\n",ceinfo);
    if (mysql_real_query(conn, sql, end - sql))
        printf("%s\n", mysql_error(conn));
    fclose(fp);
}


void addreinfo(int solution_id) {
    _addreinfo_mysql(solution_id, "error.out");
}

void adddiffinfo(int solution_id) {
    _addreinfo_mysql(solution_id, "diff.out");
}

void addcustomout(int solution_id) {
    _addreinfo_mysql(solution_id, "user.out");
}

void _update_user_mysql(char *user_id) {
    char sql[BUFFER_SIZE];
    sprintf(sql,
            R"(UPDATE `users` SET `solved`=(SELECT count(DISTINCT `problem_id`) FROM `solution` WHERE `user_id`='%s' AND `result`='4') WHERE `user_id`='%s')",
            user_id, user_id);
    if (mysql_real_query(conn, sql, strlen(sql)))
        write_log(mysql_error(conn));
    sprintf(sql,
            R"(UPDATE `users` SET `submit`=(SELECT count(*) FROM `solution` WHERE `user_id`='%s' and problem_id>0) WHERE `user_id`='%s')",
            user_id, user_id);
    if (mysql_real_query(conn, sql, strlen(sql)))
        write_log(mysql_error(conn));
}


void update_user(char *user_id) {
    _update_user_mysql(user_id);
}

void _update_problem_mysql(int p_id) {
    char sql[BUFFER_SIZE];
    sprintf(sql,
            R"(UPDATE `problem` SET `accepted`=(SELECT count(*) FROM `solution` WHERE `problem_id`='%d' AND `result`='4') WHERE `problem_id`='%d')",
            p_id, p_id);
    if (mysql_real_query(conn, sql, strlen(sql)))
        write_log(mysql_error(conn));
    sprintf(sql,
            R"(UPDATE `problem` SET `submit`=(SELECT count(*) FROM `solution` WHERE `problem_id`='%d') WHERE `problem_id`='%d')",
            p_id, p_id);
    if (mysql_real_query(conn, sql, strlen(sql)))
        write_log(mysql_error(conn));
}

void update_problem(int pid) {
    _update_problem_mysql(pid);
}

void umount(char *work_dir) {
    execute_cmd("/bin/umount -f %s/proc", work_dir);
    execute_cmd("/bin/umount -f %s/dev ", work_dir);
    execute_cmd("/bin/umount -f %s/lib ", work_dir);
    execute_cmd("/bin/umount -f %s/lib64 ", work_dir);
    execute_cmd("/bin/umount -f %s/etc/alternatives ", work_dir);
    execute_cmd("/bin/umount -f %s/usr ", work_dir);
    execute_cmd("/bin/umount -f %s/bin ", work_dir);
    execute_cmd("/bin/umount -f %s/proc ", work_dir);
    execute_cmd("/bin/umount -f bin usr lib lib64 etc/alternatives proc dev ");
    execute_cmd("/bin/umount -f %s/* ", work_dir);
    execute_cmd("/bin/umount -f %s/log/* ", work_dir);
    execute_cmd("/bin/umount -f %s/log/etc/alternatives ", work_dir);
}

int compile(int lang, char *work_dir) {
    int pid;
    if (webSocket.isconnected()) {
        webSocket << ws_send(solution_id, 2, 0, 0, 0, 0, 0);
    }
    const char *CP_C[] = {"/usr/local/bin/gcc", "Main.c", "-o", "Main", "-fno-asm", "-Wall", "-O2",
                          "-lm", "--static", "-std=c11", "-DONLINE_JUDGE", nullptr};
    const char *CP_CC[] = {"/usr/local/bin/gcc", "Main.c", "-o", "Main", "-fno-asm", "-Wall", "-O2",
                           "-lm", "--static", "-std=c99", "-DONLINE_JUDGE", nullptr};
    const char *CP_X[] = {"/usr/local/bin/g++", "-fno-asm", "-Wall", "-O2",
                          "-lm", "--static", "-std=c++17", "-DONLINE_JUDGE", "-o", "Main", "Main.cc", nullptr};
    const char *CP_XX[] = {"/usr/local/bin/g++", "-fno-asm", "-Wall", "-O2",
                           "-lm", "--static", "-std=c++11", "-DONLINE_JUDGE", "-o", "Main", "Main.cc", nullptr};
    const char *CP_XXX[] = {"/usr/local/bin/g++", "-fno-asm", "-Wall", "-O2",
                            "-lm", "--static", "-std=c++98", "-DONLINE_JUDGE", "-o", "Main", "Main.cc", nullptr};
    const char *CP_P[] =
            {"fpc", "Main.pas", "-Cs32000000", "-Sh", "-O2", "-Co", "-Ct", "-Ci", nullptr};
    //      const char * CP_J[] = { "javac", "-J-Xms32m", "-J-Xmx256m","-encoding","UTF-8", "Main.java",NULL };

    const char *CP_R[] = {"ruby", "-c", "Main.rb", nullptr};
    const char *CP_B[] = {"chmod", "+rx", "Main.sh", nullptr};
    //const char * CP_Y[] = { "python", "-c",
    //                        "import py_compile; py_compile.compile(r'Main.py')", NULL };
    //const char * CP_Y3[] = { "python3", "-c",
    //                         "import py_compile; py_compile.compile(r'Main.py')", NULL };
    const char *CP_PH[] = {"php", "-l", "Main.php", nullptr};
    const char *CP_PL[] = {"perl", "-c", "Main.pl", nullptr};
    const char *CP_CS[] = {"gmcs", "-warn:0", "Main.cs", nullptr};
    const char *CP_OC[] = {"gcc", "-o", "Main", "Main.m",
                           "-fconstant-string-class=NSConstantString", "-I",
                           "/usr/include/GNUstep/", "-L", "/usr/lib/GNUstep/Libraries/",
                           "-lobjc", "-lgnustep-base", nullptr};
    const char *CP_BS[] = {"fbc", "-lang", "qb", "Main.bas", nullptr};
    const char *CP_CLANG[] = {"clang", "Main.c", "-o", "Main", "-fno-asm", "-Wall",
                              "-lm", "--static", "-std=c99", "-DONLINE_JUDGE", nullptr};
    const char *CP_CLANG_CPP[] = {"clang++", "Main.cc", "-o", "Main", "-fno-asm", "-Wall",
                                  "-lm", "--static", "-std=c++11", "-DONLINE_JUDGE", nullptr};
    const char *CP_LUA[] = {"luac", "-o", "Main", "Main.lua", nullptr};
    //const char * CP_JS[] = { "js24","-c", "Main.js", NULL };
    const char *CP_GO[] = {"go", "build", "-o", "Main", "Main.go", nullptr};

    char javac_buf[7][32];
    char *CP_J[7];

    for (int i = 0; i < 7; i++)
        CP_J[i] = javac_buf[i];

    sprintf(CP_J[0], "javac");
    sprintf(CP_J[1], "-J%s", java_xms);
    sprintf(CP_J[2], "-J%s", java_xmx);
    sprintf(CP_J[3], "-encoding");
    sprintf(CP_J[4], "UTF-8");
    sprintf(CP_J[5], "Main.java");
    CP_J[6] = (char *) nullptr;

    pid = fork();
    if (pid == 0) {
        struct rlimit LIM{};
        LIM.rlim_max = 60;
        LIM.rlim_cur = 60;
        setrlimit(RLIMIT_CPU, &LIM);
        alarm(60);
        LIM.rlim_max = static_cast<rlim_t>(10 * STD_MB);
        LIM.rlim_cur = static_cast<rlim_t>(10 * STD_MB);
        setrlimit(RLIMIT_FSIZE, &LIM);

        if (lang == 3 || lang == 17) {
            LIM.rlim_max = static_cast<rlim_t>(STD_MB << 11);
            LIM.rlim_cur = static_cast<rlim_t>(STD_MB << 11);
        } else {
            LIM.rlim_max = static_cast<rlim_t>(STD_MB * 256);
            LIM.rlim_cur = static_cast<rlim_t>(STD_MB * 256);
        }
        setrlimit(RLIMIT_AS, &LIM);
        if (lang != 2 && lang != 11) {
            freopen("ce.txt", "w", stderr);
            //freopen("/dev/null", "w", stdout);
        } else {
            freopen("ce.txt", "w", stdout);
        }
        if (lang != 1 && lang != 3 && lang != 9 && lang != 6 && lang != 11 && lang != 18) {
            execute_cmd("mkdir -p bin usr lib lib64 etc/alternatives proc tmp dev");
            execute_cmd("chown judge *");
            execute_cmd("mount -o bind /bin bin");
            execute_cmd("mount -o bind /usr usr");
            execute_cmd("mount -o bind /lib lib");
#ifndef __i386
            execute_cmd("mount -o bind /lib64 lib64");
#endif
            execute_cmd("mount -o bind /etc/alternatives etc/alternatives");
            execute_cmd("mount -o bind /proc proc");
            if (lang > 2 && lang != 10 && lang != 13 && lang != 14)
                execute_cmd("mount -o bind /dev dev");
            chroot(work_dir);
            //chroot(work_dir);
        }
        while (setgid(1536) != 0)
            sleep(1);
        while (setuid(1536) != 0)
            sleep(1);
        while (setresuid(1536, 1536, 1536) != 0)
            sleep(1);
        switch (lang) {
            case C11:
                execvp(CP_C[0], (char *const *) CP_C);
                if (DEBUG)
                    cout << CP_C[0] << endl;
                break;
            case CPP17:
                execvp(CP_X[0], (char *const *) CP_X);
                if (DEBUG)
                    cout << CP_X[0] << endl;
                break;
            case PASCAL:
                execvp(CP_P[0], (char *const *) CP_P);
                if (DEBUG)
                    cout << CP_P[0] << endl;
                break;
            case JAVA:
                execvp(CP_J[0], (char *const *) CP_J);
                if (DEBUG)
                    cout << CP_J[0] << endl;
                break;
            case RUBY:
                execvp(CP_R[0], (char *const *) CP_R);
                if (DEBUG)
                    cout << CP_R[0] << endl;
                break;
            case BASH:
                execvp(CP_B[0], (char *const *) CP_B);
                if (DEBUG)
                    cout << CP_B[0] << endl;
                break;
            case PYTHON2:
                //execvp(CP_Y[0], (char * const *) CP_Y);
                break;
            case PHP:
                execvp(CP_PH[0], (char *const *) CP_PH);
                break;
            case PERL:
                execvp(CP_PL[0], (char *const *) CP_PL);
                break;
            case CSHARP:
                execvp(CP_CS[0], (char *const *) CP_CS);
                break;
            case OBJC:
                execvp(CP_OC[0], (char *const *) CP_OC);
                break;
            case FREEBASIC:
                execvp(CP_BS[0], (char *const *) CP_BS);
                break;
            case CLANG:
                execvp(CP_CLANG[0], (char *const *) CP_CLANG);
                break;
            case CLANGPP:
                execvp(CP_CLANG_CPP[0], (char *const *) CP_CLANG_CPP);
                break;
            case LUA:
                execvp(CP_LUA[0], (char *const *) CP_LUA);
                break;
                //case JAVASCRIPT:
                //  execvp(CP_JS[0], (char * const *) CP_JS);
                //  break;
            case GO:
                execvp(CP_GO[0], (char *const *) CP_GO);
                break;
            case PYTHON3:
                //execvp(CP_Y3[0], (char * const *) CP_Y3);
                break;
            case CPP11:
                execvp(CP_XX[0], (char *const *) CP_XX);
                if (DEBUG)
                    cout << CP_XX[0] << endl;
                break;
            case CPP98:
                execvp(CP_XXX[0], (char *const *) CP_XXX);
                if (DEBUG)
                    cout << CP_XXX[0] << endl;
                break;
            case C99:
                execvp(CP_CC[0], (char *const *) CP_CC);
                if (DEBUG)
                    cout << CP_CC[0] << endl;
            default:
                printf("nothing to do!\n");
        }
        if (DEBUG)
            printf("compile end!\n");
        //exit(!system("cat ce.txt"));
        exit(0);
    } else {
        int status = 0;

        waitpid(pid, &status, 0);
        if (lang > JAVA && lang < PHP)
            status = static_cast<int>(get_file_size("ce.txt"));
        if (DEBUG)
            printf("status=%d\n", status);
        execute_cmd("/bin/umount -f bin usr lib lib64 etc/alternatives proc dev 2>&1 >/dev/null");
        execute_cmd("/bin/umount -f %s/* 2>&1 >/dev/null", work_dir);
        umount(work_dir);
        return status;
    }

}

/*
 int read_proc_statm(int pid){
 FILE * pf;
 char fn[4096];
 int ret;
 sprintf(fn,"/proc/%d/statm",pid);
 pf=fopen(fn,"r");
 fscanf(pf,"%d",&ret);
 fclose(pf);
 return ret;
 }
 */
int get_proc_status(int pid, const char *mark) {
    FILE *pf;
    char fn[BUFFER_SIZE], buf[BUFFER_SIZE];
    int ret = 0;
    sprintf(fn, "/proc/%d/status", pid);
    pf = fopen(fn, "re");
    auto m = static_cast<int>(strlen(mark));
    while (pf && fgets(buf, BUFFER_SIZE - 1, pf)) {

        buf[strlen(buf) - 1] = 0;
        if (strncmp(buf, mark, m) == 0) {
            sscanf(buf + m + 1, "%d", &ret);
        }
    }
    if (pf)
        fclose(pf);
    return ret;
}

int init_mysql_conn() {

    conn = mysql_init(nullptr);
    //mysql_real_connect(conn,host_name,user_name,password,db_name,port_number,0,0);
    const char timeout = 30;
    mysql_options(conn, MYSQL_OPT_CONNECT_TIMEOUT, &timeout);

    if (!mysql_real_connect(conn, host_name, user_name, password, db_name,
                            static_cast<unsigned int>(port_number), 0, 0)) {
        write_log("%s", mysql_error(conn));
        return 0;
    }
    const char *utf8sql = "set names utf8";
    if (mysql_real_query(conn, utf8sql, strlen(utf8sql))) {
        write_log("%s", mysql_error(conn));
        return 0;
    }
    return 1;
}


void get_solution(int solution_id, char *work_dir, int lang, char *usercode) {
    char sql[BUFFER_SIZE], src_pth[BUFFER_SIZE];
    // get the source code
    MYSQL_RES *res;
    MYSQL_ROW row;
    sprintf(sql, "SELECT source FROM source_code WHERE solution_id=%d",
            solution_id);
    mysql_real_query(conn, sql, strlen(sql));
    res = mysql_store_result(conn);
    row = mysql_fetch_row(res);
    sprintf(usercode, "%s", row[0]);
    // create the src file
    sprintf(src_pth, "Main.%s", lang_ext[lang]);
    if (DEBUG)
        printf("Main=%s", src_pth);
    FILE *fp_src = fopen(src_pth, "we");
    fprintf(fp_src, "%s", row[0]);
    mysql_free_result(res);
    fclose(fp_src);
}


void get_custominput(int solution_id, char *work_dir) {
    char sql[BUFFER_SIZE], src_pth[BUFFER_SIZE];
    // get the source code
    MYSQL_RES *res;
    MYSQL_ROW row;
    sprintf(sql, "SELECT input_text FROM custominput WHERE solution_id=%d",
            solution_id);
    mysql_real_query(conn, sql, strlen(sql));
    res = mysql_store_result(conn);
    row = mysql_fetch_row(res);
    if (row != nullptr) {

        // create the src file
        sprintf(src_pth, "data.in");
        FILE *fp_src = fopen(src_pth, "w");
        fprintf(fp_src, "%s", row[0]);
        fclose(fp_src);

    }
    mysql_free_result(res);
}


void get_solution_info(int solution_id, int &p_id, char *user_id,
                       int &lang) {
    MYSQL_RES *res;
    MYSQL_ROW row;

    char sql[BUFFER_SIZE];
    // get the problem id and user id from Table:solution
    sprintf(sql,
            "SELECT problem_id, user_id, language FROM solution where solution_id=%d",
            solution_id);
    //printf("%s\n",sql);
    mysql_real_query(conn, sql, strlen(sql));
    res = mysql_store_result(conn);
    row = mysql_fetch_row(res);
    p_id = atoi(row[0]);
    strcpy(user_id, row[1]);
    lang = atoi(row[2]);
    mysql_free_result(res);
}


void get_problem_info(int p_id, double &time_lmt, int &mem_lmt, int &isspj) {
    char sql[BUFFER_SIZE];
    MYSQL_RES *res;
    MYSQL_ROW row;
    sprintf(sql,
            "SELECT time_limit,memory_limit,spj FROM problem where problem_id=%d",
            p_id);
    mysql_real_query(conn, sql, strlen(sql));
    res = mysql_store_result(conn);
    row = mysql_fetch_row(res);
    time_lmt = atof(row[0]);
    mem_lmt = atoi(row[1]);
    isspj = (row[2][0] == '1');
    mysql_free_result(res);
    if (time_lmt <= 0)
        time_lmt = 1;

}

char *escape(char s[], const char t[]) {
    int i, j;
    for (i = j = 0; t[i] != '\0'; ++i) {
        if (t[i] == '\'') {
            s[j++] = '\'';
            s[j++] = '\\';
            s[j++] = '\'';
            s[j++] = '\'';
            continue;
        } else {
            s[j++] = t[i];
        }
    }
    s[j] = '\0';
    return s;
}

void prepare_files(char *filename, int namelen, char *infile, int &p_id,
                   char *work_dir, char *outfile, char *userfile, int runner_id) {
    //              printf("ACflg=%d %d check a file!\n",ACflg,solution_id);

    char fname0[BUFFER_SIZE];
    char fname[BUFFER_SIZE];
    strncpy(fname0, filename, static_cast<size_t>(namelen));
    fname0[namelen] = 0;
    escape(fname, fname0);
    printf("%s\n%s\n", fname0, fname);
    sprintf(infile, "%s/data/%d/%s.in", oj_home, p_id, fname);
    execute_cmd("/bin/cp '%s' %s/data.in", infile, work_dir);
    execute_cmd("/bin/cp %s/data/%d/*.dic %s/", oj_home, p_id, work_dir);

    sprintf(outfile, "%s/data/%d/%s.out", oj_home, p_id, fname0);
    sprintf(userfile, "%s/run%d/user.out", oj_home, runner_id);
}

void copy_shell_runtime(char *work_dir) {

    execute_cmd("/bin/mkdir %s/lib", work_dir);
    execute_cmd("/bin/mkdir %s/lib64", work_dir);
    execute_cmd("/bin/mkdir %s/bin", work_dir);
    //  execute_cmd("/bin/cp /lib/* %s/lib/", work_dir);
    //  execute_cmd("/bin/cp -a /lib/i386-linux-gnu %s/lib/", work_dir);
    //  execute_cmd("/bin/cp -a /usr/lib/i386-linux-gnu %s/lib/", work_dir);
    execute_cmd("/bin/cp -a /lib/x86_64-linux-gnu %s/lib/", work_dir);
    execute_cmd("/bin/cp /lib64/* %s/lib64/", work_dir);
    //  execute_cmd("/bin/cp /lib32 %s/", work_dir);
    execute_cmd("/bin/cp /bin/busybox %s/bin/", work_dir);
    execute_cmd("/bin/ln -s /bin/busybox %s/bin/sh", work_dir);
    execute_cmd("/bin/cp /bin/bash %s/bin/bash", work_dir);

}

void copy_objc_runtime(char *work_dir) {
    copy_shell_runtime(work_dir);
    execute_cmd("/bin/mkdir -p %s/proc", work_dir);
    execute_cmd("/bin/mount -o bind /proc %s/proc", work_dir);
    execute_cmd("/bin/mkdir -p %s/lib/", work_dir);
    execute_cmd(
            "/bin/cp -aL /lib/libdbus-1.so.3                          %s/lib/ ",
            work_dir);
    execute_cmd(
            "/bin/cp -aL /lib/libgcc_s.so.1                           %s/lib/ ",
            work_dir);
    execute_cmd(
            "/bin/cp -aL /lib/libgcrypt.so.11                         %s/lib/ ",
            work_dir);
    execute_cmd(
            "/bin/cp -aL /lib/libgpg-error.so.0                       %s/lib/ ",
            work_dir);
    execute_cmd(
            "/bin/cp -aL /lib/libz.so.1                               %s/lib/ ",
            work_dir);
    execute_cmd(
            "/bin/cp -aL /lib/tls/i686/cmov/libc.so.6                 %s/lib/ ",
            work_dir);
    execute_cmd(
            "/bin/cp -aL /lib/tls/i686/cmov/libdl.so.2                %s/lib/ ",
            work_dir);
    execute_cmd(
            "/bin/cp -aL /lib/tls/i686/cmov/libm.so.6                 %s/lib/ ",
            work_dir);
    execute_cmd(
            "/bin/cp -aL /lib/tls/i686/cmov/libnsl.so.1               %s/lib/ ",
            work_dir);
    execute_cmd(
            "/bin/cp -aL /lib/tls/i686/cmov/libpthread.so.0           %s/lib/ ",
            work_dir);
    execute_cmd(
            "/bin/cp -aL /lib/tls/i686/cmov/librt.so.1                %s/lib/ ",
            work_dir);
    execute_cmd(
            "/bin/cp -aL /usr/lib/libavahi-client.so.3                %s/lib/ ",
            work_dir);
    execute_cmd(
            "/bin/cp -aL /usr/lib/libavahi-common.so.3                %s/lib/ ",
            work_dir);
    execute_cmd(
            "/bin/cp -aL /usr/lib/libdns_sd.so.1                      %s/lib/ ",
            work_dir);
    execute_cmd(
            "/bin/cp -aL /usr/lib/libffi.so.5                         %s/lib/ ",
            work_dir);
    execute_cmd(
            "/bin/cp -aL /usr/lib/libgnustep-base.so.1.19             %s/lib/ ",
            work_dir);
    execute_cmd(
            "/bin/cp -aL /usr/lib/libgnutls.so.26                     %s/lib/ ",
            work_dir);
    execute_cmd(
            "/bin/cp -aL /usr/lib/libobjc.so.2                        %s/lib/ ",
            work_dir);
    execute_cmd(
            "/bin/cp -aL /usr/lib/libtasn1.so.3                       %s/lib/ ",
            work_dir);
    execute_cmd(
            "/bin/cp -aL /usr/lib/libxml2.so.2                        %s/lib/ ",
            work_dir);
    execute_cmd(
            "/bin/cp -aL /usr/lib/libxslt.so.1                        %s/lib/ ",
            work_dir);

}

void copy_bash_runtime(char *work_dir) {
    //char cmd[BUFFER_SIZE];
    //const char * ruby_run="/usr/bin/ruby";
    copy_shell_runtime(work_dir);
    execute_cmd("/bin/cp `which bc`  %s/bin/", work_dir);
    execute_cmd("busybox dos2unix Main.sh", work_dir);
    execute_cmd("/bin/ln -s /bin/busybox %s/bin/grep", work_dir);
    execute_cmd("/bin/ln -s /bin/busybox %s/bin/awk", work_dir);
    execute_cmd("/bin/cp /bin/sed %s/bin/sed", work_dir);
    execute_cmd("/bin/ln -s /bin/busybox %s/bin/cut", work_dir);
    execute_cmd("/bin/ln -s /bin/busybox %s/bin/sort", work_dir);
    execute_cmd("/bin/ln -s /bin/busybox %s/bin/join", work_dir);
    execute_cmd("/bin/ln -s /bin/busybox %s/bin/wc", work_dir);
    execute_cmd("/bin/ln -s /bin/busybox %s/bin/tr", work_dir);
    execute_cmd("/bin/ln -s /bin/busybox %s/bin/dc", work_dir);
    execute_cmd("/bin/ln -s /bin/busybox %s/bin/dd", work_dir);
    execute_cmd("/bin/ln -s /bin/busybox %s/bin/cat", work_dir);
    execute_cmd("/bin/ln -s /bin/busybox %s/bin/tail", work_dir);
    execute_cmd("/bin/ln -s /bin/busybox %s/bin/head", work_dir);
    execute_cmd("/bin/ln -s /bin/busybox %s/bin/xargs", work_dir);
    execute_cmd("chmod +rx %s/Main.sh", work_dir);

}

void copy_ruby_runtime(char *work_dir) {

    copy_shell_runtime(work_dir);
    execute_cmd("mkdir -p %s/usr", work_dir);
    execute_cmd("mkdir -p %s/usr/lib", work_dir);
    execute_cmd("mkdir -p %s/usr/lib64", work_dir);
    execute_cmd("cp -a /usr/lib/libruby* %s/usr/lib/", work_dir);
    execute_cmd("cp -a /usr/lib/ruby* %s/usr/lib/", work_dir);
    execute_cmd("cp -a /usr/lib64/ruby* %s/usr/lib64/", work_dir);
    execute_cmd("cp -a /usr/lib64/libruby* %s/usr/lib64/", work_dir);
    execute_cmd("cp -a /usr/bin/ruby* %s/", work_dir);

}

void copy_guile_runtime(char *work_dir) {

    copy_shell_runtime(work_dir);
    execute_cmd("/bin/mkdir -p %s/proc", work_dir);
    execute_cmd("/bin/mount -o bind /proc %s/proc", work_dir);
    execute_cmd("/bin/mkdir -p %s/usr/lib", work_dir);
    execute_cmd("/bin/mkdir -p %s/usr/share", work_dir);
    execute_cmd("/bin/cp -a /usr/share/guile %s/usr/share/", work_dir);
    execute_cmd("/bin/cp /usr/lib/libguile* %s/usr/lib/", work_dir);
    execute_cmd("/bin/cp /usr/lib/libgc* %s/usr/lib/", work_dir);
    execute_cmd("/bin/cp /usr/lib/i386-linux-gnu/libffi* %s/usr/lib/",
                work_dir);
    execute_cmd("/bin/cp /usr/lib/i386-linux-gnu/libunistring* %s/usr/lib/",
                work_dir);
    execute_cmd("/bin/cp /usr/lib/*/libgmp* %s/usr/lib/", work_dir);
    execute_cmd("/bin/cp /usr/lib/libgmp* %s/usr/lib/", work_dir);
    execute_cmd("/bin/cp /usr/lib/*/libltdl* %s/usr/lib/", work_dir);
    execute_cmd("/bin/cp /usr/lib/libltdl* %s/usr/lib/", work_dir);
    execute_cmd("/bin/cp /usr/bin/guile* %s/", work_dir);

}

void copy_python_runtime(char *work_dir) {

    copy_shell_runtime(work_dir);
    execute_cmd("mkdir -p %s/usr/include", work_dir);
    execute_cmd("mkdir -p %s/dev", work_dir);
    execute_cmd("mkdir -p %s/usr/lib", work_dir);
    execute_cmd("mkdir -p %s/usr/lib64", work_dir);
    execute_cmd("mkdir -p %s/usr/local/lib", work_dir);
    execute_cmd("cp /usr/bin/python* %s/", work_dir);
    execute_cmd("cp /usr/local/bin/python* %s/", work_dir);
    execute_cmd("cp -a /usr/lib/python* %s/usr/lib/", work_dir);
    execute_cmd("cp -a /usr/local/lib/python* %s/usr/lib/", work_dir);
    execute_cmd("cp -a /usr/lib64/python* %s/usr/lib64/", work_dir);
    execute_cmd("cp -a /usr/local/lib64/python* %s/usr/lib64/", work_dir);
    execute_cmd("cp -a /usr/local/lib/python* %s/usr/local/lib/", work_dir);
    execute_cmd("cp -a /usr/include/python* %s/usr/include/", work_dir);
    execute_cmd("cp -a /usr/local/include/python* %s/usr/include/", work_dir);
    execute_cmd("cp -a /usr/lib/libpython* %s/usr/lib/", work_dir);
    execute_cmd("cp -a /usr/local/lib/libpython* %s/usr/lib/", work_dir);
    execute_cmd("/bin/mkdir -p %s/home/judge", work_dir);
    execute_cmd("/bin/chown judge %s", work_dir);
    execute_cmd("/bin/mkdir -p %s/etc", work_dir);
    execute_cmd("/bin/grep judge /etc/passwd>%s/etc/passwd", work_dir);
    execute_cmd("/bin/mount -o bind /dev %s/dev", work_dir);


}

void copy_php_runtime(char *work_dir) {

    copy_shell_runtime(work_dir);
    execute_cmd("/bin/mkdir %s/usr", work_dir);
    execute_cmd("/bin/mkdir %s/usr/lib", work_dir);
    execute_cmd("/bin/cp /usr/lib/libedit* %s/usr/lib/", work_dir);
    execute_cmd("/bin/cp /usr/lib/libdb* %s/usr/lib/", work_dir);
    execute_cmd("/bin/cp /usr/lib/libgssapi_krb5* %s/usr/lib/", work_dir);
    execute_cmd("/bin/cp /usr/lib/libkrb5* %s/usr/lib/", work_dir);
    execute_cmd("/bin/cp /usr/lib/libk5crypto* %s/usr/lib/", work_dir);
    execute_cmd("/bin/cp /usr/lib/*/libedit* %s/usr/lib/", work_dir);
    execute_cmd("/bin/cp /usr/lib/*/libdb* %s/usr/lib/", work_dir);
    execute_cmd("/bin/cp /usr/lib/*/libgssapi_krb5* %s/usr/lib/", work_dir);
    execute_cmd("/bin/cp /usr/lib/*/libkrb5* %s/usr/lib/", work_dir);
    execute_cmd("/bin/cp /usr/lib/*/libk5crypto* %s/usr/lib/", work_dir);
    execute_cmd("/bin/cp /usr/lib/libxml2* %s/usr/lib/", work_dir);
    execute_cmd("/bin/cp /usr/lib/x86_64-linux-gnu/libxml2.so* %s/usr/lib/",
                work_dir);
    execute_cmd("/bin/cp /usr/bin/php* %s/", work_dir);
    execute_cmd("chmod +rx %s/Main.php", work_dir);

}

void copy_perl_runtime(char *work_dir) {

    copy_shell_runtime(work_dir);
    execute_cmd("/bin/mkdir %s/usr", work_dir);
    execute_cmd("/bin/mkdir %s/usr/lib", work_dir);
    execute_cmd("/bin/cp /usr/lib/libperl* %s/usr/lib/", work_dir);
    execute_cmd("/bin/cp /usr/bin/perl* %s/", work_dir);

}

void copy_freebasic_runtime(char *work_dir) {

    copy_shell_runtime(work_dir);
    execute_cmd("/bin/mkdir -p %s/usr/local/lib", work_dir);
    execute_cmd("/bin/mkdir -p %s/usr/local/bin", work_dir);
    execute_cmd("/bin/cp /usr/local/lib/freebasic %s/usr/local/lib/", work_dir);
    execute_cmd("/bin/cp /usr/local/bin/fbc %s/", work_dir);
    execute_cmd("/bin/cp -a /lib32/* %s/lib/", work_dir);

}

void copy_mono_runtime(char *work_dir) {

    copy_shell_runtime(work_dir);
    execute_cmd("/bin/mkdir %s/usr", work_dir);
    execute_cmd("/bin/mkdir %s/proc", work_dir);
    execute_cmd("/bin/mkdir -p %s/usr/lib/mono/2.0", work_dir);
    execute_cmd("/bin/cp -a /usr/lib/mono %s/usr/lib/", work_dir);
    execute_cmd("/bin/mkdir -p %s/usr/lib64/mono/2.0", work_dir);
    execute_cmd("/bin/cp -a /usr/lib64/mono %s/usr/lib64/", work_dir);

    execute_cmd("/bin/cp /usr/lib/libgthread* %s/usr/lib/", work_dir);

    execute_cmd("/bin/mount -o bind /proc %s/proc", work_dir);
    execute_cmd("/bin/cp /usr/bin/mono* %s/", work_dir);

    execute_cmd("/bin/cp /usr/lib/libgthread* %s/usr/lib/", work_dir);
    execute_cmd("/bin/cp /lib/libglib* %s/lib/", work_dir);
    execute_cmd("/bin/cp /lib/tls/i686/cmov/lib* %s/lib/tls/i686/cmov/",
                work_dir);
    execute_cmd("/bin/cp /lib/libpcre* %s/lib/", work_dir);
    execute_cmd("/bin/cp /lib/ld-linux* %s/lib/", work_dir);
    execute_cmd("/bin/cp /lib64/ld-linux* %s/lib64/", work_dir);
    execute_cmd("/bin/mkdir -p %s/home/judge", work_dir);
    execute_cmd("/bin/chown judge %s/home/judge", work_dir);
    execute_cmd("/bin/mkdir -p %s/etc", work_dir);
    execute_cmd("/bin/grep judge /etc/passwd>%s/etc/passwd", work_dir);

}

void copy_lua_runtime(char *work_dir) {

    copy_shell_runtime(work_dir);
    execute_cmd("/bin/mkdir -p %s/usr/local/lib", work_dir);
    execute_cmd("/bin/mkdir -p %s/usr/local/bin", work_dir);
    execute_cmd("/bin/cp /usr/bin/lua %s/", work_dir);

}

void copy_js_runtime(char *work_dir) {

    //  copy_shell_runtime(work_dir);
    execute_cmd("/bin/mkdir -p %s/usr/lib /lib/i386-linux-gnu/", work_dir);
    execute_cmd("/bin/cp /lib/i386-linux-gnu/libz.so.*  %s/lib/i386-linux-gnu/", work_dir);
    execute_cmd("/bin/cp /usr/lib/i386-linux-gnu/libcares.so.*  %s/lib/i386-linux-gnu/", work_dir);
    execute_cmd("/bin/cp /usr/lib/libv8.so.*  %s/lib/i386-linux-gnu/", work_dir);
    execute_cmd("/bin/cp /lib/i386-linux-gnu/libssl.so.*  %s/lib/i386-linux-gnu/", work_dir);
    execute_cmd("/bin/cp /lib/i386-linux-gnu/libcrypto.so.*  %s/lib/i386-linux-gnu/", work_dir);
    execute_cmd("/bin/cp /lib/i386-linux-gnu/libdl.so.*  %s/lib/i386-linux-gnu/", work_dir);
    execute_cmd("/bin/cp /lib/i386-linux-gnu/librt.so.*  %s/lib/i386-linux-gnu/", work_dir);
    execute_cmd("/bin/cp /usr/lib/i386-linux-gnu/libstdc++.so.*  %s/lib/i386-linux-gnu/", work_dir);
    execute_cmd("/bin/cp /lib/i386-linux-gnu/libpthread.so.*  %s/lib/i386-linux-gnu/", work_dir);
    execute_cmd("/bin/cp /lib/i386-linux-gnu/libc.so.6  %s/lib/i386-linux-gnu/", work_dir);
    execute_cmd("/bin/cp /lib/i386-linux-gnu/libm.so.6  %s/lib/i386-linux-gnu/", work_dir);
    execute_cmd("/bin/cp /lib/i386-linux-gnu/libgcc_s.so.1  %s/lib/i386-linux-gnu/", work_dir);
    execute_cmd("/bin/cp /lib/ld-linux.so.*  %s/lib/i386-linux-gnu/", work_dir);

    execute_cmd("/bin/mkdir -p %s/usr/lib /lib/x86_64-linux-gnu/", work_dir);
    execute_cmd("/bin/cp /lib/x86_64-linux-gnu/libz.so.*  %s/lib/x86_64-linux-gnu/", work_dir);
    execute_cmd("/bin/cp /usr/lib/x86_64-linux-gnu/libcares.so.*  %s/lib/x86_64-linux-gnu/", work_dir);
    execute_cmd("/bin/cp /usr/lib/libv8.so.*  %s/lib/x86_64-linux-gnu/", work_dir);
    execute_cmd("/bin/cp /lib/x86_64-linux-gnu/libssl.so.*  %s/lib/x86_64-linux-gnu/", work_dir);
    execute_cmd("/bin/cp /lib/x86_64-linux-gnu/libcrypto.so.*  %s/lib/x86_64-linux-gnu/", work_dir);
    execute_cmd("/bin/cp /lib/x86_64-linux-gnu/libdl.so.*  %s/lib/x86_64-linux-gnu/", work_dir);
    execute_cmd("/bin/cp /lib/x86_64-linux-gnu/librt.so.*  %s/lib/x86_64-linux-gnu/", work_dir);
    execute_cmd("/bin/cp /usr/lib/x86_64-linux-gnu/libstdc++.so.*  %s/lib/x86_64-linux-gnu/", work_dir);
    execute_cmd("/bin/cp /lib/x86_64-linux-gnu/libpthread.so.*  %s/lib/x86_64-linux-gnu/", work_dir);
    execute_cmd("/bin/cp /lib/x86_64-linux-gnu/libc.so.6  %s/lib/x86_64-linux-gnu/", work_dir);
    execute_cmd("/bin/cp /lib/x86_64-linux-gnu/libm.so.6  %s/lib/x86_64-linux-gnu/", work_dir);
    execute_cmd("/bin/cp /lib/x86_64-linux-gnu/libgcc_s.so.1  %s/lib/x86_64-linux-gnu/", work_dir);
    execute_cmd("/bin/cp /lib64/ld-linux-x86-64.so.2  %s/lib/x86_64-linux-gnu/", work_dir);

    execute_cmd("/bin/cp /usr/bin/nodejs %s/", work_dir);

}

void run_solution(int &lang, char *work_dir, double &time_lmt, double &usedtime,
                  int &mem_lmt) {
    nice(19);
    // now the user is "judger"
    chdir(work_dir);
    // open the files
    freopen("data.in", "r", stdin);
    //if(!DEBUG)
    //{
    freopen("user.out", "w", stdout);
    freopen("error.out", "a+", stderr);
    //}
    // trace me
    if (use_ptrace)
        ptrace(PTRACE_TRACEME, 0, NULL, NULL);
    // run me
    if (lang != 3)
        chroot(work_dir);

    while (setgid(1536) != 0)
        sleep(1);
    while (setuid(1536) != 0)
        sleep(1);
    while (setresuid(1536, 1536, 1536) != 0)
        sleep(1);

    //      char java_p1[BUFFER_SIZE], java_p2[BUFFER_SIZE];
    // child
    // set the limit
    struct rlimit LIM; // time limit, file limit& memory limit
    // time limit
    if (oi_mode)
        LIM.rlim_cur = time_lmt + 1;
    else
        LIM.rlim_cur = ((time_lmt - usedtime / 1000) + 1);
    LIM.rlim_max = LIM.rlim_cur;
    //if(DEBUG) printf("LIM_CPU=%d",(int)(LIM.rlim_cur));
    setrlimit(RLIMIT_CPU, &LIM);
    alarm(0);
    alarm(time_lmt * 10);

    // file limit
    LIM.rlim_max = (STD_F_LIM + STD_MB);
    LIM.rlim_cur = (STD_F_LIM);
    setrlimit(RLIMIT_FSIZE, &LIM);
    // proc limit
    switch (lang) {
        case GO:
            LIM.rlim_cur = LIM.rlim_max = 280;
            break;
        case JAVA:  //java
        case RUBY:  //ruby
        case CSHARP: //C#
        case SCHEMA:
        case JAVASCRIPT:
            LIM.rlim_cur = LIM.rlim_max = 80;
            break;
        case BASH: //bash
            LIM.rlim_cur = LIM.rlim_max = 3;
            break;
        default:
            LIM.rlim_cur = LIM.rlim_max = 1;
    }

    setrlimit(RLIMIT_NPROC, &LIM);

    // set the stack
    LIM.rlim_cur = (STD_MB << 6);
    LIM.rlim_max = (STD_MB << 6);
    setrlimit(RLIMIT_STACK, &LIM);
    // set the memory
    LIM.rlim_cur = (STD_MB * mem_lmt / 2 * 3);
    LIM.rlim_max = (STD_MB * mem_lmt * 2);
    if (lang < JAVA)
        setrlimit(RLIMIT_AS, &LIM);

    switch (lang) {
        case C11:
        case CPP17:
        case PASCAL:
        case OBJC:
        case FREEBASIC:
        case CLANG:
        case CLANGPP:
        case GO:
        case CPP11:
        case CPP98:
        case C99:
            execl("./Main", "./Main", (char *) nullptr);
            break;
        case JAVA:
            sprintf(java_xms, "-Xmx%dM", mem_lmt);
            //sprintf(java_xmx, "-XX:MaxPermSize=%dM", mem_lmt);
            sprintf(java_xmx, "-XX:MaxMetaspaceSize=%dM", mem_lmt);
            execl("/usr/bin/java", "/usr/bin/java", java_xms, java_xmx,
                  "-Djava.security.manager",
                  "-Djava.security.policy=./java.policy", "Main", (char *) nullptr);
            break;
        case RUBY:
            //system("/ruby Main.rb<data.in");
            execl("/ruby", "/ruby", "Main.rb", (char *) nullptr);
            break;
        case BASH: //bash
            execl("/bin/bash", "/bin/bash", "Main.sh", (char *) nullptr);
            break;
        case PYTHON2: //Python
            execl("/python2", "/python2", "Main.py", (char *) nullptr);
            break;
        case PHP: //php
            execl("/php", "/php", "Main.php", (char *) nullptr);
            break;
        case PERL: //perl
            execl("/perl", "/perl", "Main.pl", (char *) nullptr);
            break;
        case CSHARP: //Mono C#
            execl("/mono", "/mono", "--debug", "Main.exe", (char *) nullptr);
            break;
        case SCHEMA: //guile
            execl("/guile", "/guile", "Main.scm", (char *) nullptr);
            break;
        case LUA: //guile
            execl("/lua", "/lua", "Main", (char *) nullptr);
            break;
        case JAVASCRIPT: //SpiderMonkey
            execl("/nodejs", "/nodejs", "Main.js", (char *) nullptr);
            break;
        case PYTHON3://python 3
            //system("./python3 Main.py<data.in>>user.out");
            execl("/python3", "/python3", "Main.py", (char *) nullptr);
            break;
    }
    //sleep(1);
    fflush(stderr);
    exit(0);
}

int fix_python_mis_judge(char *work_dir, int &ACflg, int &topmemory,
                         int mem_lmt) {
    int comp_res = execute_cmd(
            "/bin/grep 'MemoryError'  %s/error.out", work_dir);

    if (!comp_res) {
        printf("Python need more Memory!");
        ACflg = OJ_ML;
        topmemory = mem_lmt * STD_MB;
    }

    return comp_res;
}

int fix_java_mis_judge(char *work_dir, int &ACflg, int &topmemory,
                       int mem_lmt) {
    int comp_res = OJ_AC;
    execute_cmd("chmod 700 %s/error.out", work_dir);
    if (DEBUG)
        execute_cmd("cat %s/error.out", work_dir);
    comp_res = execute_cmd("/bin/grep 'Exception'  %s/error.out", work_dir);
    if (!comp_res) {
        printf("Exception reported\n");
        ACflg = OJ_RE;
    }
    execute_cmd("cat %s/error.out", work_dir);

    comp_res = execute_cmd(
            "/bin/grep 'java.lang.OutOfMemoryError'  %s/error.out", work_dir);

    if (!comp_res) {
        printf("JVM need more Memory!");
        ACflg = OJ_ML;
        topmemory = mem_lmt * STD_MB;
    }

    if (!comp_res) {
        printf("JVM need more Memory or Threads!");
        ACflg = OJ_ML;
        topmemory = mem_lmt * STD_MB;
    }
    comp_res = execute_cmd("/bin/grep 'Could not create'  %s/error.out",
                           work_dir);
    if (!comp_res) {
        printf("jvm need more resource,tweak -Xmx(OJ_JAVA_BONUS) Settings");
        ACflg = OJ_RE;
        //topmemory=0;
    }
    return comp_res;
}


int special_judge(char *oj_home, int problem_id, char *infile, char *outfile,
                  char *userfile, char *usercode) {

    pid_t pid;
    printf("pid=%d\n", problem_id);
    printf("%s\n", usercode);
    string outfiled(global_work_dir);
    outfiled += "usercode.code";
    ofstream userout(outfiled);
    userout << usercode;
    userout.close();
    fstream users(userfile);
    string tmp;
    while (getline(users, tmp))
        cout << tmp << endl;
    pid = fork();
    int ret = 0;
    if (pid == 0) {

        while (setgid(1536) != 0)
            sleep(1);
        while (setuid(1536) != 0)
            sleep(1);
        while (setresuid(1536, 1536, 1536) != 0)
            sleep(1);
        freopen("spjresult.out", "w", stdout);

        struct rlimit LIM{}; // time limit, file limit& memory limit

        LIM.rlim_cur = 5;
        LIM.rlim_max = LIM.rlim_cur;
        setrlimit(RLIMIT_CPU, &LIM);
        alarm(0);
        alarm(10);

        // file limit
        LIM.rlim_max = static_cast<rlim_t>(STD_F_LIM + STD_MB);
        LIM.rlim_cur = static_cast<rlim_t>(STD_F_LIM);
        setrlimit(RLIMIT_FSIZE, &LIM);
        string dir = oj_home;
        dir += "/data/" + to_string(problem_id) + "/";
        if (~access((dir + "spj").c_str(), 0)) {
            ret = execute_cmd((dir + "spj %s %s %s %s").c_str(),
                              infile, outfile, userfile, outfiled.c_str());
        } else if (~access((dir + "spj.js").c_str(), 0)) {
            ret = execute_cmd(("node " + dir + "spj.js %s %s %s %s").c_str(),
                              infile, outfile, userfile, outfiled.c_str());
        } else if (~access((dir + "spj.py").c_str(), 0)) {
            ret = execute_cmd(("python3 " + dir + "spj.py %s %s %s %s").c_str(),
                              infile, outfile, userfile, outfiled.c_str());
        }
        freopen("/dev/tty", "w", stdout);
        ifstream spjout("spjresult.out");
        if (get_file_size("spjresult.out")) {
            spjout >> ret;
        }
        if (DEBUG)
            printf("spj1=%d\n", ret);
        exit(ret);
    } else {
        int status;

        waitpid(pid, &status, 0);
        ret = WEXITSTATUS(status);
        if (DEBUG)
            printf("spj2=%d\n", ret);
    }
    return ret;
}

void judge_solution(int &ACflg, double &usedtime, double time_lmt, int isspj,
                    int p_id, char *infile, char *outfile, char *userfile, char *usercode, int &PEflg,
                    int lang, char *work_dir, int &topmemory, int mem_lmt,
                    int solution_id, int num_of_test) {
    //usedtime-=1000;
    int comp_res;
    if (!oi_mode)
        num_of_test = static_cast<int>(1.0);
    if (ACflg == OJ_AC
        && usedtime > time_lmt * 1000 * (use_max_time ? 1 : num_of_test))
        ACflg = OJ_TL;
    if (topmemory > mem_lmt * STD_MB)
        ACflg = OJ_ML; //issues79
    // compare
    if (ACflg == OJ_AC) {
        if (isspj) {
            comp_res = special_judge(oj_home, p_id, infile, outfile, userfile, usercode);

            if (comp_res == 0)
                comp_res = OJ_AC;
            /*
        else {
            if (DEBUG)
                printf("fail test %s\n", infile);
            comp_res = OJ_WA;
        }*/
        } else {
            comp_res = compare(outfile, userfile);
        }
        if (comp_res == OJ_WA) {
            ACflg = OJ_WA;
            if (DEBUG)
                printf("fail test %s\n", infile);
        } else if (comp_res == OJ_PE)
            PEflg = OJ_PE;
        ACflg = comp_res;
    }
    //jvm popup messages, if don't consider them will get miss-WrongAnswer
    if (lang == JAVA) {
        comp_res = fix_java_mis_judge(work_dir, ACflg, topmemory, mem_lmt);
    }
    if (lang == PYTHON2 || lang == PYTHON3) {
        comp_res = fix_python_mis_judge(work_dir, ACflg, topmemory, mem_lmt);
    }
}

int get_page_fault_mem(struct rusage &ruse, pid_t &pidApp) {
    //java use pagefault
    int m_vmpeak, m_vmdata, m_minflt;
    m_minflt = static_cast<int>(ruse.ru_minflt * getpagesize());
    /*
    if (0 && DEBUG) {
        m_vmpeak = get_proc_status(pidApp, "VmPeak:");
        m_vmdata = get_proc_status(pidApp, "VmData:");
        printf("VmPeak:%d KB VmData:%d KB minflt:%d KB\n", m_vmpeak, m_vmdata,
               m_minflt >> 10);
    }
     */
    return m_minflt;
}

void print_runtimeerror(const char *err) {
    FILE *ferr = fopen("error.out", "a+");
    fprintf(ferr, "Runtime Error:%s\n", err);
    fclose(ferr);
}


void watch_solution(pid_t pidApp, char *infile, int &ACflg, int isspj,
                    char *userfile, char *outfile, int solution_id, int lang,
                    int &topmemory, int mem_lmt, double &usedtime, double time_lmt, int &p_id,
                    int &PEflg, char *work_dir) {
    // parent
    int tempmemory;

    if (DEBUG)
        printf("pid=%d judging %s\n", pidApp, infile);

    int status, sig, exitcode;
    struct user_regs_struct reg{};
    struct rusage ruse{};
    if (topmemory == 0)
        topmemory = get_proc_status(pidApp, "VmRSS:") << 10;
    while (true) {
        // check the usage

        wait4(pidApp, &status, 0, &ruse);

        //jvm gc ask VM before need,so used kernel page fault times and page size
        if (lang == JAVA || lang == PHP || lang == JAVASCRIPT || lang == CSHARP || lang == GO) {
            tempmemory = get_page_fault_mem(ruse, pidApp);
        } else {        //other use VmPeak
            tempmemory = get_proc_status(pidApp, "VmPeak:") << 10;
        }
        if (tempmemory > topmemory)
            topmemory = tempmemory;
        if (topmemory > mem_lmt * STD_MB) {
            if (DEBUG)
                printf("out of memory %d\n", topmemory);
            if (ACflg == OJ_AC)
                ACflg = OJ_ML;
            ptrace(PTRACE_KILL, pidApp, NULL, NULL);
            break;
        }
        //sig = status >> 8;/*status >> 8 Ã¥Â·Â®Ã¤Â¸ÂÃ¥Â¤Å¡Ã¦ËÂ¯EXITCODE*/

        if (WIFEXITED(status))
            break;
        if (get_file_size("error.out")) {
            if (DEBUG) {
                cerr << "Catch error:" << endl;
                string tmp;
                fstream err("error.out");
                while (getline(err, tmp))
                    cerr << tmp << endl;
            }
            //ACflg = OJ_RE;
            fstream file("error.out", ios::ate);
            stringstream buffer;
            buffer << file.rdbuf();
            string contents(buffer.str());
            if (contents.find("Killed") != contents.npos) {
                write_log(contents.c_str());
                print_runtimeerror(contents.c_str());
                //ptrace(PTRACE_KILL, pidApp, NULL, NULL);
                //print_runtimeerror(contents.c_str());
                if (!oi_mode)
                    break;
            }
        }
        if ((lang < RUBY || lang == CSHARP) && get_file_size("error.out") && !oi_mode) {
            ACflg = OJ_RE;
            //addreinfo(solution_id);
            ptrace(PTRACE_KILL, pidApp, NULL, NULL);
            break;
        }


        if (!isspj
            && get_file_size(userfile)
               > get_file_size(outfile) * 2 + 1024) {
            ACflg = OJ_OL;
            ptrace(PTRACE_KILL, pidApp, NULL, NULL);
            break;
        }

        exitcode = WEXITSTATUS(status);
        /*exitcode == 5 waiting for next CPU allocation          * ruby using system to run,exit 17 ok
         *  */
        if ((lang >= JAVA && exitcode == 17) || exitcode == 0x05 || exitcode == 0)
            //go on and on
            ;
        else {

            if (DEBUG) {
                printf("status>>8=%d\n", exitcode);

            }
            //psignal(exitcode, NULL);

            if (ACflg == OJ_AC) {
                switch (exitcode) {
                    case SIGCHLD:
                    case SIGALRM:
                        alarm(0);
                    case SIGKILL:
                    case SIGXCPU:
                        ACflg = OJ_TL;
                        break;
                    case SIGXFSZ:
                        ACflg = OJ_OL;
                        break;
                    default:
                        ACflg = OJ_RE;
                }
                print_runtimeerror(strsignal(exitcode));
            }
            ptrace(PTRACE_KILL, pidApp, NULL, NULL);
            break;
        }
        if (WIFSIGNALED(status)) {
            /*  WIFSIGNALED: if the process is terminated by signal
             *
             *  psignal(int sig, char *s)，like perror(char *s)，print out s, with error msg from system of sig
             * sig = 5 means Trace/breakpoint trap
             * sig = 11 means Segmentation fault
             * sig = 25 means File size limit exceeded
             */
            sig = WTERMSIG(status);
            if (DEBUG) {
                printf("WTERMSIG=%d\n", sig);
                psignal(sig, nullptr);
            }
            if (ACflg == OJ_AC) {
                switch (sig) {
                    case SIGCHLD:
                    case SIGALRM:
                        alarm(0);
                    case SIGKILL:
                    case SIGXCPU:
                        ACflg = OJ_TL;
                        break;
                    case SIGXFSZ:
                        ACflg = OJ_OL;
                        break;

                    default:
                        ACflg = OJ_RE;
                }
                print_runtimeerror(strsignal(sig));
            }
            break;
        }
        /*     comment from http://www.felix021.com/blog/read.php?1662

         WIFSTOPPED: return true if the process is paused or stopped while ptrace is watching on it
         WSTOPSIG: get the signal if it was stopped by signal
         */

        // check the system calls
        ptrace(PTRACE_GETREGS, pidApp, NULL, &reg);
        if (call_counter[reg.REG_SYSCALL]) {
            //call_counter[reg.REG_SYSCALL]--;
        } else if (record_call) {
            call_counter[reg.REG_SYSCALL] = 1;

        } else { //do not limit JVM syscall for using different JVM
            ACflg = OJ_RE;
            char error[BUFFER_SIZE];
            sprintf(error,
                    "[ERROR] A Not allowed system call: runid:%d CALLID:%ld\n"
                    " TO FIX THIS , ask admin to add the CALLID into corresponding LANG_XXV[] located at okcalls32/64.h ,\n"
                    "and recompile judge_client. \n"
                    "if you are admin and you don't know what to do ,\n"
                    " tech support can be found on http://hustoj.taobao.com\n",
                    solution_id, (long) reg.REG_SYSCALL);

            write_log(error);
            print_runtimeerror(error);
            ptrace(PTRACE_KILL, pidApp, NULL, NULL);
        }


        ptrace(PTRACE_SYSCALL, pidApp, NULL, NULL);
    }
    usedtime += (ruse.ru_utime.tv_sec * 1000 + ruse.ru_utime.tv_usec / 1000);
    usedtime += (ruse.ru_stime.tv_sec * 1000 + ruse.ru_stime.tv_usec / 1000);

    //clean_session(pidApp);
}

void clean_workdir(char *work_dir) {
    umount(work_dir);
    if (DEBUG) {
        execute_cmd("/bin/rm -rf %s/log/*", work_dir);
        execute_cmd("mkdir %s/log/", work_dir);
        execute_cmd("/bin/mv %s/* %s/log/", work_dir, work_dir);
    } else {
        execute_cmd("mkdir %s/log/", work_dir);
        execute_cmd("/bin/mv %s/* %s/log/", work_dir, work_dir);
        execute_cmd("/bin/rm -rf %s/log/*", work_dir);
    }

}

void init_parameters(int argc, char **argv, int &solution_id,
                     int &runner_id) {
    if (argc < 3) {
        fprintf(stderr, "Usage:%s solution_id runner_id.\n", argv[0]);
        fprintf(stderr, "Multi:%s solution_id runner_id judge_base_path.\n",
                argv[0]);
        fprintf(stderr,
                "Debug:%s solution_id runner_id judge_base_path debug.\n",
                argv[0]);
        exit(1);
    }
    DEBUG = (argc > 4);
    record_call = (argc > 5);
    if (argc > 5) {
        strcpy(LANG_NAME, argv[5]);
    }
    if (argc > 3)
        strcpy(oj_home, argv[3]);
    else
        strcpy(oj_home, "/home/judge");

    chdir(oj_home); // change the dir// init our work

    solution_id = atoi(argv[1]);
    runner_id = atoi(argv[2]);
    judger_number = runner_id;
}

int get_sim(int solution_id, int lang, int pid, int &sim_s_id) {
    char src_pth[BUFFER_SIZE];
    //char cmd[BUFFER_SIZE];
    sprintf(src_pth, "Main.%s", lang_ext[lang]);

    int sim = execute_cmd("/usr/bin/sim.sh %s %d", src_pth, pid);
    if (!sim) {
        execute_cmd("/bin/mkdir ../data/%d/ac/", pid);

        execute_cmd("/bin/cp %s ../data/%d/ac/%d.%s", src_pth, pid, solution_id,
                    lang_ext[lang]);
        //c cpp will
        if (lang == 0)
            execute_cmd("/bin/ln ../data/%d/ac/%d.%s ../data/%d/ac/%d.%s", pid,
                        solution_id, lang_ext[lang], pid, solution_id,
                        lang_ext[lang + 1]);
        if (lang == 1)
            execute_cmd("/bin/ln ../data/%d/ac/%d.%s ../data/%d/ac/%d.%s", pid,
                        solution_id, lang_ext[lang], pid, solution_id,
                        lang_ext[lang - 1]);

    } else {

        FILE *pf;
        pf = fopen("sim", "r");
        if (pf) {
            fscanf(pf, "%d%d", &sim, &sim_s_id);
            fclose(pf);
        }

    }
    MYSQL_RES *res;
    MYSQL_ROW row;
    string sql = "SELECT user_id FROM solution WHERE solution_id=" + to_string(solution_id);
    cout << sql << endl;
    mysql_real_query(conn, sql.c_str(), sql.length());
    res = mysql_store_result(conn);
    string uid;
    row = mysql_fetch_row(res);
    if (row)
        uid = row[0];
    sql = "SELECT user_id FROM solution WHERE solution_id=" + to_string(sim_s_id);
    cout << uid << endl;
    mysql_real_query(conn, sql.c_str(), sql.length());
    res = mysql_store_result(conn);
    string cpid;
    row = mysql_fetch_row(res);
    if (row)
        cpid = row[0];
    cout << cpid << endl;
    if (uid == cpid)
        sim = 0;
    if (solution_id <= sim_s_id)
        sim = 0;
    return sim;
}

void mk_shm_workdir(char *work_dir) {
    char shm_path[BUFFER_SIZE];
    sprintf(shm_path, "/dev/shm/hustoj/%s", work_dir);
    execute_cmd("/bin/mkdir -p %s", shm_path);
    execute_cmd("/bin/ln -s %s %s/", shm_path, oj_home);
    execute_cmd("/bin/chown judge %s ", shm_path);
    execute_cmd("chmod 755 %s ", shm_path);
    //sim need a soft link in shm_dir to work correctly
    sprintf(shm_path, "/dev/shm/hustoj/%s/", oj_home);
    execute_cmd("/bin/ln -s %s/data %s", oj_home, shm_path);

}


void print_call_array() {
    printf("int LANG_%sV[256]={", LANG_NAME);
    int i = 0;
    for (i = 0; i < call_array_size; i++) {
        if (call_counter[i]) {
            printf("%d,", i);
        }
    }
    printf("0};\n");

    printf("int LANG_%sC[256]={", LANG_NAME);
    for (i = 0; i < call_array_size; i++) {
        if (call_counter[i]) {
            printf("HOJ_MAX_LIMIT,");
        }
    }
    printf("0};\n");

}

int main(int argc, char **argv) {
    webSocket.connect("ws://localhost:5100");
    char work_dir[BUFFER_SIZE];
    char usercode[BUFFER_SIZE];
    char user_id[BUFFER_SIZE];
    solution_id = 1000;
    int runner_id = 0;
    int p_id, mem_lmt, lang, isspj, sim, sim_s_id = 0;
    double max_case_time = 0;
    double time_lmt;
    init_parameters(argc, argv, solution_id, runner_id);
    init_mysql_conf();
    if (!init_mysql_conn()) {
        exit(0); //exit if mysql is down
    }
    //set work directory to start running & judging
    sprintf(work_dir, "%s/run%s/", oj_home, argv[2]);
    global_work_dir = work_dir;
    clean_workdir(work_dir);
    if (shm_run)
        mk_shm_workdir(work_dir);
    chdir(work_dir);
    get_solution_info(solution_id, p_id, user_id, lang);
    //get the limit
    if (p_id <= 0) {//Is custom input
        time_lmt = 5;
        mem_lmt = 128;
        isspj = 0;
    } else {
        get_problem_info(p_id, time_lmt, mem_lmt, isspj);
    }
    //copy source file
    get_solution(solution_id, work_dir, lang, usercode);
    //java is lucky
    if (lang >= JAVA && lang != OBJC && lang != CLANG && lang != CLANGPP &&
        lang < CPP11) {  // Clang Clang++ not VM or Script
        // the limit for java
        time_lmt = time_lmt * java_time_bonus;
        mem_lmt = mem_lmt + java_memory_bonus;
        // copy java.policy
        if (lang == JAVA) {
            execute_cmd("/bin/cp %s/etc/java0.policy %s/java.policy", oj_home, work_dir);
            execute_cmd("chmod 755 %s/java.policy", work_dir);
            execute_cmd("chown judge %s/java.policy", work_dir);
        }
    }

    //never bigger than judged set value;
    if (time_lmt > 300 || time_lmt < 0)
        time_lmt = 300;
    if (mem_lmt > 1024 || mem_lmt < 1)
        mem_lmt = 1024;
    if (DEBUG)
        printf("time: %f mem: %d\n", time_lmt, mem_lmt);
    // compile
    //      printf("%s\n",cmd);
    // set the result to compiling
    int Compile_OK;
    Compile_OK = compile(lang, work_dir);
    if (Compile_OK != 0) {
        addceinfo(solution_id);
        string _compile_info, tmp;
        fstream ceinformation("ce.txt");
        while (getline(ceinformation, tmp)) {
            _compile_info += tmp + "\n";
        }
        if (webSocket.isconnected()) {
            webSocket << ws_send(solution_id, OJ_CE, 1, 0, 0, 0, 0, "", _compile_info);
        }
        update_solution(solution_id, OJ_CE, 0, 0, 0, 0, 0.0);
        update_user(user_id);
        update_problem(p_id);
        mysql_close(conn);
        clean_workdir(work_dir);
        write_log("compile error");
        exit(0);
    } else {
        update_solution(solution_id, OJ_RI, 0, 0, 0, 0, 0.0);
        umount(work_dir);
    }
    //exit(0);
    // run
    char fullpath[BUFFER_SIZE];
    char infile[BUFFER_SIZE];
    char outfile[BUFFER_SIZE];
    char userfile[BUFFER_SIZE];
    sprintf(fullpath, "%s/data/%d", oj_home, p_id); // the fullpath of data dir

    // open DIRs
    DIR *dp;
    dirent *dirp;
    // using http to get remote test data files
    if (p_id > 0 && (dp = opendir(fullpath)) == nullptr) {

        write_log("No such dir:%s!\n", fullpath);
        mysql_close(conn);
        exit(-1);
    }

    int ACflg, PEflg;
    ACflg = PEflg = OJ_AC;
    int namelen;
    int topmemory = 0;
    double usedtime = 0;

    //create chroot for ruby bash python
    switch (lang) {
        case RUBY:
            copy_ruby_runtime(work_dir);
            break;
        case BASH:
            copy_bash_runtime(work_dir);
            break;
        case PYTHON2:
        case PYTHON3:
            copy_python_runtime(work_dir);
            break;
        case PHP:
            copy_php_runtime(work_dir);
            break;
        case PERL:
            copy_perl_runtime(work_dir);
            break;
        case CSHARP:
            copy_mono_runtime(work_dir);
            break;
        case OBJC:
            copy_objc_runtime(work_dir);
            break;
        case FREEBASIC:
            copy_freebasic_runtime(work_dir);
            break;
        case SCHEMA:
            copy_guile_runtime(work_dir);
            break;
        case LUA:
            copy_lua_runtime(work_dir);
            break;
        case JAVASCRIPT:
            copy_js_runtime(work_dir);
        default:
            break;
    }
    // read files and run
    // read files and run
    // read files and run
    double pass_rate = 0.0;
    int num_of_test = 0;
    int finalACflg = ACflg;
    if (p_id <= 0) {  //custom input running
        printf("running a custom input...\n");
        get_custominput(solution_id, work_dir);
        init_syscalls_limits(lang);
        if (webSocket.isconnected()) {
            webSocket << ws_send(solution_id, 3, 0, 0, 0, 0, 0);
        }
        pid_t pidApp = fork();

        if (pidApp == 0) {
            run_solution(lang, work_dir, time_lmt, usedtime, mem_lmt);
        } else {
            watch_solution(pidApp, infile, ACflg, isspj, userfile, outfile,
                           solution_id, lang, topmemory, mem_lmt, usedtime, time_lmt,
                           p_id, PEflg, work_dir);

        }
        if (ACflg == OJ_TL) {
            usedtime = time_lmt * 1000;
        }
        if (ACflg == OJ_RE) {
            if (DEBUG)
                printf("add RE info of %d..... \n", solution_id);
            addreinfo(solution_id);
        } else {
            addcustomout(solution_id);
        }
        if (webSocket.isconnected()) {
            string test_run_out;
            char *end;
            char reinfo[(1 << 16)], *rend;
            FILE *fp = fopen("user.out", "re");
            rend = reinfo;
            while (fgets(reinfo, 1 << 16, fp)) {
                string tmp(reinfo);
                test_run_out += tmp;
                if (test_run_out.length() > 4096)
                    break;
            }
            fclose(fp);

            cout << "come here4" << endl;
            if (test_run_out.length() > 4096)
                test_run_out = test_run_out.substr(0, 4096);
            cout << "come here5" << endl;
            webSocket << ws_send(solution_id, OJ_TR, 1, usedtime, topmemory >> 10, 0, 0, test_run_out);
        }
        cout << "come here3" << endl;
        update_solution(solution_id, OJ_TR, usedtime, topmemory >> 10, 0, 0, 0);
        clean_workdir(work_dir);
        exit(0);
    }

    if (webSocket.isconnected()) {
        webSocket << ws_send(solution_id, 3, 0, 0, 0, 0, 0);
    }
    int pass_point = 0;
    for (; (oi_mode || ACflg == OJ_AC || ACflg == OJ_PE) && (dirp = readdir(dp)) != nullptr;) {

        namelen = isInFile(dirp->d_name); // check if the file is *.in or not
        if (namelen == 0)
            continue;
        prepare_files(dirp->d_name, namelen, infile, p_id, work_dir, outfile,
                      userfile, runner_id);
        init_syscalls_limits(lang);

        pid_t pidApp = fork();

        if (pidApp == 0) {
            printf("Running solution\n");
            run_solution(lang, work_dir, time_lmt, usedtime, mem_lmt);
        } else {
            ++num_of_test;
            if(DEBUG)
            {
                cout<<"Run test point:"<<num_of_test<<endl;
            }
            watch_solution(pidApp, infile, ACflg, isspj, userfile, outfile,
                           solution_id, lang, topmemory, mem_lmt, usedtime, time_lmt,
                           p_id, PEflg, work_dir);

            judge_solution(ACflg, usedtime, time_lmt, isspj, p_id, infile,
                           outfile, userfile, usercode, PEflg, lang, work_dir, topmemory,
                           mem_lmt, solution_id, num_of_test);
            if (use_max_time) {
                max_case_time =
                        usedtime > max_case_time ? usedtime : max_case_time;
                usedtime = 0;
            }
            //clean_session(pidApp);
        }

        if (ACflg == OJ_AC) {
            ++pass_point;
            //MYSQL_RES* res;
            //MYSQL_ROW row;
            string sql = "UPDATE solution set pass_point=" + to_string(pass_point) + " WHERE solution_id=" +
                         to_string(solution_id);
            mysql_real_query(conn, sql.c_str(), sql.length());
            //res=mysql_store_result(conn);
            //string
        }

        if (oi_mode) {
            if (ACflg == OJ_AC) {
                ++pass_rate;
            }
            if (finalACflg < ACflg) {
                finalACflg = ACflg;
            }

            ACflg = OJ_AC;
        }

        if (webSocket.isconnected()) {
            webSocket << ws_send(solution_id, 3, 0, 0, 0, pass_point, pass_rate / num_of_test);
        }
    }
    if (ACflg == OJ_AC && PEflg == OJ_PE)
        ACflg = OJ_PE;
    if (sim_enable && ACflg == OJ_AC && (!oi_mode || finalACflg == OJ_AC)
        && (lang < BASH || lang == CLANG || lang == CLANGPP || lang >= CPP11)) { //bash don't supported
        sim = get_sim(solution_id, lang, p_id, sim_s_id);
    } else {
        sim = 0;
    }
    //if(ACflg == OJ_RE)addreinfo(solution_id);

    if ((oi_mode && finalACflg == OJ_RE) || ACflg == OJ_RE) {
        if (DEBUG)
            printf("add RE info of %d..... \n", solution_id);
        addreinfo(solution_id);
    }
    if (use_max_time) {
        usedtime = max_case_time;
    }
    if (ACflg == OJ_TL) {
        usedtime = time_lmt * 1000;
    }

    if (webSocket.isconnected()) {
        webSocket << ws_send(solution_id, oi_mode ? finalACflg : ACflg, 1, usedtime, topmemory >> 10, pass_point,
                             pass_rate / num_of_test);
    }
    if (oi_mode) {
        if (num_of_test > 0)
            pass_rate /= num_of_test;
        update_solution(solution_id, finalACflg, usedtime, topmemory >> 10, sim,
                        sim_s_id, pass_rate);
    } else {
        update_solution(solution_id, ACflg, usedtime, topmemory >> 10, sim,
                        sim_s_id, 0);
    }
    if ((oi_mode && finalACflg == OJ_WA) || ACflg == OJ_WA) {
        if (DEBUG)
            printf("add diff info of %d..... \n", solution_id);
        if (!isspj)
            adddiffinfo(solution_id);
    }
    update_user(user_id);
    update_problem(p_id);
    clean_workdir(work_dir);

    if (DEBUG)
        write_log("result=%d", oi_mode ? finalACflg : ACflg);
    mysql_close(conn);
    if (record_call) {
        print_call_array();
    }
    closedir(dp);
    return 0;
}