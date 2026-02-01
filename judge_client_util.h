#pragma once

#include <string>

void init_parameters(int argc, char **argv, int &solution_id,
                     int &runner_id, std::string &judgerId);
void print_call_array();
