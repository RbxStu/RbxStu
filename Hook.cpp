//
// Created by Dottik on 4/4/2024.
//

#include "Hook.hpp"
#include <lstate.h>
#include <mutex>
#include "Scheduler.hpp"
#include "Security.hpp"
#include "lualib.h"
#include "cstdlib"
#include "ltable.h"
#include "Utilities.hpp"

Hook *Hook::g_hookSingleton = nullptr;

void Hook::freeblock__detour(lua_State *L, int32_t sizeClass, void *block) {
    if ((std::uintptr_t) block > oxorany(0x00007FF000000000)) {
        wprintf(oxorany(L"\r\n\r\n--- CALL INSTRUMENTATION ::freeblock @ RBX ---\r\n"));
        wprintf(oxorany(L"---     SUSPICIOUS BLOCK ADDRESS CAUGHT    ---\r\n"));
        wprintf(oxorany(L"--- CALL INSTRUMENTATION ::freeblock @ RBX ---\r\n\r\n"));
        wprintf(oxorany(L"lua state   : 0x%p\r\n"), L);
        wprintf(oxorany(L"sizeClass   : %ull\r\n"), sizeClass);
        wprintf(oxorany(L"block       : 0x%p\r\n"), block);
        wprintf(oxorany(L"*(block - 8): 0x%p\r\n"), *((uintptr_t *) ((std::uintptr_t) block - 8)));
        wprintf(oxorany(L"\r\n\r\n--- END CALL INSTRUMENTATION ::freeblock @ RBX ---\r\n\r\n"));

        wprintf(oxorany(L"\r\nIN ORDER TO AVOID A CRASH. THIS CALL HAS BEEN OMITTED DUE TO BLOCK APPEARING TO BE A STACK ADDRESS! POSSIBLE MEMORY LEAK MAY INSUE!\r\n"));

        return;
    }


    return Hook::get_singleton()->get_freeblock_original()(L, sizeClass, block);
}

std::mutex mutx{};

void *Hook::pseudo2addr__detour(lua_State *L, int idx) {
    mutx.lock();
    auto scheduler{Scheduler::get_singleton()};
    if (!scheduler->is_initialized() &&
        rand() % oxorany(64) ==
        0) {  // Randomness for more entropy when getting lua_State*, helped get a valid state faster.
        auto ignoreChecks = false;
        char buf[(0xff)];
        if (GetWindowTextA(GetForegroundWindow(), buf, sizeof(buf))) {
            if (strstr(buf, (oxorany_pchar(L":\\"))) != nullptr &&
                strstr(buf, oxorany_pchar(L"- Roblox Studio")) != nullptr &&
                strstr(buf, (oxorany_pchar(L".rbxl"))) != nullptr) {
                wprintf(oxorany(L"WARNING: You seem to have a local file open. This tool does not support them correctly, and the grabbed state may not be correct!\r\n"));
                ignoreChecks = true;
            }
        }
        auto ud = static_cast<RBX::Lua::ExtraSpace *>(L->userdata);
        //wprintf(oxorany(L"State ExtraSpace:\r\n"));
        //wprintf(oxorany(L"Identity: 0x%p\r\n"), ud->identity);
        //wprintf(oxorany(L"Capabilities: 0x%p\r\n"), ud->capabilities);
        wprintf(oxorany(L"Please select your Roblox Studio window if you are using a local file!\r\n"));
        wprintf(oxorany(L"Attempting to initialize scheduler... \n"));

        if (L->singlestep) {
            printf("Not eligible at all. Singlestep detected.\r\n");
            mutx.unlock();

            return Hook::get_singleton()->get_pseudo_original()(L, idx);
        }
        if (!ignoreChecks) {
            auto originalTop = lua_gettop(L);
            lua_getglobal(L, oxorany_pchar(L"game"));

            if (lua_isnil(L, -1)) {
                wprintf(oxorany(L"No DataModel found, lua_State* ignored.\r\n"));
                mutx.unlock();
                return Hook::get_singleton()->get_pseudo_original()(L, idx);
            }

            lua_getfield(L, oxorany(-1), oxorany_pchar(L"PlaceId"));
            auto placeId = luaL_optnumber(L, oxorany(-1), oxorany(0));
            lua_settop(L, originalTop); // Reset stack.

            if (placeId == oxorany(0)) {
                lua_settop(L, originalTop);
                mutx.unlock();
                return Hook::get_singleton()->get_pseudo_original()(L, idx);
            }
        } else {
            wprintf(oxorany(L"[[Hook]] Checks ignored: You seem to have a local file opened as a place. This is not fully supported, please use a Place uploaded to RBX.\r\n"));
        }
        auto oldTop = lua_gettop(L);
        auto oldObf = static_cast<RBX::Identity>(RBX::Security::to_obfuscated_identity(ud->identity));
        RBX::Security::Bypasses::set_thread_security(L, RBX::Identity::Eight_Seven);
        auto nL = RBX::Studio::Functions::rlua_newthread(L);
        lua_ref(L, -1); // Avoid dying.
        auto rL = RBX::Studio::Functions::rlua_newthread(L);
        nL->gt = luaH_clone(L, L->gt);
        rL->gt = luaH_clone(L, L->global->mainthread->gt);
        lua_ref(L, -1); // Avoid dying.
        lua_pop(L, 2);
        // RBX::Security::Bypasses::SetLuastateCapabilities(L, oldObf);

        RBX::Studio::Functions::rFromLuaState(L, nL);
        RBX::Security::Bypasses::set_thread_security(L, oldObf);

        if (nL->userdata != nullptr) free(nL->userdata);
        auto mem = malloc(oxorany(sizeof(RBX::Lua::ExtraSpace)));    // Pointer replacement.
        nL->userdata = mem;
        memcpy(nL->userdata, L->userdata,
               oxorany(0x98));    // We detach it from the original lua_State* we originate it from, thus our caps still work.
        RBX::Security::Bypasses::set_thread_security(nL, RBX::Identity::Eight_Seven);
        RBX::Security::MarkThread(nL);

        lua_settop(L, oldTop);
        //auto L_userdata = reinterpret_cast<RBX::Lua::ExtraSpace *>(L->userdata);
        //auto nL_userdata = reinterpret_cast<RBX::Lua::ExtraSpace *>(nL->userdata);
        //nL_userdata->sharedExtraSpace = L_userdata->sharedExtraSpace;
        scheduler->initialize_with(nL, rL);
    }

    mutx.unlock();

    return Hook::get_singleton()->get_pseudo_original()(L, idx);
}

Hook *Hook::get_singleton() noexcept {
    if (g_hookSingleton == nullptr)
        g_hookSingleton = new Hook();
    return g_hookSingleton;
}

FunctionTypes::rFreeBlock Hook::get_freeblock_original() {
    return this->__original__freeblock__hook;
}

FunctionTypes::pseudo2addr Hook::get_pseudo_original() {
    return this->__original__pseudo2addr__hook;
}

MH_STATUS Hook::remove_hook() const {
    return MH_DisableHook(reinterpret_cast<void *>(RBX::Studio::Offsets::pseudo2addr));
}

void Hook::wait_until_initialised() {
    auto scheduler{Scheduler::get_singleton()};
    do {
        Sleep(oxorany(88));
    } while (!scheduler->is_initialized());
}

void Hook::initialize() const {
    MH_Initialize();    // init mh.
}

MH_STATUS Hook::install_hook() const {
    MH_CreateHook(reinterpret_cast<void *>(RBX::Studio::Offsets::pseudo2addr), pseudo2addr__detour,
                  reinterpret_cast<void **>(const_cast<void *(**)(lua_State *,
                                                                  int32_t)>(&__original__pseudo2addr__hook)));
    return MH_EnableHook(reinterpret_cast<void *>(RBX::Studio::Offsets::pseudo2addr));
}

MH_STATUS Hook::install_additional_hooks() {
    MH_CreateHook(reinterpret_cast<void *>(RBX::Studio::Offsets::rFreeBlock), freeblock__detour,
                  reinterpret_cast<LPVOID *>(&this->__original__freeblock__hook));
    return MH_EnableHook(reinterpret_cast<void *>(RBX::Studio::Offsets::rFreeBlock));
}
