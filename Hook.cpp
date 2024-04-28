//
// Created by Dottik on 4/4/2024.
//

#include "Hook.hpp"
#include <lstate.h>
#include <mutex>
#include "Closures.hpp"
#include "Execution.hpp"
#include "Scheduler.hpp"
#include "Security.hpp"
#include "Utilities.hpp"
#include "cstdlib"
#include "ltable.h"
#include "lualib.h"

Hook *Hook::g_hookSingleton = nullptr;

void Hook::freeblock__detour(lua_State *L, uint32_t sizeClass, void *block) {
    if (reinterpret_cast<std::uintptr_t>(block) > 0x00007FF000000000) {
        wprintf(L"\r\n\r\n--- CALL INSTRUMENTATION ::freeblock @ RBX ---\r\n");
        wprintf(L"---     SUSPICIOUS BLOCK ADDRESS CAUGHT    ---\r\n");
        wprintf(L"--- CALL INSTRUMENTATION ::freeblock @ RBX ---\r\n\r\n");
        wprintf(L"lua state   : 0x%p\r\n", L);
        wprintf(L"sizeClass   : 0x%lx\r\n", sizeClass);
        wprintf(L"block       : 0x%p\r\n", block);
        wprintf(L"*(block - 8): 0x%p\r\n",
                *reinterpret_cast<uintptr_t **>(reinterpret_cast<std::uintptr_t>(block) - 8));
        wprintf(L"\r\n\r\n--- END CALL INSTRUMENTATION ::freeblock @ RBX ---\r\n\r\n");

        wprintf(L"\r\nIN ORDER TO AVOID A CRASH. THIS CALL HAS BEEN OMITTED DUE TO BLOCK APPEARING TO BE A STACK "
                L"ADDRESS! POSSIBLE MEMORY LEAK MAY INSUE!\r\n");

        return;
    }

    if (!Module::Utilities::is_pointer_valid(static_cast<std::uintptr_t *>(block)) ||
        !Module::Utilities::is_pointer_valid(
                reinterpret_cast<std::uintptr_t **>(reinterpret_cast<std::uintptr_t>(block) - 8)) ||
        !Module::Utilities::is_pointer_valid(*reinterpret_cast<std::uintptr_t **>(
                reinterpret_cast<std::uintptr_t>(block) -
                8))) { // It is actually a lua_Page*, but the type is not exposed on Luaus' lgc.h.
        wprintf(L"\r\n\r\n--- CALL INSTRUMENTATION ::freeblock @ RBX ---\r\n");
        wprintf(L"---     SUSPICIOUS BLOCK ADDRESS CAUGHT    ---\r\n");
        wprintf(L"--- CALL INSTRUMENTATION ::freeblock @ RBX ---\r\n\r\n");
        wprintf(L"lua state   : 0x%p\r\n", L);
        wprintf(L"sizeClass   : 0x%lx\r\n", sizeClass);
        wprintf(L"block       : 0x%p\r\n", block);
        wprintf(L"*(block - 8): 0x%p\r\n",
                *reinterpret_cast<uintptr_t **>(reinterpret_cast<std::uintptr_t>(block) - 8));
        wprintf(L"\r\n\r\n--- END CALL INSTRUMENTATION ::freeblock @ RBX ---\r\n\r\n");

        wprintf(L"\r\nIN ORDER TO AVOID A CRASH. THIS CALL HAS BEEN OMITTED DUE TO BLOCK APPEARING TO BE AN INVALID "
                L"POINTER!"
                L" POSSIBLE MEMORY LEAK MAY INSUE!\r\n");

        return;
    }


    return Hook::get_singleton()->get_freeblock_original()(L, sizeClass, block);
}

std::mutex mutx{};
long long tries = 0;

void *Hook::pseudo2addr__detour(lua_State *L, int idx) {
    mutx.lock();
    auto scheduler{Scheduler::get_singleton()};
    if (!scheduler->is_initialized() && rand() % 0xff == 0) {
        // Randomness for more entropy when getting lua_State*, helped get a valid state faster.
        auto ignoreChecks = false;
        char buf[(0xff)];

        if (tries > 30) {
            wprintf(L"You seem to be having issues trying to obtain a lua_State*, make sure to publish your game, at "
                    L"least privately, else this tool will NOT work!\r\n");
            wprintf(L"Validation checks on the lua_State* will be ignored this time. But please remember to publish "
                    L"privately if you wanna use this tool without being an unstable mess!\r\n");
            ignoreChecks = true;
        }

        if (GetWindowTextA(GetForegroundWindow(), buf, sizeof(buf))) {
            if (strstr(buf, ":\\") != nullptr && strstr(buf, "- Roblox Studio") != nullptr &&
                strstr(buf, ".rbxl") != nullptr) {
                wprintf(L"WARNING: You seem to have a local file open. This tool does not support them correctly, and "
                        L"the grabbed state may not be correct!\r\n");
                ignoreChecks = true;
            }
        }
        auto ud = static_cast<RBX::Lua::ExtraSpace *>(L->userdata);
        wprintf(L"Target Lua State ExtraSpace:\r\n");
        wprintf(L"Identity: 0x%p\r\n", ud->identity);
        wprintf(L"Capabilities: 0x%p\r\n", ud->capabilities);
        wprintf(L"Please select your Roblox Studio window if you are using a local file!\r\n");
        wprintf(L"Attempting to initialize scheduler... \n");

        if (L->singlestep) {
            wprintf(L"Not eligible at all. Singlestep detected.\r\n");
            tries++;
            mutx.unlock();
            return Hook::get_singleton()->get_pseudo_original()(L, idx);
        }
        if (!ignoreChecks) {
            auto originalTop = lua_gettop(L);
            lua_getglobal(L, "game");

            if (lua_isnil(L, -1)) {
                wprintf(L"No DataModel found, lua_State* ignored.\r\n");
                tries++;
                mutx.unlock();
                return Hook::get_singleton()->get_pseudo_original()(L, idx);
            }

            lua_getfield(L, -1, "PlaceId");
            const auto placeId = luaL_optinteger(L, -1, -1);
            lua_settop(L, originalTop); // Reset stack.
            lua_getglobal(L, "game");
            lua_getfield(L, -1, "GameId");
            const auto gameId = luaL_optinteger(L, -1, -1);
            lua_settop(L, originalTop); // Reset stack.


            if (placeId == 0 && gameId == 0) {
                wprintf(L"PlaceId and GameId are 0, lua_State* ignored.\r\n");
                lua_settop(L, originalTop);
                tries++;
                mutx.unlock();
                return Hook::get_singleton()->get_pseudo_original()(L, idx);
            }
            if (placeId == 0) {
                wprintf(L"PlaceId is 0, lua_State* ignored.\r\n");
                lua_settop(L, originalTop);
                tries++;
                mutx.unlock();
                return Hook::get_singleton()->get_pseudo_original()(L, idx);
            }
            if (gameId == 0) {
                wprintf(L"GameId is 0, lua_State* ignored.\r\n");
                lua_settop(L, originalTop);
                tries++;
                mutx.unlock();
                return Hook::get_singleton()->get_pseudo_original()(L, idx);
            }
        } else {
            wprintf(L"[[Hook]] Checks ignored: You seem to have a local file opened as a place. This is not fully "
                    L"supported, please use a Place uploaded to RBX.\r\n");
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
        RBX::Security::Bypasses::reallocate_extraspace(nL);
        RBX::Security::Bypasses::set_thread_security(nL, RBX::Identity::Eight_Seven);
        RBX::Security::MarkThread(nL);

        lua_settop(L, oldTop);
        // auto L_userdata = reinterpret_cast<RBX::Lua::ExtraSpace *>(L->userdata);
        // auto nL_userdata = reinterpret_cast<RBX::Lua::ExtraSpace *>(nL->userdata);
        // nL_userdata->sharedExtraSpace = L_userdata->sharedExtraSpace;
        lua_newtable(L);
        lua_setglobal(nL, "_G");
        scheduler->initialize_with(nL, rL);
        lua_settop(L, oldTop);
    }

    mutx.unlock();

    return Hook::get_singleton()->get_pseudo_original()(L, idx);
}

Hook *Hook::get_singleton() noexcept {
    if (g_hookSingleton == nullptr)
        g_hookSingleton = new Hook();
    return g_hookSingleton;
}

FunctionTypes::rFreeBlock Hook::get_freeblock_original() const { return this->__original__freeblock__hook; }

FunctionTypes::pseudo2addr Hook::get_pseudo_original() const { return this->__original__pseudo2addr__hook; }

MH_STATUS Hook::remove_hook() const {
    return MH_DisableHook(reinterpret_cast<void *>(RBX::Studio::Offsets::pseudo2addr));
}

void Hook::wait_until_initialised() {
    const auto scheduler{Scheduler::get_singleton()};
    do {
        std::this_thread::sleep_for(std::chrono::milliseconds(96));
    } while (!scheduler->is_initialized());
}

void Hook::initialize() {
    MH_Initialize(); // init mh.
}

MH_STATUS Hook::install_hook() {
    MH_CreateHook(
            reinterpret_cast<void *>(RBX::Studio::Offsets::pseudo2addr), pseudo2addr__detour,
            reinterpret_cast<void **>(const_cast<void *(**) (lua_State *, int32_t)>(&__original__pseudo2addr__hook)));
    return MH_EnableHook(reinterpret_cast<void *>(RBX::Studio::Offsets::pseudo2addr));
}

MH_STATUS Hook::install_additional_hooks() {
    MH_CreateHook(reinterpret_cast<void *>(RBX::Studio::Offsets::rFreeBlock), freeblock__detour,
                  reinterpret_cast<LPVOID *>(&this->__original__freeblock__hook));
    return MH_EnableHook(reinterpret_cast<void *>(RBX::Studio::Offsets::rFreeBlock));
}
