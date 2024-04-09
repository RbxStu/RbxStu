//
// Created by Dottik on 3/4/2024.
//
#include <cstdint>
#include <StudioOffsets.h>
#include <lstate.h>
#include "Security.hpp"

void RBX::Security::MarkThread(lua_State *L) {
    if (L->userdata == nullptr) {
        // Call lua capabilities to alloc and set,
        RBX::Security::Bypasses::SetLuastateCapabilities(L, RBX::Identity::Eight_Seven);
    }
    auto *plStateUd = static_cast<RBX::Lua::ExtraSpace *>(L->userdata);
    // Doing this messes Roblox up. do NOT do this.
    // plStateUd->globalActorState = 0xff; // Mark as our thread.
}

int64_t RBX::Security::DeobfuscateIdentity(int64_t identity) {
    // Some identities are merged, which means that the output doesn't matter truly. We just need to support the highest one.
    switch (identity) {
        case RBX::Identity::One_Four:
            return 4;
        case RBX::Identity::Two:
            return 2;
        case Three_Six:
            return 6;
        case RBX::Identity::Five:
            return 5;
        case RBX::Identity::Eight_Seven:
            return 8;
        case RBX::Identity::Nine:
            return 9;
    }
    return 0;
}

int64_t RBX::Security::ObfuscateIdentity(int64_t identity) {
    switch (identity) {
        case 1:
        case 4:
            return RBX::Identity::One_Four;
        case 2:
            return RBX::Identity::Two;
        case 3:
        case 6:
            return Three_Six;
        case 5:
            return RBX::Identity::Five;
        case 7:
        case 8:
            return RBX::Identity::Eight_Seven;
        case 9:
            return RBX::Identity::Nine;
    }
    return 0;
}

void RBX::Security::Bypasses::SetLuastateCapabilities(lua_State *L, RBX::Identity identity) {
    if (L->userdata == nullptr) {
        // Assume unallocated, what else would be 0 goddam.
        L->userdata = RBX::Studio::Functions::rbxAllocate(
                sizeof(RBX::Lua::ExtraSpace)); // Allocate structure for keyvals
    }

    // is NOT meant to represent the actual memory.
    auto *plStateUd = static_cast<RBX::Lua::ExtraSpace *>(L->userdata);

    plStateUd->identity = RBX::Security::DeobfuscateIdentity(identity);
    plStateUd->capabilities = 0x3FFFF00 | RBX::Security::ObfuscateIdentity(identity); // Magical constant | Identity 8.
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
