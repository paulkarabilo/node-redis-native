#ifndef _NODEADDON_COMMAND_H
#define _NODEADDON_COMMAND_H

namespace nodeaddon {
    class Command {
    public:
        static char* Build(const char* fmt...);
    };
}

#endif /* command_h */
