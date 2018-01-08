
#ifndef _PARSER_H
#define _PARSER_H

#include <v8.h>
#include <nan.h>

#include "../hiredis/hiredis.h"
using namespace v8;

namespace nodeaddon {
    class Parser {
        public:
            static Local<Value> ParseReply(redisReply* r);
    };
}

#endif