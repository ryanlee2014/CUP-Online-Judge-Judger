#!/bin/bash
# builds ./cupoj-compare from the legacy checker sources (same code the old judge_client uses)
set -e
cd "$(dirname "$0")"
CXX=${CXX:-g++}
$CXX -std=c++17 -O2 -Wall -pthread -o cupoj-compare compare_main.cpp ../../external/compare/CompareImpl.cpp ../../external/compare/Compare.cpp ../../external/compare/utils/utils.cpp
