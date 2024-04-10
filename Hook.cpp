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

Hook *Hook::g_hookSingleton = nullptr;

void Hook::freeblock__detour(lua_State *L, int32_t sizeClass, void *block) {
    if ((std::uintptr_t) block > 0x00007FF000000000) {
        printf("\r\n\r\n--- CALL INSTRUMENTATION ::freeblock @ RBX ---\r\n");
        printf("---     SUSPICIOUS BLOCK ADDRESS CAUGHT    ---\r\n");
        printf("--- CALL INSTRUMENTATION ::freeblock @ RBX ---\r\n\r\n");

        printf("lua state   : 0x%p\r\n", L);
        printf("sizeClass   : %ull\r\n", sizeClass);
        printf("block       : 0x%p\r\n", block);
        printf("*(block - 8): 0x%p\r\n", *((uintptr_t *) ((std::uintptr_t) block - 8)));

        printf("\r\n\r\n--- END CALL INSTRUMENTATION ::freeblock @ RBX ---\r\n\r\n");

        printf("\r\nIN ORDER TO AVOID A CRASH. THIS CALL HAS BEEN OMITTED DUE TO BLOCK APPEARING TO BE A STACK ADDRESS! POSSIBLE MEMORY LEAK MAY INSUE!\r\n");

        return;
    }


    return Hook::get_singleton()->get_freeblock_original()(L, sizeClass, block);
}

std::mutex mutx{};

void *Hook::pseudo2addr__detour(lua_State *L, int idx) {
    mutx.lock();
    auto scheduler{Scheduler::GetSingleton()};
    if (!scheduler->IsInitialized() &&
        rand() % 64 == 0) {  // Randomness for more entropy when getting lua_State*, helped get a valid state faster.
        auto ignoreChecks = false;
        char buf[0xff];
        if (GetWindowTextA(GetForegroundWindow(), buf, sizeof(buf))) {
            if (strstr(buf, ":\\") != nullptr && strstr(buf, "- Roblox Studio") != nullptr &&
                strstr(buf, ".rbxl") != nullptr) {
                printf("WARNING: You seem to have a local file open. This tool does not support them correctly, and the grabbed state may not be correct!\r\n");
                ignoreChecks = true;
            }
        }
        auto ud = static_cast<RBX::Lua::ExtraSpace *>(L->userdata);
        printf("State ExtraSpace:\r\n");
        printf("Identity: 0x%p\r\n", ud->identity);
        printf("Capabilities: 0x%p\r\n", ud->capabilities);
        printf("Please select your Roblox Studio window if you are using a local file!\r\n");
        printf("Attempting to initialize scheduler... \n");
        // printf("Evaluating lua_State*'s possibility of being useful...\r\n");

        if (L->singlestep) {
            printf("Not eligible at all. Singlestep detected.\r\n");
            mutx.unlock();

            return Hook::get_singleton()->get_pseudo_original()(L, idx);
        }
        if (!ignoreChecks) {
            auto originalTop = lua_gettop(L);

            lua_getglobal(L, "game");

            if (lua_isnil(L, -1)) {
                printf("No DataModel found, lua_State* ignored.\r\n");
                mutx.unlock();
                return Hook::get_singleton()->get_pseudo_original()(L, idx);
            }

            lua_getfield(L, -1, "PlaceId");
            auto placeId = luaL_optnumber(L, -1, 0);
            lua_settop(L, originalTop); // Reset stack.

            if (placeId == 0) {
                lua_settop(L, originalTop);
                mutx.unlock();
                return Hook::get_singleton()->get_pseudo_original()(L, idx);
            }
        } else {
            printf("[[Hook]] Checks ignored: You seem to have a local file opened as a place. This is not fully supported, please use a Place uploaded to RBX.\r\n");
        }
        auto oldTop = lua_gettop(L);
        auto oldObf = static_cast<RBX::Identity>(RBX::Security::ObfuscateIdentity(ud->identity));
        RBX::Security::Bypasses::SetLuastateCapabilities(L, RBX::Identity::Eight_Seven);
        auto nL = RBX::Studio::Functions::rlua_newthread(L);
        lua_ref(L, -1); // Avoid dying.
        lua_pop(L, 2);
        // RBX::Security::Bypasses::SetLuastateCapabilities(L, oldObf);

        RBX::Studio::Functions::rFromLuaState(L, nL);
        RBX::Security::Bypasses::SetLuastateCapabilities(L, oldObf);

        auto mem = malloc(0x98);    // Pointer replacement.
        nL->userdata = mem;
        memcpy(nL->userdata, L->userdata,
               0x98);    // We detach it from the original lua_State* we originate it from, thus our caps still work.
        RBX::Security::Bypasses::SetLuastateCapabilities(nL, RBX::Identity::Eight_Seven);

        lua_settop(L, oldTop);
        //auto L_userdata = reinterpret_cast<RBX::Lua::ExtraSpace *>(L->userdata);
        //auto nL_userdata = reinterpret_cast<RBX::Lua::ExtraSpace *>(nL->userdata);
        //nL_userdata->sharedExtraSpace = L_userdata->sharedExtraSpace;
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
    auto scheduler{Scheduler::GetSingleton()};
    do {
        Sleep(88);
    } while (!scheduler->IsInitialized());
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
