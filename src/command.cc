#include "../include/command.h"
#include <cstdarg>
#include <cstdio>
#include <cstring>

namespace nodeaddon {
    char* Command::Build(const char* fmt...) {
        va_list args;
        va_start(args, fmt);
        
        int size = vsnprintf(NULL, 0, fmt, args);
        if (size < 0) {
            va_end(args);
            return NULL;
        }
        char* cmd = new char[size + 1];
        size = vsnprintf(cmd, size + 1, fmt, args);
        va_end(args);
        if (size < 0) return NULL;
        return cmd;
    }
    bool Command::Is(const char* input, const char* cmd) {
        unsigned int fns = 0; //location of first non-space character;
        size_t cl = strlen(cmd); //command length
        size_t il = strlen(input); // full input length

        //in case input has leading whitespace need to skip
        while(input[fns] == ' ') fns++;
        return strncasecmp((char*)(input + fns), cmd, cl) == 0;
    }
}
