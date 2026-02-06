cmake_minimum_required(VERSION 3.9)
project(judge_client)
set(CMAKE_CXX_STANDARD 17)
if (NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif ()
if (EXISTS "/usr/local/bin/gcc-9")
    set(CMAKE_C_COMPILER /usr/local/bin/gcc-9)
    set(CMAKE_CXX_COMPILER /usr/local/bin/g++-9)
elseif (EXISTS "/usr/local/bin/gcc-8")
    set(CMAKE_C_COMPILER /usr/local/bin/gcc-8)
    set(CMAKE_CXX_COMPILER /usr/local/bin/g++-8)
elseif (EXISTS "/usr/local/bin/gcc")
    set(CMAKE_C_COMPILER /usr/local/bin/gcc)
    set(CMAKE_CXX_COMPILER /usr/local/bin/g++)
else ()
    set(CMAKE_C_COMPILER /usr/bin/gcc)
    set(CMAKE_CXX_COMPILER /usr/bin/g++)
endif ()
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall")
set(CMAKE_CXX_FLAGS_RELEASE "-O3")
option(ENABLE_COVERAGE "Build with coverage flags" OFF)
if (ENABLE_COVERAGE)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O0 -g --coverage")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O0 -g --coverage")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} --coverage")
    set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} --coverage")
endif ()
# set(CMAKE_C_COMPILER /usr/local/bin/gcc)
# set(CMAKE_CXX_COMPILER /usr/local/bin/g++)

# handle the QUIETLY and REQUIRED arguments and set MYSQL_FOUND to TRUE if
# all listed variables are TRUE
find_package(Threads)
add_executable(judge_client header/okcalls.h header/ncalls.h header/okcalls32.h header/okcalls64.h library/json.hpp library/easywsclient.cpp library/easywsclient.hpp judge_client.cpp judge_client_adapter.cpp judge_client_case_executor.cpp judge_client_compile_helpers.cpp judge_client_compile.cpp judge_client_context_helpers.cpp judge_client_context.cpp judge_client_flow_bootstrap.cpp judge_client_flow_helpers.cpp judge_client_flow_load_helpers.cpp judge_client_flow_log_helpers.cpp judge_client_flow_path_helpers.cpp judge_client_flow_prepare_helpers.cpp judge_client_flow_prep_helpers.cpp judge_client_flow_runner.cpp judge_client_flow_socket_helpers.cpp judge_client_flow_syscall_helpers.cpp judge_client_path_utils.cpp judge_client_process_utils.cpp judge_client_report_bundle_helpers.cpp judge_client_report_case_helpers.cpp judge_client_report_finalize_helpers.cpp judge_client_report_helpers.cpp judge_client_report_payload_helpers.cpp judge_client_report_result_helpers.cpp judge_client_report_score_helpers.cpp judge_client_report_task_helpers.cpp judge_client_report_test_run_helpers.cpp judge_client_run_helpers.cpp judge_client_run_io_helpers.cpp judge_client_run_judge_helpers.cpp judge_client_run_limit_helpers.cpp judge_client_run_path_helpers.cpp judge_client_run_series_helpers.cpp judge_client_watch_helpers.cpp judge_client_watch_handlers.cpp judge_client_watch_metrics.cpp judge_client_watch_phases.cpp judge_client_util_helpers.cpp judge_client_runtime_bridge.cpp judge_client_run.cpp judge_client_report.cpp judge_client_util.cpp judge_client_flow.cpp judge_client_watch.cpp model/websocket/WebSocketSender.cpp model/websocket/WebSocketSender.h library/judge_lib.h header/static_var.h model/base/JSONVectorReader.h model/base/JSONVectorReader.cpp model/base/Bundle.cpp model/base/Bundle.h library/judge_lib.h.cpp header/static_var.cpp model/base/Pack.cpp model/base/Pack.h model/wrapper/CompilerConfigReader.cpp model/wrapper/CompilerConfigReader.h model/base/ThreadPool.h model/submission/SubmissionInfo.cpp model/submission/SubmissionInfo.h model/judge/language/Language.cpp model/judge/language/Language.h model/judge/policy/SpecialJudge.cpp model/judge/policy/SpecialJudge.h model/config/ConfigInfo.cpp model/config/ConfigInfo.h manager/syscall/InitManager.cpp manager/syscall/InitManager.h external/mysql/MySQLSubmissionAdapter.cpp external/mysql/MySQLSubmissionAdapter.h)
add_executable(wsjudged wsjudged.cpp)
target_link_libraries(judge_client ${CMAKE_THREAD_LIBS_INIT})
target_link_libraries(judge_client ${CMAKE_DL_LIBS})

option(ENABLE_TESTS "Build unit tests" ON)
if (ENABLE_TESTS)
    enable_testing()
    set(TEST_SOURCES
        tests/test_main.cpp
        tests/test_registry.cpp
        tests/test_hooks.cpp
        tests/stub_easywsclient.cpp
        tests/judge_client_test.cpp
        tests/wsjudged_test.cpp
        tests/judge_client_flow_tests.cpp
        tests/judge_client_run_tests.cpp
        tests/judge_client_watch_tests.cpp
        tests/judge_lib_tests.cpp
        tests/language_tests.cpp
        tests/compare_tests.cpp
        tests/config_tests.cpp
        tests/mysql_tests.cpp
        tests/websocket_tests.cpp
        tests/special_judge_tests.cpp
        tests/bundle_pack_tests.cpp
        judge_client_adapter.cpp
        judge_client_case_executor.cpp
        judge_client_compile.cpp
        judge_client_compile_helpers.cpp
        judge_client_context_helpers.cpp
        judge_client_flow_bootstrap.cpp
        judge_client_flow_load_helpers.cpp
        judge_client_flow_log_helpers.cpp
        judge_client_flow_path_helpers.cpp
        judge_client_flow_prepare_helpers.cpp
        judge_client_flow_prep_helpers.cpp
        judge_client_flow_runner.cpp
        judge_client_flow_socket_helpers.cpp
        judge_client_flow_syscall_helpers.cpp
        judge_client_report_bundle_helpers.cpp
        judge_client_report_case_helpers.cpp
        judge_client_report_finalize_helpers.cpp
        judge_client_report_result_helpers.cpp
        judge_client_report_payload_helpers.cpp
        judge_client_report_score_helpers.cpp
        judge_client_report_task_helpers.cpp
        judge_client_report_test_run_helpers.cpp
        judge_client_run_helpers.cpp
        judge_client_run_io_helpers.cpp
        judge_client_run_judge_helpers.cpp
        judge_client_run_limit_helpers.cpp
        judge_client_run_path_helpers.cpp
        judge_client_run_series_helpers.cpp
        judge_client_path_utils.cpp
        judge_client_process_utils.cpp
        judge_client_watch_helpers.cpp
        judge_client_watch_handlers.cpp
        judge_client_watch_metrics.cpp
        judge_client_watch_phases.cpp
        judge_client_util_helpers.cpp
        judge_client_runtime_bridge.cpp
        judge_client_run.cpp
        judge_client_report.cpp
        judge_client_util.cpp
        judge_client_flow.cpp
        judge_client_context.cpp
        judge_client_flow_helpers.cpp
        judge_client_report_helpers.cpp
        judge_client_watch.cpp
        header/static_var.cpp
        library/judge_lib.h.cpp
        model/base/JSONVectorReader.cpp
        model/base/Bundle.cpp
        model/base/Pack.cpp
        model/websocket/WebSocketSender.cpp
        model/submission/SubmissionInfo.cpp
        model/judge/policy/SpecialJudge.cpp
        model/wrapper/CompilerConfigReader.cpp
        manager/syscall/InitManager.cpp
        external/mysql/MySQLSubmissionAdapter.cpp
        external/mysql/MySQLSubmissionInfoManager.cpp
        external/mysql/MySQLAutoPointer.cpp
        external/compare/Compare.cpp
        external/compare/CompareImpl.cpp
        model/config/ConfigInfo.cpp
        model/judge/language/Language.cpp
        model/judge/language/common/BonusLimit.cpp
        model/judge/language/common/CPython.cpp
        model/judge/language/common/seccomp_helper.cpp
        model/judge/language/C11.cpp
        model/judge/language/C99.cpp
        model/judge/language/Clang.cpp
        model/judge/language/Clang11.cpp
        model/judge/language/Clangpp.cpp
        model/judge/language/Clangpp17.cpp
        model/judge/language/Cpp11.cpp
        model/judge/language/Cpp17.cpp
        model/judge/language/Cpp20.cpp
        model/judge/language/Cpp98.cpp
        model/judge/language/Bash.cpp
        model/judge/language/Go.cpp
        model/judge/language/Java.cpp
        model/judge/language/Java6.cpp
        model/judge/language/Java7.cpp
        model/judge/language/Java8.cpp
        model/judge/language/Kotlin.cpp
        model/judge/language/KotlinNative.cpp
        model/judge/language/Python2.cpp
        model/judge/language/Python3.cpp
        model/judge/language/PyPy.cpp
        model/judge/language/PyPy3.cpp
        model/judge/language/Php.cpp
        model/judge/language/Perl.cpp
        model/judge/language/Ruby.cpp
        model/judge/language/Lua.cpp
        model/judge/language/Schema.cpp
        model/judge/language/JavaScript.cpp
        model/judge/language/Csharp.cpp
        model/judge/language/Objc.cpp
        model/judge/language/FreeBasic.cpp
        model/judge/language/Pascal.cpp
    )
    add_executable(judge_tests ${TEST_SOURCES})
    target_include_directories(judge_tests PRIVATE tests/fake_mysql tests/fake_seccomp)
    target_compile_definitions(judge_tests PRIVATE UNIT_TEST)
    target_compile_options(judge_tests PRIVATE -include ${CMAKE_SOURCE_DIR}/tests/test_hooks.h)
    target_link_libraries(judge_tests ${CMAKE_THREAD_LIBS_INIT})
    if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU" AND
        CMAKE_CXX_COMPILER_VERSION VERSION_LESS 9.0)
        target_link_libraries(judge_tests stdc++fs)
    endif ()
    add_test(NAME judge_tests COMMAND judge_tests)
endif ()

option(ENABLE_REAL_TESTS "Build real compile/run tests" OFF)
if (ENABLE_REAL_TESTS)
    add_executable(judge_real_tests
        tests/real_compile_run.cpp
        external/compare/Compare.cpp
        external/compare/CompareImpl.cpp
        external/compare/utils/utils.cpp
        model/judge/language/common/seccomp_helper.cpp
    )
    target_link_libraries(judge_real_tests ${CMAKE_THREAD_LIBS_INIT} seccomp)
endif ()
