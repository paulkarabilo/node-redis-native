#include <v8.h>
#include "../include/parser.h"

namespace node_redis_addon {
    /**
     * Convert redis command reply into a javascript 
     * runtime object that can be passed out of scope 
     */
    Local<Value> Parser::ParseReply(redisReply* r) {
        Nan::EscapableHandleScope scope;
        Local<Value> reply;

        if (r->type == REDIS_REPLY_NIL) {
            return scope.Escape(Nan::Null());
        } else if (r->type == REDIS_REPLY_INTEGER) {
            return scope.Escape(Nan::New<Number>(r->integer));
        } else if (r->type == REDIS_REPLY_STRING || r->type == REDIS_REPLY_STATUS) {
            return scope.Escape(Nan::New<String>(r->str, r->len).ToLocalChecked());
        } else if (r->type == REDIS_REPLY_ARRAY) {
            Local<Array> a = Nan::New<Array>();
            for (uint32_t i = 0; i < r->elements; i++) {
                a->Set(i, ParseReply(r->element[i]));
            }
            return scope.Escape(a);
        }
        return scope.Escape(Nan::Null());
    }
}