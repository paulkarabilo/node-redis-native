#include <nan.h>
#include "../include/client.h"

NAN_MODULE_INIT(init) {
    node_redis_addon::NodeRedisAddon::Initialize(target);
}

NODE_MODULE(addon, init)
