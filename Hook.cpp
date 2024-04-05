//
// Created by Dottik on 4/4/2024.
//

#include "Hook.hpp"
#include <lstate.h>
#include <mutex>
#include "Scheduler.hpp"

Hook *Hook::g_hookSingleton = nullptr;

std::mutex mutx{};

void *Hook::pseudo2addr__detour(lua_State *L, int idx) {
    mutx.lock();

    auto scheduler{Scheduler::GetSingleton()};
    if (!scheduler->IsInitialized()) {
        printf("Initializing scheduler... \n");
        auto nL = lua_newthread(L);
        lua_ref(L, -1); // Avoid dying.
        lua_pop(L, 1);      // Restore stack.
        scheduler->InitializeWith(nL);
    }

    mutx.unlock();
    return Hook::get_singleton()->get_pseudo_original()(L);
}

lua_State *Hook::lua_newthread__detour(lua_State *L) {
    mutx.lock();

    auto scheduler{Scheduler::GetSingleton()};
    if (!scheduler->IsInitialized()) {
        printf("Initializing scheduler... \n");
        auto nL = lua_newthread(L);
        lua_ref(L, -1); // Avoid dying.
        lua_pop(L, 1);      // Restore stack.
        scheduler->InitializeWith(nL);
    }

    mutx.unlock();
    return Hook::get_singleton()->get_pseudo_original()(L);
}

Hook *Hook::get_singleton() noexcept {
    if (g_hookSingleton == nullptr)
        g_hookSingleton = new Hook();
    return g_hookSingleton;
}
