//
// Created by Dottik on 14/11/2023.
//

#pragma once

// Lua
#include "Dependencies/Luau/VM/src/lapi.h"
#include "Dependencies/Luau/VM/include/lua.h"
#include "Dependencies/Luau/VM/include/lualib.h"

// C++
#include <string>
#include <cstdint>


class Environment {
private:
    static Environment *singleton;
public:
    static Environment *GetSingleton();

    int Register(lua_State *L, bool useInitScript);
};


