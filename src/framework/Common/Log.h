#pragma once

#include <spdlog/spdlog.h>
#include <stdexcept>

#ifndef ROOT_PATH_SIZE
#	define ROOT_PATH_SIZE 0
#endif

#define __FILENAME__ (static_cast<const char *>(__FILE__) + ROOT_PATH_SIZE)


#define LOGI(...) spdlog::info(__VA_ARGS__);
#define LOGW(...) spdlog::warn(__VA_ARGS__);
#define LOGE(...) {spdlog::error("[{}:{}] {}", __FILENAME__, __LINE__, fmt::format(__VA_ARGS__)); throw std::runtime_error("ERROR");}
#define LOGD(...) spdlog::debug(__VA_ARGS__);

  // void DebugBreak() ;
