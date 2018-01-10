#ifndef _NODE_REDIS_ADDON_PARSER_H
#define _NODE_REDIS_ADDON_PARSER_H

#include <v8.h>
#include <nan.h>

#include "../hiredis/hiredis.h"
using namespace v8;

namespace node_redis_addon {
    class Parser {
        public:
            static Local<Value> ParseReply(redisReply* r);
    };
}

#endif