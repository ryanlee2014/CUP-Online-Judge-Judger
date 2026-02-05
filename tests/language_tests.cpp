#include "test_common.h"

TEST(LanguageClassesBasics) {
    C11 c11;
    C99 c99;
    Clang clang;
    Clang11 clang11;
    Clangpp clangpp;
    Clangpp17 clangpp17;
    Cpp98 cpp98;
    Cpp11 cpp11;
    Cpp17 cpp17;
    Cpp20 cpp20;
    Bash bash;
    Go go;
    Java java;
    Java6 java6;
    Java7 java7;
    Java8 java8;
    Kotlin kotlin;
    KotlinNative kotlin_native;
    Python2 py2;
    Python3 py3;
    PyPy pypy;
    PyPy3 pypy3;
    Php php;
    Perl perl;
    Ruby ruby;
    Lua lua;
    Schema schema;
    JavaScript js;
    Csharp cs;
    Objc objc;
    FreeBasic fb;
    Pascal pas;
    int counter[512] = {};
    c11.initCallCounter(counter);
    EXPECT_TRUE(counter[0] == HOJ_MAX_LIMIT);
    C11 &cpp98_as_c11 = cpp98;
    EXPECT_EQ(cpp98_as_c11.getFileSuffix(), "cc");
    EXPECT_EQ(py2.getFileSuffix(), "py");
    EXPECT_EQ(java.getFileSuffix(), "java");
    EXPECT_EQ(js.getFileSuffix(), "js");
    if (run_seccomp()) {
        bash.buildSeccompSandbox();
        java.buildSeccompSandbox();
        py3.buildSeccompSandbox();
        kotlin_native.buildSeccompSandbox();
        php.buildSeccompSandbox();
        schema.buildSeccompSandbox();
        cs.buildSeccompSandbox();
        objc.buildSeccompSandbox();
        fb.buildSeccompSandbox();
        pas.buildSeccompSandbox();
        ruby.buildSeccompSandbox();
        perl.buildSeccompSandbox();
        lua.buildSeccompSandbox();
        go.buildSeccompSandbox();
        clang.buildSeccompSandbox();
        clang11.buildSeccompSandbox();
        clangpp.buildSeccompSandbox();
        clangpp17.buildSeccompSandbox();
    }
}

TEST(LanguageBaseBasics) {
    test_hooks::reset();
    DummyLanguage lang;
    TempDir tmp;
    int calls[call_array_size] = {};
    lang.initCallCounter(calls);
    EXPECT_EQ(calls[0], HOJ_MAX_LIMIT);
    lang.setProcessLimit();
    lang.setCompileProcessLimit();
    lang.setCompileMount(tmp.path.string().c_str());
    lang.setCompileExtraConfig();
    lang.buildRuntime(tmp.path.string().c_str());
    EXPECT_EQ(lang.buildTimeLimit(1.5, 2.0), 1.5);
    EXPECT_EQ(lang.buildMemoryLimit(64, 10), 64);
    lang.buildChrootSandbox(tmp.path.string().c_str());
    rlimit lim{};
    lang.runMemoryLimit(lim);
    int ac = ACCEPT;
    lang.fixACFlag(ac);
    int top = 0;
    lang.fixFlagWithVMIssue(const_cast<char *>(tmp.path.string().c_str()), ac, top, 64);
    EXPECT_TRUE(lang.supportParallel());
    EXPECT_TRUE(lang.isValidExitCode(0));
    EXPECT_TRUE(!lang.isValidExitCode(1));
    lang.setDebug(1);
}

TEST(LanguagePythonFamily) {
    test_hooks::reset();
    TempDir tmp;
    Python2 py2;
    Python3 py3;
    PyPy pypy;
    PyPy3 pypy3;
    exercise_python_like(py2, tmp.path, 3);
    exercise_python_like(py3, tmp.path, 3);
    exercise_python_like(pypy, tmp.path, 0);
    exercise_python_like(pypy3, tmp.path, 0);
}

TEST(LanguagePythonNoErrorBranches) {
    test_hooks::reset();
    TempDir tmp;
    auto old_cwd = std::filesystem::current_path();
    std::filesystem::current_path(tmp.path);
    write_file(tmp.path / "error.out", "");
    Python2 py2;
    Python3 py3;
    PyPy pypy;
    PyPy3 pypy3;
    EXPECT_EQ(py2.fixACStatus(ACCEPT), ACCEPT);
    EXPECT_EQ(py3.fixACStatus(ACCEPT), ACCEPT);
    EXPECT_EQ(pypy.fixACStatus(ACCEPT), ACCEPT);
    EXPECT_EQ(pypy3.fixACStatus(ACCEPT), ACCEPT);
    int ac = ACCEPT;
    py2.fixACFlag(ac);
    EXPECT_EQ(ac, ACCEPT);
    py3.fixACFlag(ac);
    EXPECT_EQ(ac, ACCEPT);
    pypy.fixACFlag(ac);
    EXPECT_EQ(ac, ACCEPT);
    pypy3.fixACFlag(ac);
    EXPECT_EQ(ac, ACCEPT);
    write_file(tmp.path / "error.out", "OtherError");
    EXPECT_EQ(py2.fixACStatus(ACCEPT), ACCEPT);
    EXPECT_EQ(py3.fixACStatus(ACCEPT), ACCEPT);
    EXPECT_EQ(pypy.fixACStatus(ACCEPT), ACCEPT);
    EXPECT_EQ(pypy3.fixACStatus(ACCEPT), ACCEPT);
    ac = ACCEPT;
    py2.fixACFlag(ac);
    EXPECT_EQ(ac, ACCEPT);
    py3.fixACFlag(ac);
    EXPECT_EQ(ac, ACCEPT);
    pypy.fixACFlag(ac);
    EXPECT_EQ(ac, ACCEPT);
    pypy3.fixACFlag(ac);
    EXPECT_EQ(ac, ACCEPT);
    std::filesystem::current_path(old_cwd);
}

TEST(LanguageC11Extra) {
    test_hooks::reset();
    C11Expose c11;
    rlimit lim{};
    c11.runMemoryLimit(lim);
    c11.setCompileMount("work");
    EXPECT_TRUE(c11.enableSim());
    EXPECT_TRUE(c11.gotErrorWhileRunning(true));
    if (run_seccomp()) {
        c11.buildSeccompSandbox();
    }
    c11.run(64);
    EXPECT_EQ(test_hooks::state().last_exec_path, "./Main");
    EXPECT_TRUE(c11.getArgs() != nullptr);
}

TEST(LanguageJavaFamily) {
    test_hooks::reset();
    TempDir tmp;
    std::string work = tmp.path.string();
    std::strcpy(oj_home, work.c_str());
    write_file(tmp.path / "etc" / "java0.policy", "policy");
    JavaExpose java;
    std::vector<std::string> args = {"javac", "Main.java"};
    java.compile(args, "-Xms32m", "-Xmx256m");
    EXPECT_EQ(test_hooks::state().last_exec_path, "javac");
    java.run(64);
    EXPECT_EQ(test_hooks::state().last_exec_path, "/usr/bin/java");
    java.setProcessLimit();
    java.setCompileProcessLimit();
    java.buildRuntime(work.c_str());
    EXPECT_EQ(java.buildTimeLimit(1.0, 2.0), 4.0);
    EXPECT_EQ(java.buildMemoryLimit(64, 10), 74);
    java.setCompileMount(work.c_str());
    java.setExtraPolicy(oj_home, work.c_str());
    int calls[call_array_size] = {};
    java.initCallCounter(calls);
    EXPECT_EQ(java.getFileSuffix(), "java");
    EXPECT_TRUE(java.getArgs() != nullptr);
    EXPECT_TRUE(java.enableSim());
    int ac = ACCEPT;
    int top = 0;
    test_hooks::state().system_result = 0;
    java.fixFlagWithVMIssue(work.data(), ac, top, 64);
    EXPECT_TRUE(java.isValidExitCode(17));
    EXPECT_TRUE(java.gotErrorWhileRunning(true));
    if (run_seccomp()) {
        java.buildSeccompSandbox();
    }
    java.buildChrootSandbox(work.c_str());
    Java6 java6;
    java6.run(32);
    EXPECT_EQ(test_hooks::state().last_exec_path, "/usr/bin/java-6");
    Java7 java7;
    java7.run(32);
    EXPECT_EQ(test_hooks::state().last_exec_path, "/usr/bin/java-7");
    Java8 java8;
    java8.run(32);
    EXPECT_EQ(test_hooks::state().last_exec_path, "/usr/bin/java-8");
}

TEST(LanguageFileSuffixes) {
    std::unique_ptr<Language> clangpp(createInstanceclangpp());
    std::unique_ptr<Language> clangpp17(createInstanceclangpp17());
    std::unique_ptr<Language> cpp17(createInstancecpp17());
    std::unique_ptr<Language> cpp20(createInstancecpp20());
    EXPECT_EQ(clangpp->getFileSuffix(), "cc");
    EXPECT_EQ(clangpp17->getFileSuffix(), "cc");
    EXPECT_EQ(cpp17->getFileSuffix(), "cc");
    EXPECT_EQ(cpp20->getFileSuffix(), "cc");
}

TEST(LanguageKotlinFamily) {
    test_hooks::reset();
    Kotlin kotlin;
    std::vector<std::string> args = {"kotlinc", "Main.kt"};
    kotlin.compile(args, "-Xms32m", "-Xmx256m");
    EXPECT_EQ(test_hooks::state().last_exec_path, "kotlinc");
    std::vector<std::string> empty_args;
    expect_exit([&]() { kotlin.compile(empty_args, "", ""); }, 0);
    kotlin.run(64);
    EXPECT_EQ(test_hooks::state().last_exec_path, "/usr/bin/java");
    EXPECT_EQ(kotlin.getFileSuffix(), "kt");
    EXPECT_TRUE(!kotlin.supportParallel());
    KotlinNativeExpose native;
    int calls[call_array_size] = {};
    native.initCallCounter(calls);
    EXPECT_EQ(native.getFileSuffix(), "kt");
    native.setProcessLimit();
    native.setCompileProcessLimit();
    native.setCompileMount("work");
    native.buildRuntime("work");
    EXPECT_EQ(native.buildTimeLimit(1.0, 2.0), 4.0);
    EXPECT_EQ(native.buildMemoryLimit(64, 10), 74);
    EXPECT_TRUE(native.getArgs() != nullptr);
    native.run(64);
    if (run_seccomp()) {
        native.buildSeccompSandbox();
    }
    EXPECT_TRUE(native.gotErrorWhileRunning(true));
}

TEST(LanguageOtherRuntimes) {
    test_hooks::reset();
    TempDir tmp;
    int stdout_fd = dup(fileno(stdout));
    auto old_cwd = std::filesystem::current_path();
    std::filesystem::current_path(tmp.path);
    write_file(tmp.path / "ce.txt", "err");
    std::string work = tmp.path.string();
    Bash bash;
    int calls[call_array_size] = {};
    bash.initCallCounter(calls);
    EXPECT_EQ(bash.buildTimeLimit(1.0, 2.0), 4.0);
    EXPECT_EQ(bash.buildMemoryLimit(64, 10), 74);
    EXPECT_EQ(bash.getCompileResult(0), 3);
    bash.setProcessLimit();
    bash.buildRuntime(work.c_str());
    if (run_seccomp()) {
        bash.buildSeccompSandbox();
    }
    rusage ruse{};
    rlimit lim{};
    EXPECT_TRUE(bash.getMemory(ruse, getpid()) >= 0);
    EXPECT_TRUE(bash.gotErrorWhileRunning(true));
    bash.run(64);
    EXPECT_EQ(bash.getFileSuffix(), "sh");

    Go go;
    go.initCallCounter(calls);
    go.setProcessLimit();
    go.setCompileProcessLimit();
    EXPECT_EQ(go.buildTimeLimit(1.0, 2.0), 4.0);
    EXPECT_EQ(go.buildMemoryLimit(64, 10), 74);
    go.runMemoryLimit(lim);
    EXPECT_TRUE(!go.gotErrorWhileRunning(false));
    EXPECT_TRUE(!go.enableSim());
    EXPECT_EQ(go.getFileSuffix(), "go");
    if (run_seccomp()) {
        go.buildSeccompSandbox();
    }

    Lua lua;
    lua.initCallCounter(calls);
    lua.buildRuntime(work.c_str());
    EXPECT_EQ(lua.buildTimeLimit(1.0, 2.0), 4.0);
    EXPECT_EQ(lua.buildMemoryLimit(64, 10), 74);
    EXPECT_TRUE(lua.getMemory(ruse, getpid()) >= 0);
    if (run_seccomp()) {
        lua.buildSeccompSandbox();
    }
    lua.run(64);
    EXPECT_EQ(lua.getFileSuffix(), "lua");

    JavaScript js;
    js.initCallCounter(calls);
    js.setProcessLimit();
    js.buildRuntime(work.c_str());
    EXPECT_EQ(js.buildTimeLimit(1.0, 2.0), 4.0);
    EXPECT_EQ(js.buildMemoryLimit(64, 10), 74);
    if (run_seccomp()) {
        js.buildSeccompSandbox();
    }
    js.run(64);
    EXPECT_EQ(js.getFileSuffix(), "js");

    CsharpExpose cs;
    cs.initCallCounter(calls);
    cs.buildRuntime(work.c_str());
    cs.setProcessLimit();
    EXPECT_EQ(cs.buildTimeLimit(1.0, 2.0), 4.0);
    EXPECT_EQ(cs.buildMemoryLimit(64, 10), 74);
    EXPECT_TRUE(cs.getArgs() != nullptr);
    if (run_seccomp()) {
        cs.buildSeccompSandbox();
    }
    cs.run(64);
    EXPECT_TRUE(cs.gotErrorWhileRunning(true));
    EXPECT_EQ(cs.getFileSuffix(), "cs");

    Php php;
    php.initCallCounter(calls);
    php.buildRuntime(work.c_str());
    EXPECT_EQ(php.buildTimeLimit(1.0, 2.0), 4.0);
    EXPECT_EQ(php.buildMemoryLimit(64, 10), 74);
    if (run_seccomp()) {
        php.buildSeccompSandbox();
    }
    php.run(64);
    EXPECT_EQ(php.getFileSuffix(), "php");

    Perl perl;
    perl.initCallCounter(calls);
    perl.buildRuntime(work.c_str());
    EXPECT_EQ(perl.buildTimeLimit(1.0, 2.0), 4.0);
    EXPECT_EQ(perl.buildMemoryLimit(64, 10), 74);
    EXPECT_TRUE(perl.getMemory(ruse, getpid()) >= 0);
    if (run_seccomp()) {
        perl.buildSeccompSandbox();
    }
    perl.run(64);
    EXPECT_EQ(perl.getFileSuffix(), "pl");

    Ruby ruby;
    ruby.initCallCounter(calls);
    ruby.setProcessLimit();
    ruby.buildRuntime(work.c_str());
    EXPECT_EQ(ruby.buildTimeLimit(1.0, 2.0), 4.0);
    EXPECT_EQ(ruby.buildMemoryLimit(64, 10), 74);
    EXPECT_EQ(ruby.getCompileResult(0), 3);
    EXPECT_TRUE(ruby.getMemory(ruse, getpid()) >= 0);
    if (run_seccomp()) {
        ruby.buildSeccompSandbox();
    }
    ruby.run(64);
    EXPECT_EQ(ruby.getFileSuffix(), "rb");

    Schema schema;
    schema.initCallCounter(calls);
    schema.setProcessLimit();
    schema.buildRuntime(work.c_str());
    EXPECT_EQ(schema.buildTimeLimit(1.0, 2.0), 4.0);
    EXPECT_EQ(schema.buildMemoryLimit(64, 10), 74);
    EXPECT_TRUE(schema.getMemory(ruse, getpid()) >= 0);
    if (run_seccomp()) {
        schema.buildSeccompSandbox();
    }
    schema.run(64);
    EXPECT_EQ(schema.getFileSuffix(), "scm");

    Objc objc;
    objc.initCallCounter(calls);
    objc.buildRuntime(work.c_str());
    if (run_seccomp()) {
        objc.buildSeccompSandbox();
    }
    EXPECT_TRUE(objc.getMemory(ruse, getpid()) >= 0);
    objc.run(64);
    EXPECT_EQ(objc.getFileSuffix(), "m");

    FreeBasic fb;
    fb.initCallCounter(calls);
    fb.buildRuntime(work.c_str());
    fb.setCompileMount(work.c_str());
    EXPECT_EQ(fb.buildTimeLimit(1.0, 2.0), 4.0);
    EXPECT_EQ(fb.buildMemoryLimit(64, 10), 74);
    fb.setCompileExtraConfig();
    freopen("/dev/stdout", "w", stdout);
    fb.runMemoryLimit(lim);
    EXPECT_TRUE(fb.getMemory(ruse, getpid()) >= 0);
    EXPECT_TRUE(!fb.gotErrorWhileRunning(false));
    EXPECT_TRUE(!fb.enableSim());
    if (run_seccomp()) {
        fb.buildSeccompSandbox();
    }
    fb.run(64);
    EXPECT_EQ(fb.getFileSuffix(), "bas");

    Pascal pas;
    pas.initCallCounter(calls);
    pas.buildRuntime(work.c_str());
    pas.setCompileExtraConfig();
    freopen("/dev/stdout", "w", stdout);
    EXPECT_TRUE(pas.getMemory(ruse, getpid()) >= 0);
    if (run_seccomp()) {
        pas.buildSeccompSandbox();
    }
    pas.run(64);
    EXPECT_EQ(pas.getFileSuffix(), "pas");
    std::filesystem::current_path(old_cwd);
    if (stdout_fd >= 0) {
        fflush(stdout);
        dup2(stdout_fd, fileno(stdout));
        close(stdout_fd);
    }
}

TEST(LanguageSeccompSandboxes) {
    test_hooks::reset();
    Bash bash;
    C11 c11;
    Csharp cs;
    FreeBasic fb;
    Go go;
    Java java;
    JavaScript js;
    KotlinNative kn;
    Lua lua;
    Objc objc;
    Pascal pas;
    Perl perl;
    Php php;
    PyPy pypy;
    PyPy3 pypy3;
    Python2 py2;
    Python3 py3;
    Ruby ruby;
    Schema schema;
    Clang clang;
    Clang11 clang11;
    Clangpp clangpp;
    Clangpp17 clangpp17;
    Cpp11 cpp11;
    Cpp17 cpp17;
    Cpp20 cpp20;
    Cpp98 cpp98;
    C99 c99;
    exercise_seccomp_sandbox(bash);
    exercise_seccomp_sandbox(c11);
    exercise_seccomp_sandbox(cs);
    exercise_seccomp_sandbox(fb);
    exercise_seccomp_sandbox(go);
    exercise_seccomp_sandbox(java);
    exercise_seccomp_sandbox(js);
    exercise_seccomp_sandbox(kn);
    exercise_seccomp_sandbox(lua);
    exercise_seccomp_sandbox(objc);
    exercise_seccomp_sandbox(pas);
    exercise_seccomp_sandbox(perl);
    exercise_seccomp_sandbox(php);
    exercise_seccomp_sandbox(pypy);
    exercise_seccomp_sandbox(pypy3);
    exercise_seccomp_sandbox(py2);
    exercise_seccomp_sandbox(py3);
    exercise_seccomp_sandbox(ruby);
    exercise_seccomp_sandbox(schema);
    exercise_seccomp_sandbox(clang);
    exercise_seccomp_sandbox(clang11);
    exercise_seccomp_sandbox(clangpp);
    exercise_seccomp_sandbox(clangpp17);
    exercise_seccomp_sandbox(cpp11);
    exercise_seccomp_sandbox(cpp17);
    exercise_seccomp_sandbox(cpp20);
    exercise_seccomp_sandbox(cpp98);
    exercise_seccomp_sandbox(c99);
}

TEST(LanguageFactoryFunctions) {
    exercise_factory(createInstancebash, destroyIntancebash);
    exercise_factory(createInstancec11, destroyInstancec11);
    exercise_factory(createInstancec99, destroyInstancec99);
    exercise_factory(createInstanceclang, destroyInstanceclang);
    exercise_factory(createInstanceclang11, destroyInstanceclang11);
    exercise_factory(createInstanceclangpp, destroyInstanceclangpp);
    exercise_factory(createInstanceclangpp17, destroyInstanceclangpp17);
    exercise_factory(createInstancecpp11, destroyInstancecpp11);
    exercise_factory(createInstancecpp17, destroyInstancecpp17);
    exercise_factory(createInstancecpp20, destroyInstancecpp20);
    exercise_factory(createInstancecpp98, destroyInstancecpp98);
    exercise_factory(createInstancecsharp, destroyInstancecsharp);
    exercise_factory(createInstancefreebasic, destroyInstancefreebasic);
    exercise_factory(createInstancego, destroyInstancego);
    exercise_factory(createInstancejava, destroyInstanceJava);
    exercise_factory(createInstancejava6, destroyInstancejava6);
    exercise_factory(createInstancejava7, destroyInstancejava7);
    exercise_factory(createInstancejava8, destroyInstancejava8);
    exercise_factory(createInstancejavascript, destroyInstancejavascript);
    exercise_factory(createInstancekotlin, destroyInstancekotlin);
    exercise_factory(createInstancekotlinnative, destroyInstancekotlinnative);
    exercise_factory(createInstancelua, destroyInstancelua);
    exercise_factory(createInstanceobjectivec, destroyInstanceobjectivec);
    exercise_factory(createInstancepascal, destroyInstancepascal);
    exercise_factory(createInstanceperl, destroyInstanceperl);
    exercise_factory(createInstancephp, destroyInstancephp);
    exercise_factory(createInstancepypy, destroyInstancepypy);
    exercise_factory(createInstancepypy3, destroyInstancepypy3);
    exercise_factory(createInstancepython2, destroyInstancepython2);
    exercise_factory(createInstancepython3, destroyInstancepython3);
    exercise_factory(createInstanceruby, destroyInstanceruby);
    exercise_factory(createInstanceschema, destroyInstanceschema);
}
