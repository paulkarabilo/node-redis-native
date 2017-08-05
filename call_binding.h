#include <uv.h>
#include "./addon.h"

namespace nodeaddon {
    typedef struct {
        NodeAddon* addon;
        Nan::Callback* callback;
    } CallBinding;
}