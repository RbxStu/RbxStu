//
// Created by Dottik on 3/4/2024.
//
#include "Security.hpp"
#include <StudioOffsets.h>
#include <Utilities.hpp>
#include <cstdint>
#include <cstdio>
#include <lstate.h>

int64_t RBX::Security::deobfuscate_identity(RBX::Identity identity) {
    // Some identities are merged, which means that the output doesn't matter truly. We just need to support the highest
    // one.
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

int64_t RBX::Security::to_obfuscated_identity(int64_t identity) {
    switch (identity) {
        case 1:
        case 4:
            return RBX::Identity::One_Four;
        case 2:
            return RBX::Identity::Two;
        case 3:
        case 6:
            return RBX::Identity::Three_Six;
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

RBX::Lua::ExtraSpace *RBX::Security::Bypasses::reallocate_extraspace(lua_State *L) {
    const auto ud = static_cast<RBX::Lua::ExtraSpace *>(malloc(sizeof(RBX::Lua::ExtraSpace)));

    if (Module::Utilities::is_pointer_valid(static_cast<RBX::Lua::OriginalExtraSpace *>(L->userdata))) {
        // If userdata is valid, we must copy it to our new buffer.
        // We must only copy the SIZE of Robloxs' ExtraSpace, not ours.
        memcpy(ud, L->userdata, 0x98);
    }

    L->userdata = ud;
    return ud;
}

void RBX::Security::Bypasses::set_thread_security(lua_State *L, const RBX::Identity identity) {
    // Assume unallocated, what else would be 0 goddam.
    if (!Module::Utilities::is_pointer_valid(static_cast<RBX::Lua::ExtraSpace *>(L->userdata)))
        L->userdata = malloc(sizeof(RBX::Lua::ExtraSpace));

    auto *plStateUd = static_cast<RBX::Lua::ExtraSpace *>(L->userdata);

    plStateUd->identity = RBX::Security::deobfuscate_identity(identity);
    plStateUd->capabilities =
            0x3FFFF00 | RBX::Security::to_obfuscated_identity(RBX::Security::deobfuscate_identity(identity));
    // Magical constant | Custom_Identity (Or Capabilities in some cases)
}

static void set_proto(Proto *proto, uintptr_t *proto_identity) {
    proto->userdata = static_cast<void *>(proto_identity);
    for (auto i = 0; i < proto->sizep; i++)
        set_proto(proto->p[i], proto_identity);
}

bool RBX::Security::Bypasses::set_luaclosure_security(Closure *cl, const RBX::Identity identity) {
    if (cl->isC)
        return false;
    const auto pProto = cl->l.p;
    auto *pMem = pProto->userdata != nullptr ? static_cast<std::uintptr_t *>(pProto->userdata)
                                             : static_cast<std::uintptr_t *>(malloc(sizeof(std::uintptr_t)));
    *pMem = 0x3FFFF00 | (RBX::Security::to_obfuscated_identity(
                                RBX::Security::deobfuscate_identity(identity))); // Magical constant | Identity 8.
    set_proto(pProto, pMem);
    return true;
}

void RBX::Security::Bypasses::wipe_proto(Closure *lClosure) {
    if (lClosure->isC)
        return;
    auto proto = lClosure->l.p;
    proto->debugname = nullptr;
    proto->linedefined = -1;
}
