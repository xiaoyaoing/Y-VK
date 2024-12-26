//
// Created by 打工人 on 2023/3/19.
//

#include "Log.h"

// void DebugBreak() {
//     __debugbreak();
// }
void Log::DebugBreakPoint() {
    throw std::runtime_error("DebugBreakPoint");
}