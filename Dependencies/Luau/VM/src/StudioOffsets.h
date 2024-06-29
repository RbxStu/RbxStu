//
// Created by Dottik on 3/4/2024.
//
#pragma once

#include "Windows.h"
#include <cstdint>

struct lua_TValue;
#define RebaseAddress(x) (x - 0x140000000 + reinterpret_cast<uintptr_t>(GetModuleHandleA("RobloxStudioBeta.exe")))

namespace RBX::Studio::Offsets {
    // Search for "C Stack Overflow"
    // Signature: 80 79 06 00 0F 85 ? ? ? ? E9 ? ? ? ?
    static std::uintptr_t luau_execute = RebaseAddress(0x1435131d0);

    // Get to lua_pushvalue and get inside of pseudo2addr, you will find the data xref.
    const static std::uintptr_t _luaO_nilobject = RebaseAddress(0x146fb3a18);

    // Find " ,"metatable": ", and go to the top of the second xref, a ldebug
    // function. It will contain two references to luaH_dummynode.
    const static std::uintptr_t _luaH_dummynode = RebaseAddress(0x146fb38d8);

    // The way we get our lua state is the classic, hooking.
    // Signature: 0F 84 88 00 00 00 81 FA EF D8 FF FF 74 45 81 FA F0 D8 FF FF 74 32 48 8B 41 ? 44 2B CA 48 8B 48 ? 4C 8B 01 41 0F B6 40 ? 44 3B C8 7F 12 41 8D 41 ? 48 98 48 83 C0 ? 48 C1 E0 ? 49 03 C0 C3
    static std::uintptr_t pseudo2addr = RebaseAddress(0x143504bb0);

    // search for coroutine.wrap. Inside there will be a lua_pushthread and lua_newthread call. Inside lua_newthread luaE_newthread is called.
    // Signature: E8 ? ? ? ? 48 8B 57 ? 48 8B D8 44 0F B6 42 ? C6 00 ? 41 80 E0 ? 44 88 40 ? 0F B6 57 ? 88 50 ? 48 8B D7 48 8B 4F ? 48 89 48 ? 33 C0 89 43
    // ? 48 8B CB 48 89 43 ? 48 89 43 ? 48 89 43 ? 48 89 43 ? 48 89 43 ? 48 89 43 ? 48 89 43 ? 48 89 43 ? 48 89 43 ? 0F B6 47 ? 88 43 ? E8 ? ? ? ? 48 8B
    // 47 ? 48 89 43 ? 0F B6 47 ? 88 43 ?
    static std::uintptr_t rluaE_newthread = RebaseAddress(0x1435060a0);

    // search for "memory allocation error: block too big". You will find luaG_runerrorL, last call is luaD_throw.
    // Signature for luaG_runerrorL (Calls luaD_throw): 48 89 50 ? 4C 89 40 ? 4C 89 48 ? 53 48 81 EC ? ? ? ? 48 8B D9 4C 8D 48 ? 4C 8B C2 48 8D 4C 24 ? BA
    // ? ? ? ? E8 ? ? ? ? 48 8D 54 24 ? 48 8B CB E8 ? ? ? ? BA ? ? ? ? 48 8B CB E8 ? ? ? ?
    static std::uintptr_t rLuaD_throw = RebaseAddress(0x143509df0);

    // search for coroutine.wrap. Inside there will be a lua_pushthread and lua_newthread call.
    // Signature: 48 8B 51 ? 48 8B D9 48 8B 42 ? 48 39 42 48 72 07 B2 ? E8 ? ? ? ? F6 43 01 04 74 0F 4C 8D 43 ? 48 8B D3 48 8B CB E8 ? ? ? ? 48 8B CB E8 ?
    // ? ? ? 48 8B 4B ? 48 8B F8 48 89 01 C7 41 ? ? ? ? ? 48 83 43 08 ? 48 8B 4B ? 48 8B 81 ? ? ? ? 48 85 C0 74 08 48 8B D7 48 8B CB FF D0 48 8B 5C 24 ?
    // 48 8B C7 48 83 C4 ? 5F C3
    static std::uintptr_t rlua_newthread = RebaseAddress(0x143502b10);

    // search for "defer"
    static std::uintptr_t rTask_defer = RebaseAddress(0x1420b9ad0);

    // Get into luaC_step, then into gcstep and then into luaH_free, which inside has the call you need to freestack (or luaM_freearray I don't remember)
    // Which inside calls freeblock.
    // Signature: 4C 8B 51 ? 49 83 E8 ? 44 8B CA 4C 8B D9 49 8B 10 48 83 7A 28 00 75 22 83 7A 30 00 7D 1C 49 63 C1 49 8D 0C C2 49 8B 44 C2 ? 48 89 42 ? 48
    // 85 C0 74 03 48 89 10 48 89 51 ? 48 8B 42 ? 49 89 00 83 6A ? ? 4C 89 42 ? 75 4B 48 8B 4A ? 48 85 C9 74 06 48 8B 02 48 89 01 48 8B 0A 48 85 C9 74 0A
    // 48 8B 42 ? 48 89 41 ? EB 17 41 0F B6 C1 49 39 54 C2 60 49 8D 0C C2 75 08 48 8B 42 ? 48 89 41 ? 49 8B 43 ? 45 33 C9 4C 63 42 ? 48 8B 48 ? 48 FF 60 ?
    static std::uintptr_t rFreeBlock = RebaseAddress(0x1435202a0);

    // Appears to copy ones' L->userdata into another for new states. Search for
    // "Failed to create Lua State", on the userthread user callback.
    // Signature: 74 4A 48 8B CD E8 ? ? ? ? 48 8B F0 B9 ? ? ? ? E8 ? ? ? ? 48 89 44 24 ? 48 85 C0 74 10 48 8B D6 48 8B C8 E8 ? ? ? ? 48 8B F0 EB 02 33 F6 48 8B CD E8 ? ? ? ? 48 89 46 ? 48 8B D6 48 8B CF E8 ? ? ? ?
    static std::uintptr_t rFromLuaState = RebaseAddress(0x141f04f40);

    // Search for luaD_rununprotected, crawl until you reach into luaD_pcall, from which, search for xrefs into a function referencing
    // luaO_nilobject and pseudo2addr, it will also have an if check at the end checking for a negative value (first bit set).
    // Signature: 48 89 4C 24 ? 48 83 EC ? 48 8B C2 49 8B D0 FF D0 33 C0 EB 04 8B 44 24 48 48 83 C4 ? C3
    static std::uintptr_t rLuaD_rawrununprotected = RebaseAddress(0x143509ba0);

    // Follow into lua_newthread, an if statement will lie there checking totalbytes and GCthreshold, that is luaC_needsgc, which is called inside
    // luaC_checkgc, which calls luaC_step! Signature: 48 8B 59 ? B8 ? ? ? ? 0F B6 F2 0F 29 74 24 ? 4C 8B F1 44 8B 43 ? 44 0F AF 43 ? 48 8B 6B ? 48 2B 6B ? 41 F7 E8 8B FA C1 FF ? 8B C7 C1 E8 ? 03 F8 48 8B 83 ? ? ? ? 48 85 C0 74 04 33 D2 FF D0 0F B6 43 ? 84 C0 75 32
    static std::uintptr_t rLuaC_Step = RebaseAddress(0x14351dc40);

    // search for "ProximityPrompt_Triggered".
    static std::uintptr_t fireproximityprompt = RebaseAddress(0x1429028e0);

    // search for "InvalidInstance". Caller with two arguments (Modifies lua stack)
    static std::uintptr_t pushinstance = RebaseAddress(0x141db24c0);

    // Search for "TaskSchedulerTargetFps". Use the .data segment reference it refers towards. It is a DWORD (or int32_t)
    const static std::uintptr_t FFlag_TaskSchedulerTargetFps = RebaseAddress(0x148b6a504);
} // namespace RBX::Studio::Offsets

struct lua_State;

namespace FunctionTypes {
    using luau_execute = void(__fastcall*)(lua_State* L);
    using pseudo2addr = lua_TValue*(__fastcall*)(lua_State* L, int32_t lua_index);
    using rTask_defer = int(__fastcall*)(lua_State* L);
    using rluaE_newthread = lua_State*(__fastcall*)(lua_State* L);
    using rlua_newthread = lua_State*(__fastcall*)(lua_State* L);
    using rFromLuaState = void(__fastcall*)(lua_State* LP, lua_State* L);
    using rFreeBlock = void(__fastcall*)(lua_State* L, int32_t sizeClass, void* block);
    using rLuaD_throw = void(__fastcall*)(lua_State* L, int32_t errcode);
    using rLuaD_rawrununprotected = int32_t(__fastcall*)(lua_State* L, void (*PFunc)(lua_State* L, void* ud), void* ud);
    using rLuaC_Step = size_t(__fastcall*)(lua_State* L, bool assist);
    using fireproximityprompt = void(__fastcall*)(void* proximityPrompt);
    using pushinstance = std::uintptr_t(__fastcall*)(lua_State* L, void* instance);
}; // namespace FunctionTypes

namespace RBX::Studio::Functions {
    const static auto luau_execute = reinterpret_cast<FunctionTypes::luau_execute>(RBX::Studio::Offsets::luau_execute);
    const static auto rTask_defer = reinterpret_cast<FunctionTypes::rTask_defer>(RBX::Studio::Offsets::rTask_defer);
    const static auto rluaE_newthread = reinterpret_cast<FunctionTypes::rluaE_newthread>(RBX::Studio::Offsets::rluaE_newthread);
    const static auto rlua_newthread = reinterpret_cast<FunctionTypes::rlua_newthread>(RBX::Studio::Offsets::rlua_newthread);
    const static auto rFromLuaState = reinterpret_cast<FunctionTypes::rFromLuaState>(RBX::Studio::Offsets::rFromLuaState);
    const static auto rFreeBlock = reinterpret_cast<FunctionTypes::rFreeBlock>(RBX::Studio::Offsets::rFreeBlock);
    const static auto rLuaD_throw = reinterpret_cast<FunctionTypes::rLuaD_throw>(RBX::Studio::Offsets::rLuaD_throw);
    const static auto rLuaD_rawrununprotected = reinterpret_cast<FunctionTypes::rLuaD_rawrununprotected>(RBX::Studio::Offsets::rLuaD_rawrununprotected);
    const static auto rLuaC_Step = reinterpret_cast<FunctionTypes::rLuaC_Step>(RBX::Studio::Offsets::rLuaC_Step);
    const static auto fireproximityprompt = reinterpret_cast<FunctionTypes::fireproximityprompt>(RBX::Studio::Offsets::fireproximityprompt);
    const static auto pushinstance = reinterpret_cast<FunctionTypes::pushinstance>(RBX::Studio::Offsets::pushinstance);
    const static auto pseudo2addr = reinterpret_cast<FunctionTypes::pseudo2addr>(RBX::Studio::Offsets::pseudo2addr);
} // namespace RBX::Studio::Functions

/*
 *  How to get this to compile when updating Luau?
 *      - Modify lobject.cpp and lobject.h to use Studios' luaO_nilobject, same thing with ltable.cpp and ltable.h and luaH_dummynode, as well as
 * modifying lvm.cpp to use luau_execute. You must use luau_load when compiling code, for anyone using this to develop anything.
 */