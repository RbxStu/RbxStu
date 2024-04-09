//
// Created by Dottik on 25/11/2023.
//
#include <lua.h>
#include <string>
#include <oxorany.hpp>
#include <Utilities.hpp>
#include <Closures.hpp>
#include <lobject.h>
#include <lgc.h>
#include "ClosureLibrary.hpp"
#include "Execution.hpp"


int iscclosure(lua_State * L) {
    auto utilities{Module::Utilities::GetSingleton()};
    if (lua_gettop(L) < 1 || lua_type(L, 1) != LUA_TFUNCTION) {
        luaL_error(L,   // Cursed tower of pain!
                   utilities->ToString(
                           std::wstring((oxorany(L"iscclosure: bad parameter #1. Expected function, got ")) +
                                        utilities->ToWideString(lua_typename(L, 1)))).c_str());
    }

    lua_pushboolean(L, lua_iscfunction(L, 1));
    return 1;
}

int islclosure(lua_State * L) {
    auto utilities{Module::Utilities::GetSingleton()};
    if (lua_gettop(L) < 1 || lua_type(L, 1) != LUA_TFUNCTION) {
        luaL_error(L,   // Cursed tower of pain!
                   utilities->ToString(
                           std::wstring((oxorany(L"islclosure: bad parameter #1. Expected function, got ")) +
                                        utilities->ToWideString(lua_typename(L, 1)))).c_str());
    }
    lua_pushboolean(L, lua_isLfunction(L, 1));
    return 1;
}

int newcclosure(lua_State * L) {
    auto utilities{Module::Utilities::GetSingleton()};
    auto closures{Module::Closures::GetSingleton()};
    if (lua_gettop(L) < 1 || lua_type(L, 1) != LUA_TFUNCTION) {
        luaL_error(L,   // Cursed tower of pain!
                   utilities->ToString(
                           std::wstring((oxorany(L"newcclosure: bad parameter #1. Expected function, got ")) +
                                        utilities->ToWideString(lua_typename(L, 1)))).c_str());
    }

    if (lua_iscfunction(L, 1) == 1) {
        lua_pushvalue(L, 1); // Why do you give me a C closure already?.
        return 1;
    }

    // Use wrapper.
    closures->ToCClosure(L, 1);
    return 1;
}

int newlclosure(lua_State * L) {
    auto utilities{Module::Utilities::GetSingleton()};
    auto closures{Module::Closures::GetSingleton()};
    if (lua_gettop(L) < 1 || lua_type(L, 1) != LUA_TFUNCTION) {
        luaL_error(L,   // Cursed tower of pain!
                   utilities->ToString(
                           std::wstring((oxorany(L"newlclosure: bad parameter #1. Expected function, got ")) +
                                        utilities->ToWideString(lua_typename(L, 1)))).c_str());
    }

    if (lua_isLfunction(L, 1) == 1) {
        lua_pushvalue(L, 1); // Why do you give me an L closure already?.
        return 1;
    }

    // Use wrapper.
    closures->ToLClosure(L, 1);
    return 1;
}

int hookfunction(lua_State * L) {
    // Being honest, I'm lazy to write all the statement for error handling I did before lol.
    luaL_checktype(L, 1, LUA_TFUNCTION);
    luaL_checktype(L, 2, LUA_TFUNCTION);
    auto utilities{Module::Utilities::GetSingleton()};
    auto closures{Module::Closures::GetSingleton()};
    auto *toHook = lua_toclosure(L, 1);
    auto *hookWith = lua_toclosure(L, 2);
    auto toHookUpvalues = toHook->nupvalues;
    auto hookWithUpvalues = hookWith->nupvalues;

    if (toHook->isC) {
        // FIXME: this shit won't hook, i have tried so many things for this C<->X hooking, and finally given up.
        lua_CFunction hook_f = nullptr;
        if (!hookWith->isC) {
            // Convert L to C.
            closures->ToCClosure(L, 2);
            hookWith = lua_toclosure(L, -1);    // Get closure at stack top.
            hook_f = NewCClosureHandler;
            lua_pop(L, 1);
        } else {
            hook_f = hookWith->c.f;
        }

        lua_clonefunction(L, 1);

        toHook->c.f = [](lua_State *L) -> int { return 0; };

        for (int i = 0; i < hookWith->nupvalues; i++) {
            TValue * cl_tval = &hookWith->c.upvals[i];
            TValue * ncl_tval = &toHook->c.upvals[i];

            ncl_tval->value = cl_tval->value;
            ncl_tval->tt = cl_tval->tt;
        }

        toHook->nupvalues = hookWith->nupvalues;
        toHook->c.cont = hookWith->c.cont;
        toHook->c.f = hook_f;

        return 1;
    } else {
        if (hookWith->isC) {
            // Convert C to L.
            closures->ToLClosure(L, 2);
            hookWith = lua_toclosure(L, -1);    // Get closure at stack top.
            lua_pop(L, 1);
        }

        lua_clonefunction(L, 1); // Clone original LClosure

        toHook->env = hookWith->env;
        toHook->stacksize = hookWith->stacksize;
        toHook->preload = hookWith->preload;

        for (int i = 0; i < hookWithUpvalues; i++)
            // Set uvs
                setobj2n (L, &toHook->l.uprefs[i], &hookWith->l.uprefs[i]);

        toHook->nupvalues = hookWith->nupvalues;
        toHook->l.p = hookWith->l.p;

        return 1;
    }


    lua_pushnil(L); // Should never happen, but in the remote case it does... uhmmm, cope!
    return 1;
}

int loadstring(lua_State * L) {
    auto execution{Execution::GetSingleton()};
    auto utilities{Module::Utilities::GetSingleton()};

    const std::string scriptText = lua_tostring(L, 1);
    const char *const chunkName = luaL_optstring(L, 2, utilities->RandomString(16).c_str());

    if (std::string(scriptText).empty() || lua_type(L, 1) != LUA_TSTRING) {
        lua_pushnil(L);
        return 1;
    }

    return execution->lua_loadstring(L, scriptText,
                                     chunkName);    // Return the Execution implementation of the custom luau_loadstring.
}

int cloenfunction(lua_State*L) {
    //TODO: Implement
}

void ClosureLibrary::RegisterEnvironment(lua_State *L) {
    static const luaL_Reg reg[] = {
            {("iscclosure"),     iscclosure},
            {("islclosure"),     islclosure},
            {("newcclosure"),    newcclosure},
            {("newlclosure"),    newlclosure},

            {("hookfunction"),   hookfunction},
            {("hookfunc"),       hookfunction},
            {("replaceclosure"), hookfunction},

            {("loadstring"),     loadstring},
            {("compile"),        loadstring},

            {nullptr,            nullptr},
    };

    lua_pushvalue(L, LUA_GLOBALSINDEX);
    luaL_register(L, nullptr, reg);
    lua_pop(L, 1);
}
