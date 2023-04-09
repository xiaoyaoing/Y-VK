//
// Created by 打工人 on 2023/3/19.
//

#include "String.h"

std::string string_format(const std::string &fmt_str, ...) {
        int final_n, n = ((int) fmt_str.size()) * 2; /* reserve 2 times as much as the length of the fmt_str */
        std::string str;
        std::unique_ptr<char[]> formatted;
        va_list ap;
        while (true) {
            formatted.reset(new char[n]); /* wrap the plain char array into the unique_ptr */
            strcpy(&formatted[0], fmt_str.c_str());
            va_start(ap, fmt_str);
            final_n = vsnprintf(&formatted[0], n, fmt_str.c_str(), ap);
            va_end(ap);
            if (final_n < 0 || final_n >= n)
                n += abs(final_n - n + 1);
            else
                break;
        }
        return {formatted.get()};

}