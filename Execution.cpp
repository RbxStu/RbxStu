//
// Created by Dottik on 14/11/2023.
//

#include <iostream>

#include "Environment/Environment.hpp"
#include <Execution.hpp>
#include <Luau/Compiler.h>
#include "oxorany.hpp"
#include "Utilities.hpp"
#include "Security.hpp"

Execution *Execution::singleton = nullptr;

Execution *Execution::GetSingleton() {
    if (Execution::singleton == nullptr)
        Execution::singleton = new Execution();
    return Execution::singleton;
}

std::string Execution::Compile(const std::string &code) {
    auto opts = Luau::CompileOptions{};
    opts.debugLevel = 1;
    opts.optimizationLevel = 2;
    const char *mutableGlobals[] = {"_G", nullptr};
    opts.mutableGlobals = mutableGlobals;
    return Luau::compile(code, opts);
}

int Execution::lua_loadstring(lua_State *L, const std::string &code, std::string chunkName, RBX::Identity identity) {
    auto utilities{Module::Utilities::get_singleton()};
    auto wCode = utilities->ToWideCharacter(code.c_str());
    delete[] wCode;

    auto bytecode = this->Compile(code);
    if (chunkName.empty()) chunkName = utilities->RandomString(32);

    if (luau_load(L, chunkName.c_str(), bytecode.c_str(), bytecode.size(), 0) != LUA_OK) {
        std::string err = lua_tostring(L, -1);
        const wchar_t *wErr = utilities->ToWideCharacter(err.c_str());
        lua_pop(L, 1);
        std::wcout << oxorany(L"Compilation Error ->") << wErr << std::endl;
        delete[] wErr;
        lua_pushnil(L);
        lua_pushlstring(L, err.c_str(), err.length());
        return 2;
    }

    auto *pClosure = const_cast<Closure *>(reinterpret_cast<const Closure *>(lua_topointer(L, -1)));

    RBX::Security::Bypasses::set_luaclosure_security(pClosure, identity);

    return 1;
}

int Execution::RegisterEnvironment(lua_State *L, bool useInitScript) {
    return Environment::GetSingleton()->Register(L, useInitScript);
}
