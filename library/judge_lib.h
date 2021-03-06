//
// Created by Ryan Lee on 2018/7/17.
//

#ifndef JUDGE_CLIENT_JUDGE_LIB_H
#define JUDGE_CLIENT_JUDGE_LIB_H

#include <sys/stat.h>
#include <cctype>
#include <iostream>
#include <fstream>
#include <deque>
#include "../model/submission/SubmissionInfo.h"
#include "../model/judge/language/Language.h"
#include "../external/compare/Compare.h"
#include "../external/mysql/MySQLSubmissionAdapter.h"
#include <cstdarg>
#include <future>

using namespace std;
// urlencoded function copied from http://www.geekhideout.com/urlcode.shtml
/* Converts a hex character to its integer value */
extern char from_hex(char ch);

/* Converts an integer value to its hex character*/
extern char to_hex(char code);

/* Returns a url-encoded version of str */
/* IMPORTANT: be sure to free() the returned string after use */
extern char *url_encode(char *str);


extern long get_file_size(const char *filename);

extern bool utf8_check_is_valid(const string &string);

extern int execute_cmd(const char *fmt, ...);

extern int execute_timeout_cmd(long long timeout, const char *fmt, ...);

extern int after_equal(const char *c);


extern void trim(char *c);


extern bool read_buf(char *buf, const char *key, char *value);

extern void read_int(char *buf, const char *key, int &value);

extern const char *getFileNameFromPath(const char *path);

extern bool is_number(const string &s);

extern bool check_valid_presentation_error(const char *ansfile, const char *userfile);


extern void make_diff_out_full(FILE *f1, FILE *f2, int c1, int c2, const char *path);

extern void make_diff_out_simple(FILE *f1, FILE *f2, int c1, int c2, const char *path);


extern void delnextline(char s[]);

extern int isInFile(const char fname[]);

extern void move_to_next_nonspace_character(int &c, FILE *&f, int &ret);

extern bool is_not_character(int c);


extern void umount(char *work_dir);


extern char *escape(char s[], const char t[]);

extern void clean_workdir(char *work_dir);

extern int detectArgType(const char* argument);

extern void write_log(const char* oj_home, const char *_fmt, ...);

extern bool initWebSocketConnection(string&& ip, int port);

extern bool initWebSocketConnection(string& ip, int port);

extern void getSolutionInfoFromSubmissionInfo(SubmissionInfo&, int&, char*, int&);

extern void buildSubmissionInfo(SubmissionInfo&, string&);

extern void removeSubmissionInfo(string&);

extern vector<pair<string, int> >getFileList(const string& path, const function<int(const char*)>&);

extern vector<pair<string, int> >getFileList(const string& path);

extern int get_sim(int solution_id, int lang, int pid, int &sim_s_id);

extern string getFileContent(const string& file);

extern void mk_shm_workdir(char *work_dir);

extern void make_workdir(char* work_dir);

extern int get_proc_status(int pid, const char *mark);

extern void prepare_files(const char *filename, int namelen, char *infile, int &p_id,
                   char *work_dir, char *outfile, char *userfile, int runner_id);

extern void prepare_files_with_id(const char *filename, int namelen, char *infile, int &p_id,
                           char *work_dir, char *outfile, char *userfile, int runner_id, int file_id);

extern void print_runtimeerror(const char *err);

extern void getProblemInfoFromSubmissionInfo(SubmissionInfo& submissionInfo, double& time_lmt, int& mem_lmt, int& isspj);

extern void getCustomInputFromSubmissionInfo(SubmissionInfo& submissionInfo);

extern void getSolutionFromSubmissionInfo(SubmissionInfo& submissionInfo, char* usercode);

extern string getRuntimeInfoContents(const string& filename);

extern Language* getLanguageModel(int language);

extern MySQLSubmissionAdapter* getAdapter();

extern Compare::Compare* getCompareModel();

extern bool isPython(int language);

extern void setRunUser();
#endif //JUDGE_CLIENT_JUDGE_LIB_H