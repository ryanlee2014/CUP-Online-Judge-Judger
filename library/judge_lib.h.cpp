//
// Created by Ryan Lee on 2018/7/17.
//
#include "judge_lib.h"
#include "../header/static_var.h"
#include <dirent.h>
#include <dlfcn.h>
#include <unistd.h>
#include <sstream>
#include <unordered_map>
#include <mutex>
#include <filesystem>

using namespace std;
using namespace std::chrono_literals;
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


long get_file_size(const char *filename) {
    struct stat f_stat{};

    if (stat(filename, &f_stat) == -1) {
        return 0;
    }

    return (long) f_stat.st_size;
}

bool utf8_check_is_valid(const string &string) {
    int c, i, ix, n, j;
    for (i = 0, ix = static_cast<int>(string.length()); i < ix; i++) {
        c = (unsigned char) string[i];
        //if (c==0x09 || c==0x0a || c==0x0d || (0x20 <= c && c <= 0x7e) ) n = 0; // is_printable_ascii
        if (0x00 <= c && c <= 0x7f) n = 0; // 0bbbbbbb
        else if ((c & 0xE0) == 0xC0) n = 1; // 110bbbbb
        else if (c == 0xed && i < (ix - 1) && ((unsigned char) string[i + 1] & 0xa0) == 0xa0)
            return false; //U+d800 to U+dfff
        else if ((c & 0xF0) == 0xE0) n = 2; // 1110bbbb
        else if ((c & 0xF8) == 0xF0) n = 3; // 11110bbb
            //else if (($c & 0xFC) == 0xF8) n=4; // 111110bb //byte 5, unnecessary in 4 byte UTF-8
            //else if (($c & 0xFE) == 0xFC) n=5; // 1111110b //byte 6, unnecessary in 4 byte UTF-8
        else return false;
        for (j = 0; j < n && i < ix; j++) { // n bytes matching 10bbbbbb follow ?
            if ((++i == ix) || (((unsigned char) string[i] & 0xC0) != 0x80))
                return false;
        }
    }
    return true;
}

int execute_cmd(const char *fmt, ...) {
    char cmd[BUFFER_SIZE];
    va_list ap;
    va_start(ap, fmt);
    vsprintf(cmd, fmt, ap);
    int ret = system(cmd);
    va_end(ap);
    return ret;
}

int execute_timeout_cmd(long long timeout, const char *fmt, ...) {
    std::mutex m;
    std::condition_variable cv;
    int retValue = 0;
    bool done = false;
    char cmd[BUFFER_SIZE];
    va_list ap;
    va_start(ap, fmt);
    vsprintf(cmd, fmt, ap);
    std::thread t([&]() {
        retValue = execute_cmd(cmd);
        {
            std::lock_guard<std::mutex> lock(m);
            done = true;
        }
        cv.notify_one();
    });
    t.detach();
    {
        std::unique_lock<std::mutex> l(m);
        if (!cv.wait_for(l, timeout * std::chrono::seconds(1), [&]() { return done; })) {
            throw std::runtime_error("Timeout");
        }
    }
    return retValue;
}

int after_equal(const char *c) {
    int i = 0;
    for (; c[i] && c[i] != '='; ++i);
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
    while (*end && !isspace(*end))
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
}

const char *getFileNameFromPath(const char *path) {
    for (auto i = static_cast<int>(strlen(path)); i >= 0; i--) {
        if (path[i] == '/')
            return &path[i + 1];
    }
    return path;
}

void delnextline(char s[]) {
    auto L = static_cast<int>(strlen(s));
    while (L > 0 && (s[L - 1] == '\n' || s[L - 1] == '\r'))
        s[--L] = 0;
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


int isInFile(const char fname[]) {
    auto l = static_cast<int>(strlen(fname));
    if (l <= 3 || strcmp(fname + l - 3, ".in") != 0)
        return 0;
    else
        return l - 3;
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

int detectArgType(const char *argument) {
    if (argument[0] == '-') {
        if (!strcmp(argument + 1, "language")) {
            return _LANG_NAME;
        } else if (!strcmp(argument + 1, "no_record")) {
            return _NO_RECORD;
        } else if (!strcmp(argument + 1, "dir")) {
            return _DIR;
        } else if (!strcmp(argument + 1, "record")) {
            return _RECORD_CALL;
        } else if (!strcmp(argument + 1, "solution_id")) {
            return _SOLUTION_ID;
        } else if (!strcmp(argument + 1, "runner_id")) {
            return _RUNNER_ID;
        } else if (!strcmp(argument + 1, "admin")) {
            return _ADMIN;
        } else if (!strcmp(argument + 1, "no-sim")) {
            return _NO_SIM;
        } else if (!strcmp(argument + 1, "no-mysql")) {
            return _NO_MYSQL;
        } else if (!strcmp(argument + 1, "judger_id")) {
            return _JUDGER_ID;
        } else if (!strcmp(argument + 1, "stdin")) {
            return _STDIN;
        }
        else {
            return _ERROR;
        }
    } else {
        if (!strcmp(argument, "DEBUG")) {
            return _DEBUG;
        } else {
            return _ERROR;
        }
    }
}

void write_log(const char *oj_home, const char *_fmt, ...) {
    va_list ap;
    char fmt[FOUR * ONE_KILOBYTE];
    strncpy(fmt, _fmt, FOUR * ONE_KILOBYTE);
    char buffer[FOUR * ONE_KILOBYTE];
    //      time_t          t = time(NULL);
    //int l;
    //sprintf(buffer, "%s/log/client.log", oj_home);
    //FILE *fp = fopen(buffer, "ae+");
    //if (fp == nullptr) {
    //    fprintf(stderr, "openfile error!\n");
    //    system("pwd");
    //}
    va_start(ap, _fmt);
    //l =
    vsprintf(buffer, fmt, ap);
    //fprintf(fp, "%s\n", buffer);
    //if (DEBUG) {
    cerr << buffer << endl;
    //printf("%s\n", buffer);
    //}
    va_end(ap);
    //fclose(fp);
}

bool initWebSocketConnection(string &ip, int port) {
    webSocket.connect("ws://" + ip + ":" + to_string(port));
    return webSocket.isConnected();
}

bool initWebSocketConnection(string &&ip, int port) {
    webSocket.connect("ws://" + ip + ":" + to_string(port));
    return webSocket.isConnected();
}

void getSolutionInfoFromSubmissionInfo(SubmissionInfo& submissionInfo, int& p_id, char* user_id, int& lang) {
    p_id = submissionInfo.getProblemId();
    sprintf(user_id, "%s", submissionInfo.getUserId().c_str());
    lang = submissionInfo.getLanguage();
}

void buildSubmissionInfo(SubmissionInfo& submissionInfo, string& judgerId) {
    if (READ_FROM_STDIN) {
        string jsonFromStdin;
        std::getline(cin, jsonFromStdin);
        submissionInfo.readJSON(jsonFromStdin);
    }
    else {
        string filePath = string(oj_home) + "/submission/" + judgerId + ".json";
        submissionInfo.readFromFile(filePath);
    }
}

void removeSubmissionInfo(string& uuid) {
    execute_cmd("/bin/rm -rf %s/submission/%s.json", oj_home, uuid.c_str());
}

namespace {
struct DirListCacheEntry {
    std::filesystem::file_time_type mtime{};
    vector<pair<string, int>> items;
    bool has_mtime = false;
};

int always_true_filter(const char*) {
    return 1;
}

bool is_cacheable_filter(const function<int(const char*)> &func) {
    auto target = func.target<int(*)(const char*)>();
    if (!target) {
        return false;
    }
    return *target == isInFile || *target == always_true_filter;
}

vector<pair<string, int>> read_dir_list(const string &path, const function<int(const char*)> &func) {
    auto* dp = opendir(path.c_str());
    dirent* dirp;
    if (dp == nullptr) {
        write_log(oj_home, "No such dir:%s!\n", path.c_str());
        exit(-1);
    }
    vector<pair<string, int>> inFileList;
    while ((dirp = readdir(dp)) != nullptr) {
        auto fileLen = func(dirp->d_name);
        if (fileLen) {
            inFileList.emplace_back(dirp->d_name, fileLen);
        }
    }
    closedir(dp);
    return inFileList;
}
}

vector<pair<string, int> >getFileList(const string& path, const function<int(const char*)>& func) {
    if (!is_cacheable_filter(func)) {
        return read_dir_list(path, func);
    }
    static std::mutex cache_mutex;
    static std::unordered_map<string, DirListCacheEntry> cache;
    std::error_code ec;
    auto mtime = std::filesystem::last_write_time(path, ec);
    if (ec) {
        return read_dir_list(path, func);
    }
    {
        std::lock_guard<std::mutex> lock(cache_mutex);
        auto it = cache.find(path);
        if (it != cache.end() && it->second.has_mtime && it->second.mtime == mtime) {
            return it->second.items;
        }
    }
    auto items = read_dir_list(path, func);
    {
        std::lock_guard<std::mutex> lock(cache_mutex);
        auto &entry = cache[path];
        entry.items = items;
        entry.mtime = mtime;
        entry.has_mtime = true;
    }
    return items;
}

vector<pair<string, int> >getFileList(const string& path) {
    return getFileList(path, always_true_filter);
}


int get_sim(int solution_id, int lang, int pid, int &sim_s_id) {
    if (no_sim) {
        return 0;
    }
    shared_ptr<Language> languageModel(getLanguageModel(lang));
    char src_pth[BUFFER_SIZE];
    sprintf(src_pth, "Main.%s", languageModel->getFileSuffix().c_str());
    if (DEBUG) {
        cout << "get sim: " << src_pth << endl;
    }
    int sim = 0;
    if (!admin) {
        sim = execute_cmd("/usr/bin/sim.sh %s %d", src_pth, pid);
    }
    if (!sim) {
        if (DEBUG) {
            cout << "SIM is not detected!" << endl;
        }
        execute_cmd("/bin/mkdir ../data/%d/ac/", pid);

        execute_cmd("/bin/cp %s ../data/%d/ac/%d.%s", src_pth, pid, solution_id,
                    languageModel->getFileSuffix().c_str());
        //c cpp will
        string suffix = languageModel->getFileSuffix();
        if (suffix == "c") {
            execute_cmd("/bin/ln -s %s/data/%d/ac/%d.%s %s/data/%d/ac/%d.%s", oj_home, pid,
                        solution_id, "c", oj_home, pid, solution_id,
                        "cc");
        }
        else if (suffix == "cc") {
            execute_cmd("/bin/ln -s %s/data/%d/ac/%d.%s %s/data/%d/ac/%d.%s", oj_home, pid,
                        solution_id, "cc", oj_home, pid, solution_id,
                        "c");
        }

    } else {

        FILE *pf;
        pf = fopen("sim", "r");
        if (pf) {
            fscanf(pf, "%d%d", &sim, &sim_s_id);
            fclose(pf);
        }
        if (DEBUG) {
            cout << "FIND SIM! sim:" << sim << " sim_s_id:" << sim_s_id << endl;
        }
    }
    return sim;
}

int get_sim(int solution_id, int lang, int pid, int &sim_s_id, const char *work_dir) {
    if (no_sim) {
        return 0;
    }
    shared_ptr<Language> languageModel(getLanguageModel(lang));
    auto src_path = (std::filesystem::path(work_dir) /
                     (std::string("Main.") + languageModel->getFileSuffix())).string();
    if (DEBUG) {
        cout << "get sim: " << src_path << endl;
    }
    int sim = 0;
    if (!admin) {
        sim = execute_cmd("/usr/bin/sim.sh %s %d", src_path.c_str(), pid);
    }
    if (!sim) {
        if (DEBUG) {
            cout << "SIM is not detected!" << endl;
        }
        execute_cmd("/bin/mkdir %s/data/%d/ac/", oj_home, pid);
        execute_cmd("/bin/cp %s %s/data/%d/ac/%d.%s", src_path.c_str(), oj_home, pid, solution_id,
                    languageModel->getFileSuffix().c_str());
        string suffix = languageModel->getFileSuffix();
        if (suffix == "c") {
            execute_cmd("/bin/ln -s %s/data/%d/ac/%d.%s %s/data/%d/ac/%d.%s", oj_home, pid,
                        solution_id, "c", oj_home, pid, solution_id, "cc");
        } else if (suffix == "cc") {
            execute_cmd("/bin/ln -s %s/data/%d/ac/%d.%s %s/data/%d/ac/%d.%s", oj_home, pid,
                        solution_id, "cc", oj_home, pid, solution_id, "c");
        }
        return 0;
    }
    string buf;
    buf = string("echo \"select `solution_id`, `sim`, `sim_s_id` from solution where solution_id=") +
          to_string(solution_id) + "\"| mysql -h " + string(host_name) +
          " -u " + string(user_name) + " -p" + string(password) +
          " " + string(db_name);
    FILE *pFile = popen(buf.c_str(), "re");
    if (!pFile) {
        return sim;
    }
    char buffer[512];
    char s_id[BUFFER_SIZE];
    while (fgets(buffer, sizeof(buffer), pFile)) {
        if (buffer[0] == 0) {
            continue;
        }
        if (sscanf(buffer, "%d %d %d", &solution_id, &sim, &sim_s_id) == 3) {
            break;
        }
    }
    pclose(pFile);
    sprintf(s_id, "%d", sim_s_id);
    sim_s_id = atoi(s_id);
    return sim;
}

string getFileContent(const string& file) {
    FILE *fp = fopen(file.c_str(), "rb");
    if (!fp) {
        return "";
    }
    if (fseek(fp, 0, SEEK_END) != 0) {
        fclose(fp);
        return "";
    }
    long size = ftell(fp);
    if (size < 0) {
        fclose(fp);
        return "";
    }
    if (fseek(fp, 0, SEEK_SET) != 0) {
        fclose(fp);
        return "";
    }
    string content;
    if (size == 0) {
        fclose(fp);
        return content;
    }
    content.resize(static_cast<size_t>(size));
    size_t read_size = fread(&content[0], 1, content.size(), fp);
    fclose(fp);
    content.resize(read_size);
    return content;
}

void mk_shm_workdir(char *work_dir) {
    char shm_path[BUFFER_SIZE];
    sprintf(shm_path, "%s/submission", oj_home);
    execute_cmd("/bin/chown -R judge %s", shm_path);
    execute_cmd("/bin/chgrp -R judge %s", shm_path);
    sprintf(shm_path, "/dev/shm/hustoj/%s", work_dir);
    execute_cmd("/bin/mkdir -p %s", shm_path);
    execute_cmd("/bin/ln -s %s %s/", shm_path, oj_home);
    execute_cmd("/bin/chown judge %s ", shm_path);
    execute_cmd("chmod 755 %s ", shm_path);
    //sim need a soft link in shm_dir to work correctly
    sprintf(shm_path, "/dev/shm/hustoj/%s/", oj_home);
    execute_cmd("/bin/ln -s %s/data %s", oj_home, shm_path);
}

void make_workdir(char* work_dir) {
    char shm_path[BUFFER_SIZE];
    sprintf(shm_path, "%s/submission", oj_home);
    execute_cmd("/bin/chown -R judge %s", shm_path);
    execute_cmd("/bin/chgrp -R judge %s", shm_path);
    execute_cmd("/bin/mkdir -p %s", work_dir);
    execute_cmd("/bin/chown -R judge %s ", work_dir);
    execute_cmd("/bin/chgrp -R judge %s ", work_dir);
    execute_cmd("/bin/chmod 755 %s", work_dir);
    sprintf(shm_path, "/dev/shm/hustoj/%s/", oj_home);
    execute_cmd("/bin/ln -s %s/data %s", oj_home, shm_path);
}

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

static bool copy_file_fast(const std::string &source, const std::string &target) {
    std::error_code ec;
    std::filesystem::copy_file(source, target, std::filesystem::copy_options::overwrite_existing, ec);
    return !ec;
}

static void copy_dic_files(int problem_id, const char *work_dir) {
    std::error_code ec;
    auto dic_dir = std::filesystem::path(oj_home) / "data" / std::to_string(problem_id);
    if (!std::filesystem::exists(dic_dir, ec)) {
        return;
    }
    auto dic_mtime = std::filesystem::last_write_time(dic_dir, ec);
    if (ec) {
        return;
    }
    const string cache_key = std::filesystem::path(work_dir).string() + "|" + std::to_string(problem_id);
    {
        static std::mutex cache_mutex;
        static std::unordered_map<string, std::filesystem::file_time_type> cache;
        std::lock_guard<std::mutex> lock(cache_mutex);
        auto it = cache.find(cache_key);
        if (it != cache.end() && it->second == dic_mtime) {
            return;
        }
        cache[cache_key] = dic_mtime;
    }
    for (const auto &entry : std::filesystem::directory_iterator(dic_dir, ec)) {
        if (ec) {
            break;
        }
        if (!entry.is_regular_file()) {
            continue;
        }
        auto path = entry.path();
        if (path.extension() != ".dic") {
            continue;
        }
        auto target = std::filesystem::path(work_dir) / path.filename();
        copy_file_fast(path.string(), target.string());
    }
}

void prepare_files(const char *filename, int namelen, char *infile, int &p_id,
                   char *work_dir, char *outfile, char *userfile, int runner_id) {
    //              printf("ACflg=%d %d check a file!\n",ACflg,solution_id);

    char fname0[BUFFER_SIZE];
    char fname[BUFFER_SIZE];
    strncpy(fname0, filename, static_cast<size_t>(namelen));
    fname0[namelen] = 0;
    escape(fname, fname0);
    printf("%s\n%s\n", fname0, fname);
    snprintf(infile, BUFFER_SIZE, "%s/data/%d/%s.in", oj_home, p_id, fname);
    copy_file_fast(infile, (std::filesystem::path(work_dir) / "data.in").string());
    copy_dic_files(p_id, work_dir);
    snprintf(outfile, BUFFER_SIZE, "%s/data/%d/%s.out", oj_home, p_id, fname0);
    snprintf(userfile, BUFFER_SIZE, "%s/run%d/user.out", oj_home, runner_id);
}

void prepare_files_with_id(const char *filename, int namelen, char *infile, int &p_id,
                   char *work_dir, char *outfile, char *userfile, int runner_id, int file_id) {
    //              printf("ACflg=%d %d check a file!\n",ACflg,solution_id);

    char fname0[BUFFER_SIZE];
    char fname[BUFFER_SIZE];
    strncpy(fname0, filename, static_cast<size_t>(namelen));
    fname0[namelen] = 0;
    escape(fname, fname0);
    printf("%s\n%s\n", fname0, fname);
    snprintf(infile, BUFFER_SIZE, "%s/data/%d/%s.in", oj_home, p_id, fname);
    char data_name[BUFFER_SIZE];
    snprintf(data_name, sizeof(data_name), "data%d.in", file_id);
    copy_file_fast(infile, (std::filesystem::path(work_dir) / data_name).string());
    copy_dic_files(p_id, work_dir);
    snprintf(outfile, BUFFER_SIZE, "%s/data/%d/%s.out", oj_home, p_id, fname0);
    snprintf(userfile, BUFFER_SIZE, "%s/run%d/user%d.out", oj_home, runner_id, file_id);
}

void print_runtimeerror(const char *err) {
    FILE *ferr = fopen("error.out", "a+");
    fprintf(ferr, "Runtime Error:%s\n", err);
    fclose(ferr);
}

void print_runtimeerror(const char *err, const char *work_dir) {
    auto path = (std::filesystem::path(work_dir) / "error.out").string();
    FILE *ferr = fopen(path.c_str(), "a+");
    fprintf(ferr, "Runtime Error:%s\n", err);
    fclose(ferr);
}


void getProblemInfoFromSubmissionInfo(SubmissionInfo& submissionInfo, double& time_lmt, int& mem_lmt, int& isspj) {
    time_lmt = submissionInfo.getTimeLimit();
    mem_lmt = int(submissionInfo.getMemoryLimit());
    isspj = int(submissionInfo.getSpecialJudge());
    if (time_lmt <= 0) {
        time_lmt = 1;
    }
}


void getCustomInputFromSubmissionInfo(SubmissionInfo& submissionInfo) {
    if (DEBUG) {
        cout << submissionInfo.getCustomInput() << endl;
    }
    char src_pth[BUFFER_SIZE];
    sprintf(src_pth, "data.in");
    FILE *fp_src = fopen(src_pth, "w");
    fprintf(fp_src, "%s", submissionInfo.getCustomInput().c_str());
    fclose(fp_src);
}

void getCustomInputFromSubmissionInfo(SubmissionInfo& submissionInfo, const char* work_dir) {
    if (DEBUG) {
        cout << submissionInfo.getCustomInput() << endl;
    }
    auto path = (std::filesystem::path(work_dir) / "data.in").string();
    FILE *fp_src = fopen(path.c_str(), "w");
    fprintf(fp_src, "%s", submissionInfo.getCustomInput().c_str());
    fclose(fp_src);
}


void getSolutionFromSubmissionInfo(SubmissionInfo& submissionInfo, char* usercode) {
    char src_pth[BUFFER_SIZE];
    shared_ptr<Language> languageModel(getLanguageModel(submissionInfo.getLanguage()));
    sprintf(usercode, "%s", submissionInfo.getSource().c_str());
    sprintf(src_pth, "Main.%s", languageModel->getFileSuffix().c_str());
    if (DEBUG) {
        printf("Main=%s", src_pth);
        cout << usercode << endl;
    }
    FILE *fp_src = fopen(src_pth, "we");
    fprintf(fp_src, "%s", usercode);
    fclose(fp_src);
}

void getSolutionFromSubmissionInfo(SubmissionInfo& submissionInfo, char* usercode, const char* work_dir) {
    shared_ptr<Language> languageModel(getLanguageModel(submissionInfo.getLanguage()));
    sprintf(usercode, "%s", submissionInfo.getSource().c_str());
    auto path = (std::filesystem::path(work_dir) /
                 (std::string("Main.") + languageModel->getFileSuffix())).string();
    if (DEBUG) {
        printf("Main=%s", path.c_str());
        cout << usercode << endl;
    }
    FILE *fp_src = fopen(path.c_str(), "we");
    fprintf(fp_src, "%s", usercode);
    fclose(fp_src);
}


string getRuntimeInfoContents(const string& filename) {
    string runtimeInfo;
    FILE *fp = fopen(filename.c_str(), "rb");
    if (!fp) {
        return runtimeInfo;
    }
    constexpr size_t kMaxRead = 5120;
    runtimeInfo.resize(kMaxRead);
    size_t read_size = fread(&runtimeInfo[0], 1, kMaxRead, fp);
    fclose(fp);
    runtimeInfo.resize(read_size);
    return runtimeInfo;
}

template<class Instance>
Instance* getDynamicLibraryInstance (const char* dynamicLibraryPath, const char* createInstanceMethodName) {
    static std::mutex cache_mutex;
    static std::unordered_map<std::string, void*> create_cache;
    static std::unordered_map<std::string, void*> handler_cache;
    const std::string key = std::string(dynamicLibraryPath) + "|" + createInstanceMethodName;
    void* create_ptr = nullptr;
    {
        std::lock_guard<std::mutex> lock(cache_mutex);
        auto it = create_cache.find(key);
        if (it != create_cache.end()) {
            create_ptr = it->second;
        } else {
            void* handler = dlopen(dynamicLibraryPath, RTLD_LAZY);
            if (!handler) {
                cerr << "Cannot load library: " << dynamicLibraryPath << ": " << dlerror() << endl;
                exit(1);
            }
            dlerror();
            create_ptr = dlsym(handler, createInstanceMethodName);
            const char* dlsym_error = dlerror();
            if (dlsym_error) {
                cerr << "Cannot load symbol " << createInstanceMethodName << " create: " << dlsym_error << endl;
                exit(1);
            }
            handler_cache.emplace(key, handler);
            create_cache.emplace(key, create_ptr);
        }
    }
    auto createInstance = reinterpret_cast<Instance* (*)()>(create_ptr);
    return createInstance();
}


Language* getLanguageModel(int language) {
    string languageName = languageNameReader.GetString(to_string(language));
    return getDynamicLibraryInstance<Language>(("/usr/lib/cupjudge/lib" + languageName + ".so").c_str(), (string("createInstance") + languageName).c_str());
}

MySQLSubmissionAdapter* getAdapter() {
    return getDynamicLibraryInstance<MySQLSubmissionAdapter>("/usr/lib/cupjudge/libmysql.so", "createInstance");
}

Compare::Compare* getCompareModel() {
    return getDynamicLibraryInstance<Compare::Compare>("/usr/lib/cupjudge/libcompare.so", "createInstance");
}

void setRunUser () {
#ifdef UNIT_TEST
    return;
#else
    while (setgid(1536) != 0)
        sleep(1);
    while (setuid(1536) != 0)
        sleep(1);
    while (setresuid(1536, 1536, 1536) != 0)
        sleep(1);
#endif
}

bool isPython(int language) {
    switch (language) {
        case PYTHON2:
        case PYTHON3:
        case PYPY:
        case PYPY3:
            return true;
        default:
            return false;
    }
    return false;
}
