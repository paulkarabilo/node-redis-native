#ifndef NODE_ADDON_CALL_BINDING_H
#define NODE_ADDON_CALL_BINDING_H

#include <uv.h>
#include "./addon.h"

namespace nodeaddon {
    typedef struct CallBinding {
        NodeAddon* addon;
        Nan::Callback* callback;
        CallBinding();
        CallBinding(NodeAddon* a, Local<Function> cb) {
            addon = a;
            callback = new Nan::Callback(cb);
        };
        ~CallBinding() {
            delete callback;
        };
    } CallBinding;
}

#endif
