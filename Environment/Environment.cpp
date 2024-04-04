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
    auto scheduler{Scheduler::GetSingleton()};
    lua_State *gL = scheduler->GetGlobalState();
    lua_pushvalue(gL, LUA_GLOBALSINDEX);
    lua_xmove(gL, L, 1);
    return 1;
}

int print(lua_State *L) {
    auto utilities{Module::Utilities::GetSingleton()};
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

int warn(lua_State *L) {
    auto utilities{Module::Utilities::GetSingleton()};
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

int error(lua_State *L) {
    auto utilities{Module::Utilities::GetSingleton()};
    auto argc = lua_gettop(L);

    std::wstringstream strStream;
    strStream << oxorany(L"[ERROR] ");
    for (int i = 0; i < argc - 1; i++) {
        const char *lStr = luaL_tolstring(L, i + 1, nullptr);
        strStream << utilities->ToWideString(lStr).c_str() << oxorany(L" ");
    }
    std::wcerr << termcolor::red << strStream.str() << termcolor::reset << std::endl;

    luaL_error(L, utilities->ToString(strStream.str()).c_str());
    return 0;
}

int getgc(lua_State *L) {
    bool addTables = luaL_optboolean(L, 1, false);

    lua_createtable(L, 128, 0);    // getgc table, prealloc sum space, because yes.

    typedef struct {
        lua_State *pLua;
        bool accessTables;
        int itemsFound;
    } GCOContext;

    auto gcCtx = GCOContext{L, addTables, 0};

#pragma clang diagnostic push
#pragma ide diagnostic ignored "ConstantFunctionResult"
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
#pragma clang diagnostic pop

    return 1;
}

int httpget(lua_State *L) {
    std::string targetUrl = lua_tostring(L, 1);

    if (lua_type(L, 1) != LUA_TSTRING) {
        luaL_error(L, (std::string(
                oxorany_pchar(L"Wrong parameters given to httpget. Expected string at parameter #1, got ")) +
                       lua_typename(L, lua_type(L, 1))).c_str());
    }

    if (targetUrl.find(oxorany_pchar(L"http://")) != 0 && targetUrl.find(oxorany_pchar(L"https://")) != 0) {
        luaL_error(L, oxorany_pchar(L"Invalid protocol (expected 'http://' or 'https://')"));
    }

    auto response = cpr::Get(
            cpr::Url{targetUrl},
            cpr::Header{{oxorany_pchar(L"User-Agent"), oxorany_pchar(L"Luau/WinInet")}}
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

int Environment::Register(lua_State *L, bool useInitScript) {
    static const luaL_Reg reg[] = {
            {("getreg"),  getreg},
            {("getgc"),   getgc},
            {("getgenv"), getgenv},
            {("print"),   print},
            {("warn"),    warn},
            {("error"),   error},
            {("HttpGet"), httpget},
            {nullptr,     nullptr},
    };

    lua_pushvalue(L, LUA_GLOBALSINDEX);
    luaL_register(L, nullptr, reg);
    lua_pop(L, 1);

    auto closuresLibrary = ClosureLibrary{};
    std::cout << "Registering Closure Library" << std::endl;
    closuresLibrary.RegisterEnvironment(L);

    auto debugLibrary = DebugLibrary{};
    std::cout << "Registering Debug Library" << std::endl;
    debugLibrary.RegisterEnvironment(L);

    if (useInitScript) {
        std::cout << "Running init script..." << std::endl;
        auto execution{Execution::GetSingleton()};
        auto utilities{Module::Utilities::GetSingleton()};

        // Add init script, (someday!!!)
        std::string str = {"print\"init loaded\""};
        execution->lua_loadstring(L, str, utilities->RandomString(32));
        lua_pcall(L, 0, 0, 0);
        std::cout << "Init script executed." << std::endl;
    }

    Sleep(1000);
    return 0;
}
