#ifndef NODE_ADDON_CALL_BINDING_H
#define NODE_ADDON_CALL_BINDING_H

#include <uv.h>
#include "./addon.h"

namespace nodeaddon {
    typedef struct {
        NodeAddon* addon;
        Nan::Callback* callback;
    } CallBinding;
}

#endif
