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

Execution *Execution::get_singleton() {
    if (Execution::singleton == nullptr)
        Execution::singleton = new Execution();
    return Execution::singleton;
}

std::string Execution::compile_to_bytecode(const std::string &code) {
    auto opts = Luau::CompileOptions{};
    opts.debugLevel = 2;
    opts.optimizationLevel = 2;
    // const char *mutableGlobals[] = {"_G", nullptr};
    // opts.mutableGlobals = mutableGlobals;
    return Luau::compile(code, opts);
}

int Execution::lua_loadstring(lua_State *L, const std::string &code, std::string chunkName, RBX::Identity identity) {
    auto utilities{Module::Utilities::get_singleton()};
    auto wCode = utilities->to_wchar(code.c_str());
    delete[] wCode;

    auto bytecode = this->compile_to_bytecode(code);
    if (chunkName.empty()) chunkName = utilities->get_random_string(32);

    if (luau_load(L, chunkName.c_str(), bytecode.c_str(), bytecode.size(), 0) != LUA_OK) {
        const char *err = lua_tostring(L, -1);
        printf("[Execution::lua_loadstring] luau_load failed with error \'%s\'", err);
        lua_pop(L, 1);
        lua_pushnil(L);
        lua_pushlstring(L, err, strlen(err));
        return 2;
    }

    auto *pClosure = const_cast<Closure *>(static_cast<const Closure *>(lua_topointer(L, -1)));

    RBX::Security::Bypasses::set_luaclosure_security(pClosure, identity);

    return 1;
}

int Execution::register_environment(lua_State *L, bool useInitScript) {
    return Environment::get_singleton()->register_env(L, useInitScript);
}
