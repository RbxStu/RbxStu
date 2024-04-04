//
// Created by Dottik on 3/4/2024.
//
#include <cstdint>
#include <StudioOffsets.h>
#include <lstate.h>

using Studio_LuaStateUserData [[maybe_unused]] = struct {
    uint8_t __garbage__filler__[48];
    uintptr_t user_facing_identity;
    uint8_t __filler__[8];
    uintptr_t obfucated_identity;
};

static void SetLuastateCapabilities(lua_State *L) {
    // Studio_LuaStateUserData is NOT meant to represent the actual memory, just key values.
    auto *plStateUd = static_cast<Studio_LuaStateUserData *>(L->userdata);
    plStateUd->user_facing_identity = 8;
    plStateUd->obfucated_identity = 0x3FFFF00 | 0x3F; // Magical constant | Identity 8.
}

static void set_proto(Proto *proto, uintptr_t *proto_identity) {
    proto->userdata = static_cast<void *>(proto_identity);
    for (auto i = 0; i < proto->sizep; i++)
        set_proto(proto->p[i], proto_identity);
}

static void SetClosureCapabilities(Closure *cl) {
    auto protos = cl->l.p;
    auto *mem = new uintptr_t{};    // We must allocate this on the heap, as ud is a pointer, if we set it as a value, it is not going to work.
    *mem = 0x3FFFF00 | 0x3F;  // Magical constant | Identity 8.
    set_proto(protos, mem);
}
