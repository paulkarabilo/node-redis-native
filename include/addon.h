#ifndef NODE_ADDON_H
#define NODE_ADDON_H

#include <nan.h>
#include "./hiredis/async.h"

using namespace v8;
using namespace node;

#define STR_(n) #n
#define STR(n) STR_(n)

#define ASSERT_NARGS(name, n)      \
    if (info.Length() != n) { \
        return Nan::ThrowError("Method " name " accepts " STR(n) " arguments"); \
    }

#define ASSERT_TYPE(name, i, type) \
    if (!info[i]->Is##type()) { \
        return Nan::ThrowTypeError("Method " name " expects argument #" STR(i) "of type " STR(type)); \
    }

#define ASSERT_NUMBER(name, i) ASSERT_TYPE(name, i, Number)

#define ASSERT_STRING(name, i) ASSERT_TYPE(name, i, String)

#define ASSERT_FUNCTION(name, i) ASSERT_TYPE(name, i, Function)

#define BIND_CALL(command, cb) \
    Local<Function> callback = Local<Function>::Cast(cb); \
    NodeAddon* addon = Nan::ObjectWrap::Unwrap<NodeAddon>(info.Holder()); \
    CallBinding* binding = new CallBinding(addon, callback); \
    redisAsyncCommand(addon->context, RedisCallback, (void*)binding, (command));

namespace nodeaddon {
    class NodeAddon : public Nan::ObjectWrap {
        public:
            static NAN_MODULE_INIT(Initialize);
            ~NodeAddon();
        private:
            redisAsyncContext* context;
            char* host;
            uint16_t port;
            static void RedisCallback(redisAsyncContext* c, void* r, void* privdata);
            static Local<Value> ParseReply(redisReply *r);
            NodeAddon(Local<Object>);
            static NAN_METHOD(New);
            static NAN_METHOD(Call);
            static NAN_METHOD(Get);
            static NAN_METHOD(Set);
            static NAN_METHOD(Incr);
            static NAN_METHOD(Del);
            static NAN_METHOD(Exists);
    };
}
#endif
