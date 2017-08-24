#include "../hiredis/hiredis.h"
#include "../hiredis/async.h"
#include "../hiredis/adapters/libuv.h"
#include "../include/addon.h"
#include "../include/call_binding.h"
#include <iostream>

using namespace std;

namespace nodeaddon {
    NAN_MODULE_INIT(NodeAddon::Initialize) {
        Local<FunctionTemplate> client = Nan::New<FunctionTemplate>(New);
        client->InstanceTemplate()->SetInternalFieldCount(1);
        Nan::SetPrototypeMethod(client, "call", Call);
        Nan::Set(target, Nan::New("Client").ToLocalChecked(), Nan::GetFunction(client).ToLocalChecked());
    }

    NAN_METHOD(NodeAddon::New) {
        Local<Object> options;
        if (info.Length() == 1 && info[0]->IsObject()) {
            options = info[0]->ToObject();
        } else {
            options = Nan::New<Object>();
        }
        NodeAddon *addon = new NodeAddon(options);
        addon->Wrap(info.This());
        info.GetReturnValue().Set(info.This());
    }

    NAN_METHOD(NodeAddon::Call) {
        ASSERT_NARGS("Call", 2);
        ASSERT_STRING("Call", 0);
        ASSERT_FUNCTION("Call", 1);
        BIND_CALL(info[1], STR_ARG(0));
    }

    Local<Value> NodeAddon::ParseReply(redisReply* r) {
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
            for (uint32_t i = 0; i < r->len; i++) {
                a->Set(i, ParseReply(r->element[i]));
            }
            return scope.Escape(a);
        }

        return scope.Escape(Nan::Null());
    }

    void NodeAddon::RedisCallback(redisAsyncContext* c, void* r, void* privdata) {
        Nan::HandleScope scope;
        redisReply* reply = static_cast<redisReply*>(r);
        CallBinding* binding = static_cast<CallBinding*>(privdata);
        int argc = 2;
        Local<Value> argv[argc];
        if (reply->type == REDIS_ERR) {
            argv[0] = Nan::New<String>(reply->str).ToLocalChecked();
            argv[1] = Nan::Null();
        } else {
            argv[0] = Nan::Null();
            argv[1] = ParseReply(reply);
        }
        binding->callback->Call(argc, argv);
        delete binding;
    }

    void NodeAddon::ConnectCallback(const redisAsyncContext* c, int status) {
        Nan::HandleScope scope;
        NodeAddon* addon = static_cast<NodeAddon*>(c->data);
        if (addon->onConnect != nullptr) {
            int argc = 1;
            Local<Value> argv[argc] = {Nan::New<Number>(status)};
            addon->onConnect->Call(argc, argv);
        }
    }

    void NodeAddon::DisconnectCallback(const redisAsyncContext* c, int status) {
        Nan::HandleScope scope;
        NodeAddon* addon = static_cast<NodeAddon*>(c->data);
        if (addon->onDisconnect != nullptr) {
            int argc = 1;
            Local<Value> argv[argc] = {Nan::New<Number>(status)};
            addon->onDisconnect->Call(argc, argv);
        }
    }

    NodeAddon::NodeAddon(Local<Object> options) : Nan::ObjectWrap() {
        Nan::HandleScope scope;
        Local<Value> _host = Nan::Get(options, Nan::New<String>("host").ToLocalChecked()).ToLocalChecked();
        Local<Value> _port = Nan::Get(options, Nan::New<String>("port").ToLocalChecked()).ToLocalChecked();
        Local<Value> _onConnect = Nan::Get(options, Nan::New<String>("onConnect").ToLocalChecked()).ToLocalChecked();
        Local<Value> _onDisconnect = Nan::Get(options, Nan::New<String>("onDisconnect").ToLocalChecked()).ToLocalChecked();
        Local<Value> onError = Nan::Get(options, Nan::New<String>("onError").ToLocalChecked()).ToLocalChecked();
        if (_host->IsString()) {
            Nan::Utf8String host_val(_host);
            host = *host_val;
        } else {
            host = (char*)"localhost";
        }
        if (_port->IsNumber()) {
            port = _port->IntegerValue();
        } else {
            port = 6379;
        }

        context = redisAsyncConnect(host, port);
        if (context->err && onError->IsFunction()) {
            Nan::Callback cb(Local<Function>::Cast(onError));
            int argc = 1;
            Local<Value> argv[argc] {Nan::New<String>(context->errstr).ToLocalChecked()};
            cb.Call(argc, argv);
        } else {
            context->data = (void*)this;
            if (_onConnect->IsFunction()) {
                onConnect = new Nan::Callback(Local<Function>::Cast(_onConnect));
                redisAsyncSetConnectCallback(context, ConnectCallback);
            }
            if (_onDisconnect->IsFunction()) {
                onDisconnect = new Nan::Callback(Local<Function>::Cast(_onDisconnect));
                redisAsyncSetDisconnectCallback(context, DisconnectCallback);
            }
            redisLibuvAttach(context, uv_default_loop());
        }
    }

    NodeAddon::~NodeAddon() {
        if (onDisconnect != nullptr) delete onDisconnect;
        if (onConnect != nullptr) delete onConnect;
        redisAsyncDisconnect(context);
        redisAsyncFree(context);
    }
}

NAN_MODULE_INIT(init) {
    nodeaddon::NodeAddon::Initialize(target);
}

NODE_MODULE(addon, init)
