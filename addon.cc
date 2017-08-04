#include <nan.h>
#include "./hiredis/hiredis.h"
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
            void AsyncCallback();
            NodeAddon();
            static NAN_METHOD(New);
            static NAN_METHOD(Call);
    };

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
        if (info.Length() != 2) {
            return Nan::ThrowTypeError("Call method accepts 2 arguments: command and callback");
        }
        if (!info[0]->IsString()) {
            return Nan::ThrowTypeError("Command must be a string");
        }
        if (!info[1]->IsFunction()) {
            return Nan::ThrowTypeError("Callback must be a function");
        }
        Local<String> cmd = info[0].As<String>();
        Local<Function> cb = Local<Function>::Cast(info[1]);
    }

    NodeAddon::NodeAddon() : Nan::ObjectWrap() {
        context = redisAsyncConnect("127.0.0.1", 6379);
    }

    NodeAddon::~NodeAddon() {
        redisAsyncDisconnect(context);
    }
}

NAN_MODULE_INIT(init) {
    nodeaddon::NodeAddon::Initialize(target);
}

NODE_MODULE(addon, init)