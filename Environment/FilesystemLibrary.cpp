//
// Created by Dottik on 21/4/2024.
//

#include "FilesystemLibrary.hpp"
#include <lapi.h>
#include <string>

#include "ldebug.h"
#include "lualib.h"

int listfiles(lua_State *L) {
    const char *directory = luaL_checkstring(L, 1);

    if (strstr(directory, "..") != nullptr)
        luaG_runerrorL(L, "Using \'..\' is not permitted for safety reasons.");
}

void FilesystemLibrary::register_environment(lua_State *L) {
    // TODO: Implement pipes/websockets with a custom UI for setting the Workspace correctly.

    static const luaL_Reg libreg[] = {
            {"listfiles", listfiles},
            {nullptr, nullptr},
    };

    lua_newtable(L);
    luaL_register(L, nullptr, libreg);
    lua_setreadonly(L, -1, true);
    lua_pop(L, 1);
}

void FilesystemLibrary::set_workspace_path(const std::string &workspacePath) {
    if (workspacePath.length() == 0) {
        throw std::runtime_error("Your workspace directory may not be that of an empty string.");
    }

    if (workspacePath.find("..")) {
        throw std::runtime_error(
                "You cannot set your workspace path to be any that contains \'..\' in it due to safety concerns.");
    }


    FilesystemLibrary::_workspacePath = workspacePath;
}
