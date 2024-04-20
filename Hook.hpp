//
// Created by Dottik on 4/4/2024.
//

#pragma once

#include <MinHook.h>
#include <StudioOffsets.h>
#include "Scheduler.hpp"

class Hook {
    static Hook *g_hookSingleton;

    FunctionTypes::pseudo2addr __original__pseudo2addr__hook;
    FunctionTypes::rFreeBlock __original__freeblock__hook;

    static void *pseudo2addr__detour(lua_State *L, int idx);

    static void freeblock__detour(lua_State *L, int32_t sizeClass, void *block);

public:
    static Hook *get_singleton() noexcept;


    MH_STATUS install_additional_hooks();

    void initialize();

    MH_STATUS install_hook();

    static void wait_until_initialised();

    [[nodiscard]] MH_STATUS remove_hook() const;

    [[nodiscard]] FunctionTypes::rFreeBlock get_freeblock_original() const;

    [[nodiscard]] FunctionTypes::pseudo2addr get_pseudo_original() const;
};
