//
// Created by Dottik on 4/4/2024.
//

#include "Hook.hpp"
#include <lstate.h>
#include <mutex>
#include "Scheduler.hpp"
#include "Security.hpp"

Hook *Hook::g_hookSingleton = nullptr;

std::mutex mutx{};

void *Hook::pseudo2addr__detour(lua_State *L, int idx) {
    mutx.lock();

    auto scheduler{Scheduler::GetSingleton()};
    if (!scheduler->IsInitialized()) {
        printf("Initializing scheduler... \n");
        auto nL = RBX::Studio::Functions::rlua_newthread(L);
        RBX::Security::Bypasses::SetLuastateCapabilities(nL);
        auto oldTop = lua_gettop(L);
        printf("Attaching callbacks...\r\n");
        nL->global->cb = L->global->cb;
        printf("Copying UserData pointer...\r\n");
        nL->global->ud = L->global->ud;
        printf("Executing callback...\r\n");
        RBX::Studio::Functions::rFromLuaState(L, nL);
        printf("Restoring stack top...\r\n");
        lua_settop(L, oldTop);
        //auto L_userdata = reinterpret_cast<RBX::Lua::ExtraSpace *>(L->userdata);
        //auto nL_userdata = reinterpret_cast<RBX::Lua::ExtraSpace *>(nL->userdata);
        //nL_userdata->sharedExtraSpace = L_userdata->sharedExtraSpace;
        printf("Referencing thread...\r\n");
        lua_ref(L, -1); // Avoid dying.
        lua_pop(L, 1);      // Restore stack.
        scheduler->InitializeWith(nL);
    }

    mutx.unlock();
    return Hook::get_singleton()->get_pseudo_original()(L, idx);
}

Hook *Hook::get_singleton() noexcept {
    if (g_hookSingleton == nullptr)
        g_hookSingleton = new Hook();
    return g_hookSingleton;
}
