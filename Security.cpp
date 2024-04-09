//
// Created by Dottik on 3/4/2024.
//
#include <cstdint>
#include <StudioOffsets.h>
#include <lstate.h>
#include "Security.hpp"


void RBX::Security::Bypasses::SetLuastateCapabilities(lua_State *L) {
    if (L->userdata == nullptr) {
        // Assume unallocated, what else would be 0 goddam.
        L->userdata = RBX::Studio::Functions::rbxAllocate(
                sizeof(RBX::Lua::ExtraSpace)); // Allocate structure for keyvals
    }

    // is NOT meant to represent the actual memory.
    auto *plStateUd = static_cast<RBX::Lua::ExtraSpace *>(L->userdata);

    plStateUd->identity = 8;
    plStateUd->capabilities = 0x3FFFF00 | 0x3F; // Magical constant | Identity 8.
    plStateUd->taskStatus = 0;
}

void set_proto(Proto *proto, uintptr_t *proto_identity) {
    proto->userdata = static_cast<void *>(proto_identity);
    for (auto i = 0; i < proto->sizep; i++)
        set_proto(proto->p[i], proto_identity);
}

bool RBX::Security::Bypasses::SetClosureCapabilities(Closure *cl) {
    if (cl->isC) return false;
    auto protos = cl->l.p;
    auto *mem = reinterpret_cast<std::uintptr_t *>(RBX::Studio::Functions::rbxAllocate(
            sizeof(std::uintptr_t)));
    *mem = 0x3FFFF00 | 0x3F;  // Magical constant | Identity 8.
    set_proto(protos, mem);
    return true;
}
