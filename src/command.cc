#include <cstdarg>
#include <cstdio>

namespace nodeaddon {
    class Command {
        static char* Build(const char* fmt...) {
            va_list args;
            va_start(args, fmt);
            
            int size = vsnprintf(nullptr, 0, fmt, args);
            if (size < 0) {
                va_end(args);
                return NULL;
            }
            char* cmd = new char[size + 1];
            size = vsnprintf(cmd, size, fmt, args);
            va_end(args);
            if (size < 0) return NULL;
            return cmd;
        }
    };
}
