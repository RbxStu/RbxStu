//
// Created by Dottik on 25/11/2023.
//

#include <iostream>
#include "DebugLibrary.hpp"
#include "lapi.h"
#include "lua.h"
#include "lualib.h"
#include "lapi.h"
#include "Utilities.hpp"

int debug_getconstants(lua_State *L) {
    luaL_checkstack(L, 1, oxorany_pchar(L"getconstants requires one argument when being called."));

    if (!lua_isfunction(L, 1) && !lua_isnumber(L, 1)) {
        luaL_typeerror(L, 1, oxorany_pchar(L"Expected function or number for argument #1"));
    }

    if (lua_isnumber(L, 1)) {
        lua_Debug dbgInfo{};

        int num = lua_tonumber(L, 1);

        if (num < 0) {
            luaL_error(L, oxorany_pchar(L"Invalid level"));
        }

        if (!lua_getinfo(L, num, "f", &dbgInfo)) {
            luaL_error(L, oxorany_pchar(L"Failed to get information"));
        }

        if (lua_iscfunction(L, -1)) {
            luaL_argerror(L, 1, oxorany_pchar(L"stack points to a C closure, Lua function expected."));
        }
    } else {
        lua_pushvalue(L, 1); // Push copy of stack item.

        if (lua_iscfunction(L, 1)) {
            luaL_argerror(L, 1, oxorany_pchar(L"Cannot get constants from a C closure."));
        }
    }
    const auto *pClosure = lua_toclosure(L, -1);
    auto constCount = pClosure->l.p->sizek;
    auto consts = pClosure->l.p->k;


    lua_newtable(L);

    for (int i = 0; i < constCount; i++) {
        TValue *tval = &(consts[i]);

        if (tval->tt == LUA_TFUNCTION) {
            L->top->tt = LUA_TNIL;
            L->top++;
        } else {
            L->top->value = tval->value;
            L->top->tt = tval->tt;
            L->top++;
        }

        lua_rawseti(L, -2, (i + 1));
    }

    return 1;
}

int debug_getconstant(lua_State *L) {
    luaL_checkstack(L, 2, oxorany_pchar(L"getconstant requires two arguments when being called."));

    if (!lua_isfunction(L, 1) && !lua_isnumber(L, 1)) {
        luaL_typeerror(L, 1, oxorany_pchar(L"Expected function or number for argument #1"));
    }

    const int dbgIndx = luaL_checkinteger(L, 2);

    if (lua_isnumber(L, 1)) {
        lua_Debug dbgInfo{};

        int num = lua_tonumber(L, 1);

        if (num < 0) {
            luaL_error(L, oxorany_pchar(L"Invalid level"));
        }

        if (!lua_getinfo(L, num, "f", &dbgInfo)) {
            luaL_error(L, oxorany_pchar(L"Failed to get information"));
        }

        if (lua_iscfunction(L, -1)) {
            luaL_argerror(L, 1, oxorany_pchar(L"stack points to a C closure, Lua function expected."));
        }
    } else {
        lua_pushvalue(L, 1); // Push copy of stack item.

        if (lua_iscfunction(L, -1)) {
            luaL_argerror(L, 1, oxorany_pchar(L"Lua function expected."));
        }
    }
    auto *pClosure = lua_toclosure(L, -1);
    auto consts = pClosure->l.p->k;

    if (!dbgIndx) {
        luaL_argerror(L, 2, oxorany_pchar(L"constant index starts at 1"));
        return 0;
    }

    if (dbgIndx > pClosure->l.p->sizek) {
        luaL_argerror(L, 2, oxorany_pchar(L"constant index is out of range"));
        return 0;
    }

    auto tValue = &consts[dbgIndx - 1];

    if (tValue->tt == LUA_TFUNCTION) {
        lua_pushnil(L);
    } else {
        // Easier to manipulate the top like this than with fucking functions, im not switching on everything LOL
        L->top->tt = tValue->tt;
        L->top->value = tValue->value;
        L->top++;
    }

    return 1;
}

int debug_getinfo(lua_State *L) {
    luaL_checkstack(L, 1, oxorany_pchar(L"getinfo requires one argument when being called."));
    auto infoLevel = 0;

    if (lua_isnumber(L, 1)) {
        infoLevel = lua_tointeger(L, 1);
        luaL_argcheck(L, infoLevel >= 0, 1, oxorany_pchar(L"level cannot be negative"));
    } else if (lua_isfunction(L, 1)) {
        infoLevel = -lua_gettop(L);
    } else {
        luaL_argerror(L, 1, oxorany_pchar(L"function or level expected"));
    }

    lua_Debug lDbg{};

    if (!lua_getinfo(L, infoLevel, oxorany_pchar(L"fulasn"), &lDbg))
        luaL_argerror(L, 1, oxorany_pchar(L"invalid level"));

    lua_newtable(L);

    lua_pushstring(L, lDbg.source);
    lua_setfield(L, -2, oxorany_pchar(L"source"));

    lua_pushstring(L, lDbg.short_src);
    lua_setfield(L, -2, oxorany_pchar(L"short_src"));

    lua_pushvalue(L, 1);
    lua_setfield(L, -2, oxorany_pchar(L"func"));

    lua_pushstring(L, lDbg.what);
    lua_setfield(L, -2, oxorany_pchar(L"what"));

    lua_pushinteger(L, lDbg.currentline);
    lua_setfield(L, -2, oxorany_pchar(L"currentline"));

    lua_pushstring(L, lDbg.name);
    lua_setfield(L, -2, oxorany_pchar(L"name"));

    lua_pushinteger(L, lDbg.nupvals);
    lua_setfield(L, -2, oxorany_pchar(L"nups"));

    lua_pushinteger(L, lDbg.nparams);
    lua_setfield(L, -2, oxorany_pchar(L"numparams"));

    lua_pushinteger(L, lDbg.isvararg);
    lua_setfield(L, -2, oxorany_pchar(L"is_vararg"));

    return 1;
}

void DebugLibrary::RegisterEnvironment(lua_State *L) {
    static const luaL_Reg reg[] = {
            {("getconstant"),  debug_getconstant},
            {("getconstants"), debug_getconstants},
            {("getinfo"),      debug_getinfo},
            {nullptr,          nullptr},
    };

    lua_pushvalue(L, LUA_GLOBALSINDEX);
    luaL_register(L, nullptr, reg);
    lua_pop(L, 1);

    lua_newtable(L);
    luaL_register(L, nullptr, reg);
    lua_setreadonly(L, -1, true);
    lua_setfield(L, LUA_GLOBALSINDEX, "debug");

    lua_pop(L, 1);
}
