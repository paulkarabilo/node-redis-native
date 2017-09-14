#include "../hiredis/hiredis.h"
#include "../hiredis/async.h"
#include "../hiredis/adapters/libuv.h"
#include "../include/client.h"
#include "../include/call_binding.h"
#include "../include/parser.h"
#include "../include/command.h"
#include <iostream>

namespace nodeaddon {
    NAN_MODULE_INIT(NodeAddon::Initialize) {
        Local<FunctionTemplate> client = Nan::New<FunctionTemplate>(New);
        client->InstanceTemplate()->SetInternalFieldCount(1);
        Nan::SetPrototypeMethod(client, "call", Call);
        Nan::Set(target, Nan::New("Client").ToLocalChecked(), Nan::GetFunction(client).ToLocalChecked());
    }

    NAN_METHOD(NodeAddon::New) {
        Local<Object> options = (info.Length() == 1 && info[0]->IsObject()) ?
            info[0]->ToObject() :
            Nan::New<Object>();
        NodeAddon *addon = new NodeAddon(options);
        addon->Wrap(info.This());
        info.GetReturnValue().Set(info.This());
    }

    NAN_METHOD(NodeAddon::Call) {
        ASSERT_NARGS("Call", 2);
        ASSERT_STRING("Call", 0);
        ASSERT_FUNCTION("Call", 1);
        BindCall(info, info[1], STR_ARG(0));
    }

    void NodeAddon::BindCall(const Nan::FunctionCallbackInfo<Value>& info, Local<Value> cb, char* fmt) {
        NodeAddon* addon = Nan::ObjectWrap::Unwrap<NodeAddon>(info.Holder());
        Local<Function> callback = Local<Function>::Cast(cb); 
        char* command = Command::Build(fmt);
        if (command == NULL) {
            Local<Value> argv[1];
            argv[0] = Nan::New<String>("Failed to create command").ToLocalChecked();
            callback->Call(info.This(), 1, argv);
        } else if (addon->context->c.flags & REDIS_SUBSCRIBED && 
                !Command::Is(command, "subscribe") &&
                !Command::Is(command, "unsubscribe")) {
            Local<Value> argv[1];
            argv[0] = Nan::New<String>("Redis client in subscription mode, can only run subscribe/unsubscribe commands").ToLocalChecked();
            callback->Call(info.This(), 1, argv);
        } else if (addon->context->c.flags & REDIS_MONITORING) {
            Local<Value> argv[1];
            argv[0] = Nan::New<String>("Redis client in monitoring mode").ToLocalChecked();
            callback->Call(info.This(), 1, argv);
        } else {
            CallBinding* binding = new CallBinding(addon, callback);
            redisAsyncCommand(addon->context, RedisCallback, (void*)binding, command);
            delete command;
        }
    }

    void NodeAddon::RedisCallback(redisAsyncContext* c, void* r, void* privdata) {
        Nan::HandleScope scope;
        redisReply* reply = static_cast<redisReply*>(r);
        CallBinding* binding = static_cast<CallBinding*>(privdata);
        if (reply == NULL) {
            delete binding;
            return;
        }
        int argc = 2;
        Local<Value> argv[argc];
        if (reply->type == REDIS_REPLY_ERROR) {
            argv[0] = Nan::New<String>(reply->str).ToLocalChecked();
            argv[1] = Nan::Null();
        } else {
            Local<Value> parsedReply = Parser::ParseReply(reply);
            if (parsedReply->IsArray()) {
                Local<Array> replyArray = Local<Array>::Cast(parsedReply);
                if (replyArray->Length() == 3) {
                    Local<Value> firstValue = replyArray->Get(Nan::New<Number>(0));
                    if (firstValue->IsString()) {
                        Nan::Utf8String firstString(firstValue);
                        char* firstValueChar = *firstString;
                        NodeAddon* addon = binding->addon;
                        if (strncasecmp(firstValueChar, "subscribe", 9) == 0) {
                            if (addon->onSubscribe != NULL) {
                                Local<Value> argv[1] = {Nan::New<Number>(0)};
                                binding->addon->onSubscribe->Call(1, argv);
                            }
                            return;
                        }
                    }
                }
            }
            argv[0] = Nan::Null();
            argv[1] = parsedReply;
        }
        binding->callback->Call(argc, argv);
        if (c->c.flags & REDIS_SUBSCRIBED || c->c.flags & REDIS_MONITORING) return;
        delete binding;
    }

    void NodeAddon::ConnectCallback(const redisAsyncContext* c, int status) {
        Nan::HandleScope scope;
        NodeAddon* addon = static_cast<NodeAddon*>(c->data);
        if (addon->onConnect != NULL) {
            Local<Value> argv[1] = {Nan::New<Number>(status)};
            addon->onConnect->Call(1, argv);
        }
    }

    void NodeAddon::DisconnectCallback(const redisAsyncContext* c, int status) {
        Nan::HandleScope scope;
        NodeAddon* addon = static_cast<NodeAddon*>(c->data);
        if (addon->onDisconnect != NULL) {
            Local<Value> argv[1] = {Nan::New<Number>(status)};
            addon->onDisconnect->Call(1, argv);
        }
    }

    NodeAddon::NodeAddon(Local<Object> options) : Nan::ObjectWrap() {
        Nan::HandleScope scope;
        Local<Value> _host = Nan::Get(options, Nan::New<String>("host").ToLocalChecked()).ToLocalChecked();
        Local<Value> _port = Nan::Get(options, Nan::New<String>("port").ToLocalChecked()).ToLocalChecked();
        Local<Value> _onConnect = Nan::Get(options, Nan::New<String>("onConnect").ToLocalChecked()).ToLocalChecked();
        Local<Value> _onDisconnect = Nan::Get(options, Nan::New<String>("onDisconnect").ToLocalChecked()).ToLocalChecked();
        Local<Value> _onSubscribe = Nan::Get(options, Nan::New<String>("onSubscribe").ToLocalChecked()).ToLocalChecked();
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
            Local<Value> argv[argc];
            argv[0] = Nan::New<String>(context->errstr).ToLocalChecked();
            cb.Call(argc, argv);
        } else {
            context->data = (void*)this;
            redisLibuvAttach(context, uv_default_loop());
            if (_onConnect->IsFunction()) {
                onConnect = new Nan::Callback(Local<Function>::Cast(_onConnect));
                redisAsyncSetConnectCallback(context, ConnectCallback);
            } else {
                onConnect = NULL;
            }
            if (_onDisconnect->IsFunction()) {
                onDisconnect = new Nan::Callback(Local<Function>::Cast(_onDisconnect));
                redisAsyncSetDisconnectCallback(context, DisconnectCallback);
            } else {
                onDisconnect = NULL;
            }
            if (_onSubscribe->IsFunction()) {
                onSubscribe = new Nan::Callback(Local<Function>::Cast(_onSubscribe));
            } else {
                onSubscribe = NULL;
            }
        }
    }

    NodeAddon::~NodeAddon() {
        if (onDisconnect != nullptr) delete onDisconnect;
        if (onConnect != nullptr) delete onConnect;
        redisAsyncDisconnect(context);
    }
}