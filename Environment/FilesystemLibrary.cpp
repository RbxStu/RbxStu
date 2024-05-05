//
// Created by nhisoka on 5/5/2024.
//

#include "FilesystemLibrary.hpp"

#include <Execution.hpp>
#include <Utilities.hpp>
#include <fstream>
#include <lapi.h>
#include <string>

#include <thread>
#include <filesystem>

#include "ldebug.h"
#include "lualib.h"

#include <shlwapi.h>
#include <Scheduler.hpp>


std::vector<std::string> disallowed_extensions = {
    ".exe", ".bat",  ".com", ".cmd",  ".inf", ".ipa",
".apk", ".apkm", ".osx", ".pif",  ".run", ".wsh",
".bin", ".app",  ".vb",  ".vbs",  ".scr", ".fap",
".cpl", ".inf1", ".ins", ".inx",  ".isu", ".job",
".lnk", ".msi",  ".ps1", ".reg",  ".vbe", ".js",
".x86", ".pif",  ".xlm", ".scpt", ".out", ".ba_",
".jar", ".ahk",  ".xbe", ".0xe",  ".u3p", ".bms",
".jse", ".cpl",  ".ex",  ".osx",  ".rar", ".zip",
".7z",  ".py",   ".cpp", ".cs",   ".prx", ".tar",
    ".",  ".wim", ".htm",  ".html", ".css",
".appimage", ".applescript", ".x86_64", ".x64_64",
".autorun", ".tmp", ".sys", ".dat", ".ini", ".pol",
".vbscript", ".gadget", ".workflow", ".script",
".action", ".command", ".arscript", ".psc1"
};

const auto pUtilities{Module::Utilities::get_singleton()};

int listfiles(lua_State *L) {
    luaL_checkstring(L, 1);

    pUtilities->create_workspace();

    std::string path = lua_tostring(L, 1);

    if (path.find("..") != std::string::npos)
        luaG_runerrorL(L, "Using \'..\' is not permitted for safety reasons.");

    if(!path.empty()) {
        path = (pUtilities->location + "\\workspace\\" + path);
    } else {
        path = (pUtilities->location + "\\workspace");
    }

    if (!std::filesystem::is_directory(path.c_str())) {
        luaG_runerrorL(L, "directory does not exist");
        return 0;
    }

    lua_newtable(L);

    int idx = 0;
    for (const auto& entry : std::filesystem::directory_iterator(path))
    {
        auto file_path = entry.path().string();
        file_path = pUtilities->replace(file_path, (pUtilities->location + "\\workspace\\"), "");

        lua_pushinteger(L, ++idx);
        lua_pushstring(L, file_path.c_str());
        lua_settable(L, -3);
    }

    return 1;
}

int writefile(lua_State *L) {
    luaL_checktype(L, 1, LUA_TSTRING);
    luaL_checktype(L, 2, LUA_TSTRING);
    std::string path = lua_tostring(L, 1);
    std::string content = lua_tostring(L, 2);

    pUtilities->create_workspace();

    std::string extention = PathFindExtensionA(path.c_str());
    if (extention.empty()) {
        path += ".txt";
    } else {
        for (std::string& test : disallowed_extensions) {
            if (pUtilities->equals_ignore_case(PathFindExtensionA(path.c_str()), test)) {
                luaG_runerrorL(L, "attempt to escape directory");
                return 0;
            }
        }
    }

    if (path.find("..") != std::string::npos) {
        luaG_runerrorL(L, "attempt to escape directory");
        return 0;
    }

    path = (pUtilities->location + "\\workspace\\" + path);

    std::ofstream out(path);
    out << content;
    out.close();

    return 0;
}

int makefolder(lua_State *L) {
    luaL_checktype(L, 1, LUA_TSTRING);

    std::string path = lua_tostring(L, 1);

    pUtilities->create_workspace();

    if (path.find("..") != std::string::npos) {
        luaG_runerror(L, "attempt to escape directory");
        return 0;
    }

    path = (pUtilities->location + "\\workspace\\" + path);

    if(std::filesystem::is_directory(path)) {
        luaG_runerror(L, "Directory already exists");
    }

    std::filesystem::create_directories(path);

    return 0;
}

int appendfile(lua_State* L) {
    luaL_checktype(L, 1, LUA_TSTRING);
    luaL_checktype(L, 2, LUA_TSTRING);

    std::string path = lua_tostring(L, 1);
    std::string content = lua_tostring(L, 2);

    const std::string extention = PathFindExtensionA(path.c_str());
    for (std::string& test : disallowed_extensions) {
        if (pUtilities->equals_ignore_case(extention, test)) {
            luaG_runerror(L, "attempt to escape directory");
            return 0;
        }
    }

    if (path.find("..") != std::string::npos) {
        luaG_runerror(L, "attempt to escape directory");
        return 0;
    }

    path = (pUtilities->location + "\\workspace\\" + path);

    std::ofstream out;
    out.open(path, std::ios_base::app | std::ios_base::binary);
    out.write(content.c_str(), content.size());
    out.close();

    return 0;
}

int readfile(lua_State *L) {

    luaL_checktype(L, 1, LUA_TSTRING);
    std::string path = lua_tostring(L, 1);

    pUtilities->create_workspace();


    if (path.find("..") != std::string::npos) {
        luaG_runerrorL(L, "attempt to escape directory");
        return 0;
    }

    path = (pUtilities->location + "\\workspace\\" + path);

    if (!std::filesystem::exists(path.c_str())) {
        //std::cout << "file does not exist!\n";
        luaG_runerrorL(L, "file does not exist");
        return 0;
    }

    std::string output = pUtilities->read_file(path);
    lua_pushstring(L, output.c_str());
    return 1;
}

int isfolder(lua_State* L)  {
    luaL_checktype(L, 1, LUA_TSTRING);
    std::string path = lua_tostring(L, 1);

    pUtilities->create_workspace();

    if (path.find("..") != std::string::npos) {
        luaG_runerrorL(L, "attempt to escape directory");
        return 0;
    }

    path = (pUtilities->location + "\\workspace\\" + path);

    lua_pushboolean(L, std::filesystem::is_directory(path));
    return 1;
}

int isfile(lua_State* L) {
    luaL_checktype(L, 1, LUA_TSTRING);
    std::string path = lua_tostring(L, 1);

    pUtilities->create_workspace();

    if (path.find("..") != std::string::npos) {
        luaG_runerrorL(L, "attempt to escape directory");
        return 0;
    }

    path = (pUtilities->location + "\\workspace\\" + path);

    lua_pushboolean(L, std::filesystem::is_regular_file(path));
    return 1;
}

int delfolder(lua_State* L) {
    luaL_checktype(L, 1, LUA_TSTRING);
    std::string path = lua_tostring(L, 1);

    pUtilities->create_workspace();

    if (path.find("..") != std::string::npos) {
        luaG_runerrorL(L, "attempt to escape directory");
        return 0;
    }

    path = (pUtilities->location + "\\workspace\\" + path);

    if (!std::filesystem::remove_all(path)) {
        luaG_runerrorL(L, "folder does not exist");
        return 0;
    }

    return 0;
}

int delfile(lua_State* L) {
    luaL_checktype(L, 1, LUA_TSTRING);
    std::string path = lua_tostring(L, 1);

    pUtilities->create_workspace();

    if (path.find("..") != std::string::npos) {
        luaG_runerrorL(L, "attempt to escape directory");
        return 0;
    }

    path = (pUtilities->location + "\\workspace\\" + path);

    if (!std::filesystem::remove(path)) {
        luaG_runerrorL(L, "file does not exist");
        return 0;
    }

    return 0;
}

int dofile(lua_State* L) {
    luaL_checktype(L, 1, LUA_TSTRING);
    std::string path = lua_tostring(L, 1);

    pUtilities->create_workspace();

    if (path.find("..") != std::string::npos) {
        luaG_runerrorL(L, "attempt to escape directory");
        return 0;
    }

    path = (pUtilities->location + "\\workspace\\" + path);

    if (!std::filesystem::exists(path.c_str())) {
        luaG_runerrorL(L, "file does not exist");
        return 0;
    }

    auto scheduler{Scheduler::get_singleton()};

    std::string output = pUtilities->read_file(path);
    scheduler->schedule_job(output);

    return 0;
}


int loadfile(lua_State* L) {
    luaL_checktype(L, 1, LUA_TSTRING);
    std::string path = lua_tostring(L, 1);

    auto pExecution{Execution::get_singleton()};

    pUtilities->create_workspace();

    if (path.find("..") != std::string::npos) {
        luaG_runerrorL(L, "attempt to escape directory");
        return 0;
    }

    path = (pUtilities->location + "\\workspace\\" + path);

    if (!std::filesystem::exists(path.c_str())) {
        luaG_runerrorL(L, "file does not exist");
        return 0;
    }

    std::string output = pUtilities->read_file(path);

    return pExecution->lua_loadstring(L, output, pUtilities->get_random_string(32), RBX::Identity::Eight_Seven);
}


void FilesystemLibrary::register_environment(lua_State *L) {

    static const luaL_Reg reg[] = {
        {"readfile", readfile},
        {"listfiles", listfiles},
        {"writefile", writefile},
        {"makefolder", makefolder},
        {"appendfile", appendfile},
        {"delfile", delfile},
        {"delfolder", delfolder},
        {"isfile", isfile},
        {"isfolder", isfolder},
        {"dofile", dofile},
        {"loadfile", loadfile},
        {nullptr, nullptr},
    };

    lua_pushvalue(L, LUA_GLOBALSINDEX);
    luaL_register(L, nullptr, reg);
    lua_pop(L, 1);
}
