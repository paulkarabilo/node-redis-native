#ifndef _NODEADDON_COMMAND_H
#define _NODEADDON_COMMAND_H

namespace nodeaddon {
    class Command {
    public:
        static char* Build(const char* fmt...);
        static bool Is(const char* input, const char* cmd);
    };
}

#endif /* command_h */
