#pragma once
#include <nan.h>
#include "./hiredis/async.h"

using namespace v8;
using namespace node;

namespace nodeaddon {
    class NodeAddon : public Nan::ObjectWrap {
        public:
            static NAN_MODULE_INIT(Initialize);
            ~NodeAddon();
        private:
            redisAsyncContext* context;
            static void RedisCallback(redisAsyncContext* c, void* r, void* privdata);
            NodeAddon();
            static NAN_METHOD(New);
            static NAN_METHOD(Call);
    };
}