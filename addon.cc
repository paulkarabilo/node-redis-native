#include "./hiredis/hiredis.h"
#include "./hiredis/async.h"
#include "./hiredis/adapters/libuv.h"
#include "./addon.h"
#include "./call_binding.h"
#include <string.h>
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
        if (info.IsConstructCall()) {
            NodeAddon *addon = new NodeAddon();
            addon->Wrap(info.This());
            info.GetReturnValue().Set(info.This());
        } else {
            //Make non-constructor call work
        }
    }

    NAN_METHOD(NodeAddon::Call) {
        Nan::HandleScope scope;
        if (info.Length() != 2) {
            return Nan::ThrowTypeError("Call method accepts 2 arguments: command and callback");
        }
        if (!info[0]->IsString()) {
            return Nan::ThrowTypeError("Command must be a string");
        }
        if (!info[1]->IsFunction()) {
            return Nan::ThrowTypeError("Callback must be a function");
        }
        String::Utf8Value cmdUtf(info[0]->ToString());
        string cmdStr = string(*cmdUtf);

        Local<Function> cb = Local<Function>::Cast(info[1]);
        NodeAddon* addon = Nan::ObjectWrap::Unwrap<NodeAddon>(info.Holder());
        CallBinding* binding = new CallBinding;
        binding->addon = addon;
        binding->callback = new Nan::Callback(cb);
        redisAsyncCommand(addon->context, RedisCallback, (void*)binding, cmdStr.c_str());
    }

    void NodeAddon::RedisCallback(redisAsyncContext* c, void* r, void* privdata) {
        Nan::HandleScope scope;
        CallBinding* binding = static_cast<CallBinding*>(privdata);
        const unsigned int argc = 1;
        Local<Value> argv[argc];
        argv[0] = Nan::Null();
        binding->callback->Call(argc, argv);
    }


    NodeAddon::NodeAddon() : Nan::ObjectWrap() {
        context = redisAsyncConnect("localhost", 6379);
        redisLibuvAttach(context, uv_default_loop());
    }

    NodeAddon::~NodeAddon() {
        redisAsyncDisconnect(context);
    }
}

NAN_MODULE_INIT(init) {
    nodeaddon::NodeAddon::Initialize(target);
}

NODE_MODULE(addon, init)