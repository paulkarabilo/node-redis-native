#include "../hiredis/hiredis.h"
#include "../hiredis/async.h"
#include "../hiredis/adapters/libuv.h"
#include "../include/client.h"
#include "../include/call_binding.h"
#include "../include/parser.h"
#include "../include/command.h"
#include <iostream>

namespace node_redis_addon {
    NAN_MODULE_INIT(NodeRedisAddon::Initialize) {
        Local<FunctionTemplate> client = Nan::New<FunctionTemplate>(New);
        client->InstanceTemplate()->SetInternalFieldCount(1);
        Nan::SetPrototypeMethod(client, "call", Call);
        Nan::Set(target, Nan::New("Client").ToLocalChecked(), Nan::GetFunction(client).ToLocalChecked());
    }

    /**
     * Javascript client constructor wrapper
     */
    NAN_METHOD(NodeRedisAddon::New) {
        Local<Object> options = (info.Length() == 1 && info[0]->IsObject()) ?
            info[0]->ToObject() :
            Nan::New<Object>();
        NodeRedisAddon *addon = new NodeRedisAddon(options);
        addon->Wrap(info.This());
        info.GetReturnValue().Set(info.This());
    }

    NAN_METHOD(NodeRedisAddon::Call) {
        ASSERT_NARGS("Call", 2);
        ASSERT_STRING("Call", 0);
        ASSERT_FUNCTION("Call", 1);
        BindCall(info, info[1], STR_ARG(0));
    }

    /**
     * Fires redis async command with bound javascript callback
     */
    void NodeRedisAddon::BindCall(const Nan::FunctionCallbackInfo<Value>& info, Local<Value> cb, char* fmt) {
        NodeRedisAddon* addon = Nan::ObjectWrap::Unwrap<NodeRedisAddon>(info.Holder());
        Local<Function> callback = Local<Function>::Cast(cb); 
        char* command = Command::Build(fmt);

        if (command == NULL) {
            Local<Value> argv[1];
            argv[0] = Nan::New<String>("Failed to create command").ToLocalChecked();
            callback->Call(info.This(), 1, argv);
        } else if (addon->context->c.flags & REDIS_SUBSCRIBED && 
                !Command::Is(command, "subscribe") &&
                !Command::Is(command, "unsubscribe")) {
            // client is currently used in pub/sub mode, 
            // trying to make other async command will trigger exception 
            Local<Value> argv[1];
            argv[0] = Nan::New<String>("Redis client in subscription mode, can only run subscribe/unsubscribe commands").ToLocalChecked();
            callback->Call(info.This(), 1, argv);
            delete command;
        } else if (addon->context->c.flags & REDIS_MONITORING) {
            // same as previous case, but with monitoring mode
            Local<Value> argv[1];
            argv[0] = Nan::New<String>("Redis client in monitoring mode").ToLocalChecked();
            callback->Call(info.This(), 1, argv);
            delete command;
        } else {
            CallBinding* binding = new CallBinding(addon, callback);
            redisAsyncCommand(addon->context, RedisCallback, (void*)binding, command);
            delete command;
        }
    }

    /**
     * Calback method for redic async command
     * privdata contains reference to javascript command and addon object
     */
    void NodeRedisAddon::RedisCallback(redisAsyncContext* c, void* r, void* privdata) {
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
                        if (strncasecmp(firstValueChar, "subscribe", 9) == 0) {
                            if (binding->addon->onSubscribe != NULL) {
                                Local<Value> argv[1];
                                argv[0] = parsedReply;
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
        //binding will be reused in ongoing pubsub/monitor callbacks
        if (c->c.flags & REDIS_SUBSCRIBED || c->c.flags & REDIS_MONITORING) return;
        delete binding;
    }

    /**
     * Successful connect callback wrapper.
     * Status should equal 0 (REDIS_OK)
     */
    void NodeRedisAddon::ConnectCallback(const redisAsyncContext* c, int status) {
        Nan::HandleScope scope;
        NodeRedisAddon* addon = static_cast<NodeRedisAddon*>(c->data);
        if (addon->onConnect != NULL) {
            Local<Value> argv[1] = {Nan::New<Number>(status)};
            addon->onConnect->Call(1, argv);
        }
    }

    /**
     * Client disconnect callback wrapper
     * if triggered by QUIT command , then status code is -1 (REDIS_ERR)
     */
    void NodeRedisAddon::DisconnectCallback(const redisAsyncContext* c, int status) {
        Nan::HandleScope scope;
        NodeRedisAddon* addon = static_cast<NodeRedisAddon*>(c->data);
        if (addon->onDisconnect != NULL) {
            Local<Value> argv[1] = {Nan::New<Number>(status)};
            addon->onDisconnect->Call(1, argv);
        }
    }

    /**
     * Javascript addon object initializer
     */
    NodeRedisAddon::NodeRedisAddon(Local<Object> options) : Nan::ObjectWrap() {
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
            host = (char*)DEFAULT_REDIS_HOST;
        }
        if (_port->IsNumber()) {
            port = _port->IntegerValue();
        } else {
            port = DEFAULT_REDIS_PORT;
        }

        //try to create redis async client
        context = redisAsyncConnect(host, port);
        if (context->err && onError->IsFunction()) {
            Nan::Callback cb(Local<Function>::Cast(onError));
            int argc = 1;
            Local<Value> argv[argc];
            argv[0] = Nan::New<String>(context->errstr).ToLocalChecked();
            cb.Call(argc, argv);
        } else {
            //if client created successfully, store reference to 
            //addon object in its data property.
            context->data = (void*)this;
            //attach async client to main node event loop
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

    NodeRedisAddon::~NodeRedisAddon() {
        if (onDisconnect != nullptr) delete onDisconnect;
        if (onConnect != nullptr) delete onConnect;
        redisAsyncDisconnect(context);
    }
}