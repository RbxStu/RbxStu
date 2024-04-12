//
// Created by Dottik on 14/11/2023.
//
#include <iostream>
#include "Execution.hpp"
#include "Environment.hpp"
#include "Dependencies/Termcolor.hpp"
#include "Dependencies/cpr/include/cpr/cpr.h"
#include "Dependencies/HttpStatus.hpp"
#include "Utilities.hpp"
#include "oxorany.hpp"
#include "Closures.hpp"

#include "Dependencies/Luau/VM/src/lmem.h"
#include "Dependencies/Luau/VM/src/lapi.h"
#include "Dependencies/Luau/VM/src/lvm.h"
#include "Dependencies/Luau/VM/include/lua.h"
#include "Dependencies/Luau/VM/src/lgc.h"
#include "ClosureLibrary.hpp"
#include "DebugLibrary.hpp"
#include "Scheduler.hpp"
#include "Hook.hpp"
#include "Security.hpp"

Environment *Environment::singleton = nullptr;

Environment *Environment::GetSingleton() {
    if (singleton == nullptr)
        singleton = new Environment();
    return singleton;
}

int getreg(lua_State *L) {
    lua_pushvalue(L, LUA_REGISTRYINDEX);
    return 1;
}

int getgenv(lua_State *L) {
    auto scheduler{Scheduler::get_singleton()};
    lua_State *gL = scheduler->get_global_executor_state();
    lua_pushvalue(gL, LUA_GLOBALSINDEX);
    lua_xmove(gL, L, 1);
    return 1;
}

int getrenv(lua_State *L) {
    auto scheduler{Scheduler::get_singleton()};

    lua_State *gL = scheduler->get_global_roblox_state();
    lua_pushvalue(gL, LUA_GLOBALSINDEX);
    lua_xmove(gL, L, 1);
    return 1;
}

int consoleprint(lua_State *L) {
    auto utilities{Module::Utilities::get_singleton()};
    auto argc = lua_gettop(L);
    std::wstringstream strStream;
    strStream << oxorany(L"[INFO] ");
    for (int i = 0; i <= argc - 1; i++) {
        const char *lStr = luaL_tolstring(L, i + 1, nullptr);
        strStream << utilities->ToWideString(lStr).c_str() << oxorany(L" ");
    }
    std::wcout << termcolor::green << strStream.str() << termcolor::reset << std::endl;

    return 0;
}

int consolewarn(lua_State *L) {
    auto utilities{Module::Utilities::get_singleton()};
    auto argc = lua_gettop(L);
    std::wstringstream strStream;
    strStream << oxorany(L"[WARN] ");
    for (int i = 0; i <= argc - 1; i++) {
        const char *lStr = luaL_tolstring(L, i + 1, nullptr);
        strStream << utilities->ToWideString(lStr).c_str() << oxorany(L" ");
    }
    std::wcout << termcolor::yellow << strStream.str() << termcolor::reset << std::endl;

    return 0;
}

int consoleerror(lua_State *L) {
    auto utilities{Module::Utilities::get_singleton()};
    auto argc = lua_gettop(L);

    std::wstringstream strStream;
    strStream << oxorany(L"[ERROR] ");
    for (int i = 0; i < argc - 1; i++) {
        const char *lStr = luaL_tolstring(L, i + 1, nullptr);
        strStream << utilities->ToWideString(lStr).c_str() << oxorany(L" ");
    }
    std::wcerr << termcolor::red << strStream.str() << termcolor::reset << std::endl;

    luaG_runerror(L, utilities->ToString(strStream.str()).c_str());
    return 0;
}

int getgc(lua_State *L) {
    bool addTables = luaL_optboolean(L, 1, false);

    lua_createtable(L, 0, 0);    // getgc table, prealloc sum space, because yes.

    typedef struct {
        lua_State *pLua;
        bool accessTables;
        int itemsFound;
    } GCOContext;

    auto gcCtx = GCOContext{L, addTables, 0};

#pragma clang diagnostic push
#pragma ide diagnostic ignored "ConstantFunctionResult"
    auto oldThreshold = L->global->GCthreshold;
    L->global->GCthreshold = SIZE_MAX;
    // Never return true. We aren't deleting shit.
    luaM_visitgco(L, &gcCtx, [](void *ctx, lua_Page *pPage,
                                GCObject *pGcObj) -> bool {

        auto gcCtx = reinterpret_cast<GCOContext *>(ctx);
        auto L = gcCtx->pLua;

        if (iswhite(pGcObj))
            return false;   // The object is being collected/checked. Skip it.

        auto gcObjType = pGcObj->gch.tt;

        if (gcObjType == LUA_TFUNCTION || gcObjType == LUA_TUSERDATA ||
            gcObjType == LUA_TTABLE && gcCtx->accessTables) {
            // Push copy to top of stack.
            L->top->value.gc = pGcObj;
            L->top->tt = gcObjType;
            L->top++;

            // Store onto GC table.
            auto tIndx = gcCtx->itemsFound++;
            lua_rawseti(L, -2, tIndx + 1);
        }
        return false;
    });
    L->global->GCthreshold = oldThreshold;
#pragma clang diagnostic pop

    return 1;
}

int httpget(lua_State *L) {
    std::string targetUrl = lua_tostring(L, 1);

    if (lua_type(L, 1) != LUA_TSTRING) {
        luaG_runerror(L, (std::string(
                oxorany_pchar(L"Wrong parameters given to httpget. Expected string at parameter #1, got ")) +
                          lua_typename(L, lua_type(L, 1))).c_str());
    }

    if (targetUrl.find(oxorany_pchar(L"http://")) != 0 && targetUrl.find(oxorany_pchar(L"https://")) != 0) {
        luaG_runerror(L, oxorany_pchar(L"Invalid protocol (expected 'http://' or 'https://')"));
    }

    auto response = cpr::Get(
            cpr::Url{targetUrl},
            cpr::Header{{oxorany_pchar(L"User-Agent"), oxorany_pchar(L"Roblox/WinInet")}}
    );

    if (HttpStatus::IsError(response.status_code)) {
        std::string Output = std::format(("Response: %s - %s\n"), std::to_string(response.status_code),
                                         HttpStatus::ReasonPhrase(response.status_code));

        lua_pushstring(L, Output.c_str());
        return 1;
    }

    lua_pushstring(L, response.text.c_str());
    return 1;
}

int checkcaller(lua_State *L) {
    auto *extraSpace = static_cast<RBX::Lua::ExtraSpace *>(L->userdata);

    // We must include a better checkcaller, at least for the future, this gayass check is killing me so bad.
    lua_pushboolean(L, extraSpace != nullptr && ((extraSpace->identity >= 4 && extraSpace->identity <=
                                                                               9)));   // Check identity and the globalActorState (which we messed with to mark our thread)
    return 1;
}

int getidentity(lua_State *L) {
    auto *extraSpace = static_cast<RBX::Lua::ExtraSpace *>(L->userdata);

    //wprintf(oxorany(L"Current State ExtraSpace:\r\n"));
    //wprintf(oxorany(L"  Identity  : 0x%p\r\n"), extraSpace->identity);
    //wprintf(oxorany(L"Capabilities: 0x%p\r\n"), extraSpace->capabilities);
    lua_pushnumber(L, extraSpace->identity);
    return 1;
}

int setidentity(lua_State *L) {
    auto *extraSpace = static_cast<RBX::Lua::ExtraSpace *>(L->userdata);

    auto newIdentity = luaL_optinteger(L, -1, oxorany(8));

    if (newIdentity > oxorany(9) || newIdentity < oxorany(0)) {
        luaG_runerrorL(L, oxorany_pchar("You may not set your identity below 0 or above 9."));
    }

    // The identity seems to be consulted on the L->mainthread. Due to this, we may need to keep two lua states running. One originating from an elevated one, and one originated from a non elevated one.
    // With this we can bypass identity, while still being able to use functions like require, which are literally a REQUIREment. Heh.

    //wprintf(oxorany(L"Old State ExtraSpace:\r\n"));
    //wprintf(oxorany(L"  Identity  : 0x%p\r\n"), extraSpace->identity);
    //wprintf(oxorany(L"Capabilities: 0x%p\r\n"), extraSpace->capabilities);

    extraSpace->identity = newIdentity;     // Identity bypass.
    extraSpace->capabilities = oxorany(0x3FFFF00) | RBX::Security::to_obfuscated_identity(newIdentity);

    //wprintf(oxorany(L"New State ExtraSpace:\r\n"));
    //wprintf(oxorany(L"  Identity  : 0x%p\r\n"), extraSpace->identity);
    //wprintf(oxorany(L"Capabilities: 0x%p\r\n"), extraSpace->capabilities);

    return 0;
}

int getrawmetatable(lua_State *L) {
    if (!lua_getmetatable(L, 1))
        lua_pushnil(L);

    return 1;
}

int setrawmetatable(lua_State *L) {
    luaL_checktype(L, 2, LUA_TTABLE);
    lua_setmetatable(L, 1);
    return 0;
}

int setreadonly(lua_State *L) {
    luaL_argexpected(L, lua_istable(L, 1), 1, oxorany_pchar(L"table"));
    luaL_argexpected(L, lua_isboolean(L, 2), 2, oxorany_pchar(L"boolean"));
    lua_setreadonly(L, 1, lua_toboolean(L, 2));
    return 1;
}

int isreadonly(lua_State *L) {
    luaL_argexpected(L, lua_istable(L, 1), 1, oxorany_pchar(L"table"));
    lua_pushboolean(L, lua_getreadonly(L, 1));
    return 1;
}

int make_writeable(lua_State *L) {
    lua_pushboolean(L, false);
    return setreadonly(L);
}

int make_readonly(lua_State *L) {
    lua_pushboolean(L, true);
    return setreadonly(L);
}

int getnamecallmethod(lua_State *L) {
    const char *namecall_method = lua_namecallatom(L, nullptr);
    if (namecall_method == nullptr) {
        lua_pushnil(L);
    }
    lua_pushstring(L, namecall_method);
    return 1;
}

int identifyexecutor(lua_State *L) {
    lua_pushstring(L, oxorany_pchar("RbxStu"));
    lua_pushstring(L, oxorany_pchar("1.0.0"));
    return 2;
}

int Environment::Register(lua_State *L, bool useInitScript) {
    static const luaL_Reg reg[] = {
            {oxorany_pchar(L"getreg"),            getreg},
            {oxorany_pchar(L"getgc"),             getgc},
            {oxorany_pchar(L"getgenv"),           getgenv},
            {oxorany_pchar(L"getrenv"),           getrenv},
            {oxorany_pchar(L"checkcaller"),       checkcaller},
            {oxorany_pchar(L"setidentity"),       setidentity},
            {oxorany_pchar(L"getidentity"),       getidentity},
            {oxorany_pchar(L"getrawmetatable"),   getrawmetatable},
            {oxorany_pchar(L"setrawmetatable"),   setrawmetatable},
            {oxorany_pchar(L"setreadonly"),       setreadonly},
            {oxorany_pchar(L"isreadonly"),        isreadonly},
            {oxorany_pchar(L"make_writeable"),    make_writeable},
            {oxorany_pchar(L"make_readonly"),     make_readonly},
            {oxorany_pchar(L"getnamecallmethod"), getnamecallmethod},

            {oxorany_pchar(L"identifyexecutor"),  identifyexecutor},
            {oxorany_pchar(L"getexecutorname"),   identifyexecutor},


            {oxorany_pchar("consoleprint"),       consoleprint},
            {oxorany_pchar("consolewarn"),        consolewarn},
            {oxorany_pchar("consoleerror"),       consoleerror},

            {oxorany_pchar(L"HttpGet"),           httpget},

            // {oxorany_pchar(L"reinit"),            reinit},

            {nullptr,                             nullptr},
    };

    lua_pushvalue(L, oxorany(LUA_GLOBALSINDEX));
    luaL_register(L, nullptr, reg);
    lua_pop(L, 1);

    auto closuresLibrary = ClosureLibrary{};
    std::cout << oxorany_pchar(L"Registering Closure Library") << std::endl;
    closuresLibrary.RegisterEnvironment(L);

    auto debugLibrary = DebugLibrary{};
    std::cout << oxorany_pchar(L"Registering Debug Library") << std::endl;
    debugLibrary.RegisterEnvironment(L);

    if (useInitScript) {
        std::cout << oxorany_pchar(L"Running init script...") << std::endl;
        auto execution{Execution::GetSingleton()};
        auto utilities{Module::Utilities::get_singleton()};

        // Add init script, (someday!!!)
        std::string str = oxorany_pchar(LR"(
            local getgenv_c = clonefunction(getgenv)
            local getIdentity_c = clonefunction(getidentity)
            local setIdentity_c = clonefunction(setidentity)
            local rRequire = clonefunction(require)
            local hookfunction_c = clonefunction(hookfunction)
            local newcclosure_c = clonefunction(newcclosure)

            getgenv_c().GetObjects = newcclosure_c(function(assetId)
                local oldId = getIdentity_c()
                setIdentity_c(8)
                local obj = game.GetService(game, "InsertService").LoadLocalAsset((game.GetService(game, "InsertService")), assetId)
                setIdentity_c(oldId)
                return obj
            end)

            --[[
                getgenv_c().hookmetamethod = newcclosure_c(function(t, metamethod, replaceWith)
                    local mt = getrawmetatable(t)
                    if not mt[t] then error("Cannot find metamethod " .. metamethod .. " on metatable.") end
                    return hookfunction_c(mt[t], replaceWith)
                end)
            ]]
            getgenv_c().require = newcclosure_c(function(moduleScript)
                local old = getIdentity_c()
                setIdentity_c(2)
                local r = rRequire(moduleScript)
                setIdentity_c(old)
            end)

            getgenv_c().getnilinstances = newcclosure_c(function()
                local Instances = {}

                for _,Object in getreg() do
                    if typeof(Object) == "Instance" and Object.Parent == nil then
                      table.insert(Instances, Object)
                    end
                end

                return Instances
            end)

            getgenv_c().getinstances = newcclosure_c(function()
                local Instances = {}

                for _,Object in getreg() do
                    if typeof(Object) == "Instance" then
                      table.insert(Instances, Object)
                    end
                end

                return Instances
            end)
        )");
        execution->lua_loadstring(L, str, utilities->RandomString(32),
                                  static_cast<RBX::Identity>(RBX::Security::to_obfuscated_identity(
                                          static_cast<RBX::Lua::ExtraSpace *>(L->userdata)->identity)));
        lua_pcall(L, 0, 0, 0);
        // RBX::Studio::Functions::rTask_defer(L);
        std::cout << oxorany_pchar(L"Init script executed.") << std::endl;
    }

    return 0;
}
