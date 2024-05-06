//
// Created by Dottik on 21/4/2024.
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

EXTERN_C IMAGE_DOS_HEADER __ImageBase;

const std::vector<std::string> disallowed_extensions = {
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

FilesystemLibrary *FilesystemLibrary::Singleton = nullptr;

FilesystemLibrary *FilesystemLibrary::get_singleton() {
    if (FilesystemLibrary::Singleton == nullptr)
        FilesystemLibrary::Singleton = new FilesystemLibrary();

    return FilesystemLibrary::Singleton;
}


std::string read_file(std::string file_location) {
    auto close_file = [](FILE* f) { fclose(f); };
    auto holder = std::unique_ptr<FILE, decltype(close_file)>(fopen(file_location.c_str(), "rb"), close_file);

    if (!holder)
        return "";

    FILE* f = holder.get();

    if (fseek(f, 0, SEEK_END) < 0)
        return "";

    const long size = ftell(f);

    if (size < 0)
        return "";

    if (fseek(f, 0, SEEK_SET) < 0)
        return "";

    std::string res;
    res.resize(size);
    fread(const_cast<char*>(res.data()), 1, size, f);

    return res;
}

std::string GetLocation() {
    char DllPath[MAX_PATH] = {0};
    std::string location = FilesystemLibrary::get_singleton()->location;
    GetModuleFileNameA((HINSTANCE)&__ImageBase, DllPath, _countof(DllPath));

    std::string dll_path = DllPath;

    std::string exploit_path = dll_path.substr(0, dll_path.rfind('\\'));

    location = exploit_path;

    return location;
}

void create_workspace() {
    std::string location = FilesystemLibrary::get_singleton()->location;
    
    if (!std::filesystem::exists(location + ("\\workspace")))
        std::filesystem::create_directory(location + ("\\workspace"));
}

std::string replace(std::string subject, std::string search, std::string replace) {
    size_t pos = 0;

    while ((pos = subject.find(search, pos)) != std::string::npos)
    {
        subject.replace(pos, search.length(), replace);
        pos += replace.length();
    }

    return subject;
}

BOOL equals_ignore_case(std::string_view a, std::string_view b) {
    return std::equal(a.begin(), a.end(),
                      b.begin(), b.end(),
                      [](char a, char b) {
                          return tolower(a) == tolower(b);
                      }
    );
}


int listfiles(lua_State *L) {
    luaL_checkstring(L, 1);

    create_workspace();

    std::string path = lua_tostring(L, 1);
    
    std::string location = FilesystemLibrary::get_singleton()->location;

    if (path.find("..") != std::string::npos)
        luaG_runerrorL(L, "Using \'..\' is not permitted for safety reasons.");

    if(!path.empty()) {
        path = (location + "\\workspace\\" + path);
    } else {
        path = (location + "\\workspace");
    }

    if (!std::filesystem::is_directory(path.c_str())) {
        luaG_runerrorL(L, "directory does not exist");;
    }

    lua_newtable(L);

    int idx = 0;
    for (const auto& entry : std::filesystem::directory_iterator(path))
    {
        auto file_path = entry.path().string();
        file_path = replace(file_path, (FilesystemLibrary::get_singleton()->location+ "\\workspace\\"), "");

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

    create_workspace();

    std::string extention = PathFindExtensionA(path.c_str());
    for (std::basic_string<char> test : disallowed_extensions) {
            if (equals_ignore_case(PathFindExtensionA(path.c_str()), test)) {
                luaG_runerrorL(L, "illegal file extension");
            }
    }

    if (path.find("..") != std::string::npos) {
        luaG_runerrorL(L, "attempt to escape directory");;
    }

    path = (FilesystemLibrary::get_singleton()->location+ "\\workspace\\" + path);

    std::ofstream out(path);
    out << content;
    out.close();

    return 0;
}

int makefolder(lua_State *L) {
    luaL_checktype(L, 1, LUA_TSTRING);

    std::string path = lua_tostring(L, 1);

    create_workspace();

    if (path.find("..") != std::string::npos) {
        luaG_runerror(L, "attempt to escape directory");
    }

    path = (FilesystemLibrary::get_singleton()->location+ "\\workspace\\" + path);

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
    for (std::basic_string<char> test : disallowed_extensions) {
        if (equals_ignore_case(extention, test)) {
            luaG_runerror(L, "attempt to escape directory");
        }
    }

    if (path.find("..") != std::string::npos) {
        luaG_runerror(L, "attempt to escape directory");;
    }

    path = (FilesystemLibrary::get_singleton()->location+ "\\workspace\\" + path);

    std::ofstream out;
    out.open(path, std::ios_base::app | std::ios_base::binary);
    out.write(content.c_str(), content.size());
    out.close();

    return 0;
}

int readfile(lua_State *L) {

    luaL_checktype(L, 1, LUA_TSTRING);
    std::string path = lua_tostring(L, 1);

    create_workspace();


    if (path.find("..") != std::string::npos) {
        luaG_runerrorL(L, "attempt to escape directory");
    }

    path = (FilesystemLibrary::get_singleton()->location+ "\\workspace\\" + path);

    if (!std::filesystem::exists(path.c_str())) {
        //std::cout << "file does not exist!\n";
        luaG_runerrorL(L, "file does not exist");
    }

    std::string output = read_file(path);
    lua_pushstring(L, output.c_str());
    return 1;
}

int isfolder(lua_State* L)  {
    luaL_checktype(L, 1, LUA_TSTRING);
    std::string path = lua_tostring(L, 1);

    create_workspace();

    if (path.find("..") != std::string::npos) {
        luaG_runerrorL(L, "attempt to escape directory");
    }

    path = (FilesystemLibrary::get_singleton()->location+ "\\workspace\\" + path);

    lua_pushboolean(L, std::filesystem::is_directory(path));
    return 1;
}

int isfile(lua_State* L) {
    luaL_checktype(L, 1, LUA_TSTRING);
    std::string path = lua_tostring(L, 1);

    create_workspace();

    if (path.find("..") != std::string::npos) {
        luaG_runerrorL(L, "attempt to escape directory");
    }

    path = (FilesystemLibrary::get_singleton()->location+ "\\workspace\\" + path);

    lua_pushboolean(L, std::filesystem::is_regular_file(path));
    return 1;
}

int delfolder(lua_State* L) {
    luaL_checktype(L, 1, LUA_TSTRING);
    std::string path = lua_tostring(L, 1);

    create_workspace();

    if (path.find("..") != std::string::npos) {
        luaG_runerrorL(L, "attempt to escape directory");;
    }

    path = (FilesystemLibrary::get_singleton()->location+ "\\workspace\\" + path);

    if (!std::filesystem::remove_all(path)) {
        luaG_runerrorL(L, "folder does not exist");
    }

    return 0;
}

int delfile(lua_State* L) {
    luaL_checktype(L, 1, LUA_TSTRING);
    std::string path = lua_tostring(L, 1);

    create_workspace();

    if (path.find("..") != std::string::npos) {
        luaG_runerrorL(L, "attempt to escape directory");
    }

    path = (FilesystemLibrary::get_singleton()->location+ "\\workspace\\" + path);

    if (!std::filesystem::remove(path)) {
        luaG_runerrorL(L, "file does not exist");
    }

    return 0;
}

int dofile(lua_State* L) {
    luaL_checktype(L, 1, LUA_TSTRING);
    std::string path = lua_tostring(L, 1);

    create_workspace();

    if (path.find("..") != std::string::npos) {
        luaG_runerrorL(L, "attempt to escape directory");
    }

    path = (FilesystemLibrary::get_singleton()->location+ "\\workspace\\" + path);

    if (!std::filesystem::exists(path.c_str())) {
        luaG_runerrorL(L, "file does not exist");
    }

    auto scheduler{Scheduler::get_singleton()};

    std::string output = read_file(path);
    scheduler->schedule_job(output);

    return 0;
}


int loadfile(lua_State* L) {
    luaL_checktype(L, 1, LUA_TSTRING);
    std::string path = lua_tostring(L, 1);

    auto pExecution{Execution::get_singleton()};

    create_workspace();

    if (path.find("..") != std::string::npos) {
        luaG_runerrorL(L, "attempt to escape directory");
        return 0;
    }

    path = (FilesystemLibrary::get_singleton()->location+ "\\workspace\\" + path);

    if (!std::filesystem::exists(path.c_str())) {
        luaG_runerrorL(L, "file does not exist");
    }

    std::string output = read_file(path);

    return pExecution->lua_loadstring(L, output, "", RBX::Identity::Eight_Seven);
}


void FilesystemLibrary::register_environment(lua_State *L) {
    GetLocation(); // grabs location for the workspace

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
