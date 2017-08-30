#include <nan.h>
#include "../include/addon.h"

NAN_MODULE_INIT(init) {
    nodeaddon::NodeAddon::Initialize(target);
}

NODE_MODULE(addon, init)
