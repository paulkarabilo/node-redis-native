#ifndef _NODE_REDIS_ADDON_CALL_BINDING_H
#define _NODE_REDIS_ADDON_CALL_BINDING_H

#include <uv.h>
#include "./client.h"

namespace node_redis_addon {
    typedef struct CallBinding {
        NodeRedisAddon* addon;
        Nan::Callback* callback;
        CallBinding();
        CallBinding(NodeRedisAddon* a, Local<Function> cb) {
            addon = a;
            callback = new Nan::Callback(cb);
        };
        ~CallBinding() {
            delete callback;
        };
    } CallBinding;
}

#endif
