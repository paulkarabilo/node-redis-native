

#ifndef _COMMAND_H
#define _COMMAND_H


using namespace v8;

namespace nodeaddon {
    class Command {
    public:
        static char* Build(const char* fmt...);
    };
}

#endif /* command_h */
