#ifndef _NODE_REDIS_ADDON_COMMAND_H
#define _NODE_REDIS_ADDON_COMMAND_H

namespace node_redis_addon {
    class Command {
    public:
        static char* Build(const char* fmt...);
        static bool Is(const char* input, const char* cmd);
    };
}

#endif /* command_h */
