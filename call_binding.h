#include <uv.h>
#include "./addon.h"

namespace nodeaddon {
    typedef struct {
        uv_work_t req;
        NodeAddon* addon;
        Nan::Callback* callback;
    } CallBinding;
}