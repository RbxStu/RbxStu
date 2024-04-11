//
// Created by Dottik on 3/4/2024.
//
#include <cstdint>
#include <StudioOffsets.h>
#include <lstate.h>
#include <cstdio>
#include "Security.hpp"

void RBX::Security::MarkThread(lua_State *L) {  // Unsafe operation. Do NOT do.
    if (L->userdata == nullptr) {
        // Call lua capabilities to alloc and set,
        RBX::Security::Bypasses::set_thread_security(L, RBX::Identity::Eight_Seven);
    }
    auto *plStateUd = static_cast<RBX::Lua::ExtraSpace *>(L->userdata);
    // Doing this messes Roblox up. do NOT do this.
    // plStateUd->globalActorState = 0xff; // Mark as our thread.
    plStateUd->executor_thread_mark = oxorany(0xff);
    printf("Marked thread: %d", plStateUd->executor_thread_mark == 0xff);
}

int64_t RBX::Security::deobfuscate_identity(RBX::Identity identity) {
    // Some identities are merged, which means that the output doesn't matter truly. We just need to support the highest one.
    switch (identity) {
        case RBX::Identity::One_Four:
            return oxorany(4);
        case RBX::Identity::Two:
            return oxorany(2);
        case Three_Six:
            return oxorany(6);
        case RBX::Identity::Five:
            return oxorany(5);
        case RBX::Identity::Eight_Seven:
            return oxorany(8);
        case RBX::Identity::Nine:
            return oxorany(9);
    }
    return oxorany(0);
}

int64_t RBX::Security::to_obfuscated_identity(int64_t identity) {
    switch (identity) {
        case 1:
        case 4:
            return (static_cast<int64_t>(RBX::Identity::One_Four));
        case 2:
            return (static_cast<int64_t>(RBX::Identity::Two));
        case 3:
        case 6:
            return (static_cast<int64_t>(RBX::Identity::Three_Six));
        case 5:
            return (static_cast<int64_t>(RBX::Identity::Five));
        case 7:
        case 8:
            return (static_cast<int64_t>(RBX::Identity::Eight_Seven));
        case 9:
            return (static_cast<int64_t>(RBX::Identity::Nine));
    }
    return oxorany(0);
}

void RBX::Security::Bypasses::set_thread_security(lua_State *L, RBX::Identity identity) {
    if (L->userdata == nullptr) {
        // Assume unallocated, what else would be 0 goddam.
        L->userdata = RBX::Studio::Functions::rbxAllocate(
                sizeof(RBX::Lua::ExtraSpace)); // Allocate structure for keyvals
    }

    // is NOT meant to represent the actual memory.
    auto *plStateUd = static_cast<RBX::Lua::ExtraSpace *>(L->userdata);

    //wprintf(oxorany(L"\r\n===== BEFORE MODIFICATION =====\r\n"));
    //wprintf(oxorany(L"[RBX::Security::Bypasses::set_thread_security]   Identity  : 0x%p\r\n"), plStateUd->identity);
    //wprintf(oxorany(L"[RBX::Security::Bypasses::set_thread_security] Capabilities: 0x%p\r\n"), plStateUd->capabilities);
    //wprintf(oxorany(L"[RBX::Security::Bypasses::set_thread_security]  taskStatus : 0x%p\r\n"), plStateUd->taskStatus);

    plStateUd->identity = RBX::Security::deobfuscate_identity(identity);
    plStateUd->capabilities = oxorany(0x3FFFF00) | RBX::Security::to_obfuscated_identity(
            RBX::Security::deobfuscate_identity(identity)); // Magical constant | Identity 8.
    plStateUd->taskStatus = oxorany(0);

    //wprintf(oxorany(L"\r\n=====  POST MODIFICATION  =====\r\n"));
    //wprintf(oxorany(L"\r\n[RBX::Security::Bypasses::set_thread_security]   Identity  : 0x%p\r\n"), plStateUd->identity);
    //wprintf(oxorany(L"[RBX::Security::Bypasses::set_thread_security] Capabilities: 0x%p\r\n"), plStateUd->capabilities);
    //wprintf(oxorany(L"[RBX::Security::Bypasses::set_thread_security]  taskStatus : 0x%p\r\n"), plStateUd->taskStatus);
}

void set_proto(Proto *proto, uintptr_t *proto_identity) {
    proto->userdata = static_cast<void *>(proto_identity);
    for (auto i = oxorany(0); i < proto->sizep; i++)
            set_proto(proto->p[i], proto_identity);
}

bool RBX::Security::Bypasses::set_luaclosure_security(Closure *cl) {
    if (cl->isC) return false;
    auto protos = cl->l.p;
    auto *mem = reinterpret_cast<std::uintptr_t *>(RBX::Studio::Functions::rbxAllocate(
            sizeof(std::uintptr_t)));
    *mem = oxorany(0x3FFFF00) | oxorany(0x3F);  // Magical constant | Identity 8.
    set_proto(protos, mem);
    return true;
}
