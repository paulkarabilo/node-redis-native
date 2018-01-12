#ifndef _NODE_REDIS_ADDON_CLIENT_H
#define _NODE_REDIS_ADDON_CLIENT_H

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

//convert param that comes from javascript into char*
#define STR_ARG(n) (*(Nan::Utf8String)(info[(n)]))

#define DEFAULT_REDIS_PORT 6379
#define DEFAULT_REDIS_HOST "localhost"

namespace node_redis_addon {
    class NodeRedisAddon : public Nan::ObjectWrap {
        public:
            static NAN_MODULE_INIT(Initialize);
            ~NodeRedisAddon();
        private:
            redisAsyncContext* context;
            char* host;
            uint16_t port;
            Nan::Callback* onConnect;
            Nan::Callback* onDisconnect;
            Nan::Callback* onSubscribe;
            static void RedisCallback(redisAsyncContext* c, void* r, void* privdata);
            static void BindCall(const Nan::FunctionCallbackInfo<Value>& info, Local<Value> cb, char* fmt);
            static void ConnectCallback(const redisAsyncContext* c, int status);
            static void DisconnectCallback(const redisAsyncContext* c, int status);
            static bool CheckSubscribeCallback(Local<Value> callbackReply);
            static Local<Value> ParseReply(redisReply *r);
            NodeRedisAddon(Local<Object>);
            static NAN_METHOD(New);
            static NAN_METHOD(Call);
            static NAN_METHOD(Disconnect);
    };
}
#endif
