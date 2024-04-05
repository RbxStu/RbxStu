//
// Created by Dottik on 4/4/2024.
//

#pragma once

#include <MinHook.h>
#include <StudioOffsets.h>
#include "Scheduler.hpp"

class Hook {
    static Hook *g_hookSingleton;

    FunctionTypes::lua_newthread __original__hook;
private:
    static void *pseudo2addr__detour(lua_State *L, int idx);

    static lua_State *lua_newthread__detour(lua_State *L);


public:
    static Hook *get_singleton() noexcept;

    [[nodiscard]] MH_STATUS install_hook() const {
        MH_Initialize();    // init mh.
        /*MH_CreateHook(reinterpret_cast<void *>(RBX::Studio::Offsets::pseudo2addr), pseudo2addr__detour,
                      reinterpret_cast<void **>(const_cast<void *(**)(lua_State *, int32_t)>(&__original__hook)));*/
        MH_CreateHook(reinterpret_cast<void *>(RBX::Studio::Offsets::lua_newthread), lua_newthread__detour,
                      reinterpret_cast<void **>(const_cast<lua_State *(**)(lua_State *)>(&__original__hook)));

        return MH_EnableHook(reinterpret_cast<void *>(RBX::Studio::Offsets::lua_newthread));
    }

    void wait_until_initialised() const {
        auto scheduler{Scheduler::GetSingleton()};
        do {
            Sleep(88);
        } while (!scheduler->IsInitialized());
    }

    [[nodiscard]] MH_STATUS remove_hook() const {
        return MH_DisableHook(reinterpret_cast<void *>(RBX::Studio::Offsets::lua_newthread));
    }

    [[nodiscard]] FunctionTypes::lua_newthread get_pseudo_original() {
        return this->__original__hook;
    }
};

