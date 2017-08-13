#ifndef NODE_ADDON_H
#define NODE_ADDON_H

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
            static Local<Value> ParseReply(redisReply *r);
            NodeAddon(Local<Object>);
            static NAN_METHOD(New);
            static NAN_METHOD(Call);
            static NAN_METHOD(Get);
            static NAN_METHOD(Set);
    };
}
#endif
