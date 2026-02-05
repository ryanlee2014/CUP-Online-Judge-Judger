#pragma once

#include <functional>
#include <sys/types.h>

pid_t spawn_child(const std::function<void()> &fn);
