//
// Created by Dottik on 22/11/2023.
//
#include <iostream>
#include <lapi.h>
#include <lua.h>
#include <lualib.h>

#include "Closures.hpp"
#include "Execution.hpp"
#include "lfunc.h"
#include "lgc.h"

/*
 *  TODO: Rewrite implementation of the Closure map. The closure map, due to its nature, when hooking NC->NC, we will be
 * leaking memory. as the backing L closure of the newcclosure is now leaked into memory, and not being able to being
 * freed (lua_ref(L, X))
 * */

Module::Closures *Module::Closures::singleton = nullptr;

Module::Closures *Module::Closures::GetSingleton() {
    if (Closures::singleton == nullptr)
        Closures::singleton = new Closures();

    return Closures::singleton;
}

bool Module::Closures::IsCClosureHandler(const Closure *cl) { return cl->isC && cl->c.f == NewCClosureHandler; }


void Module::Closures::AddWrappedClosure(Closure *wrapper, Closure *original) { this->closureMap[wrapper] = original; }


Closure *Module::Closures::FindWrappedClosure(Closure *wrapper) { return this->closureMap.find(wrapper)->second; }

static void set_proto(Proto *proto, uintptr_t *proto_identity) {
    // NOLINT(*-no-recursion)
    proto->userdata = static_cast<void *>(proto_identity);
    for (auto i = oxorany(0); i < proto->sizep; i++)
        set_proto(proto->p[i], proto_identity);
}

const Closure *Module::Closures::CloneClosure(lua_State *L, Closure *cl) {
    if (cl->isC) {
        Closure *newcl = luaF_newCclosure(L, cl->nupvalues, cl->env);

        if (cl->c.debugname != nullptr)
            newcl->c.debugname = (const char *) cl->c.debugname;

        for (int i = 0; i < cl->nupvalues; i++)
            setobj2n(L, &newcl->c.upvals[i], &cl->c.upvals[i])

                    newcl->c.f = cl->c.f;
        newcl->c.cont = cl->c.cont;

        setclvalue(L, L->top, newcl) L->top++;

        // Allow newcclosures to be cloned successfully by cloning the original for the wrapper.
        if (this->IsCClosureHandler(cl))
            this->AddWrappedClosure(newcl, this->FindWrappedClosure(cl));

        return static_cast<const Closure *>(lua_topointer(L, -1));
    } else {
        setclvalue(L, L->top, cl) L->top++;
        lua_clonefunction(L, -1);
        auto l = static_cast<Closure *>(const_cast<void *>(lua_topointer(L, -1)))->l;
        set_proto(l.p, static_cast<std::uintptr_t *>(l.p->userdata != nullptr
                                                             ? l.p->userdata
                                                             : malloc(sizeof(std::uintptr_t)))); // Copy proto.
        return static_cast<const Closure *>(lua_topointer(L, -1));
    }
}

/*
 * Wraps a C Closure in a Lua Closure. Useful for hooking.  | Pushes the resulting L closure to the top of the stack.
 * @param L Lua State.
 * @param idx Index on lua stack
 * */
void Module::Closures::ToLClosure(lua_State *L, int idx) const {
    // "function inspired by Joe's code, thx Joe for being so cool"
    //  - Dottik
    auto utilities{Module::Utilities::get_singleton()};
    auto execution{Execution::get_singleton()};
    lua_newtable(L); // t
    lua_newtable(L); // Meta

    lua_pushvalue(L, LUA_GLOBALSINDEX);
    lua_setfield(L, -2, "__index");
    lua_setreadonly(L, -1, true);
    lua_setmetatable(L, -2);

    lua_pushvalue(L, idx < 0 ? idx - 1 : idx); // Push a copy of the val at idx into the top
    lua_setfield(L, -2, "abcdefg"); // Set abcdefg to that of idx
    auto code = "return abcdefg(...)";
    if (auto bytecode = execution->compile_to_bytecode(code);
        luau_load(L, utilities->get_random_string(32).c_str(), bytecode.c_str(), bytecode.size(), -1) != LUA_OK) {
        std::cout << "Failure. luau_load failed: " << lua_tostring(L, -1) << std::endl;
    }
    lua_ref(L, -1);
}

/*
 * Wraps a Lua Closure in a C closure. Useful for hooking. | Pushes the CClosure onto the stack.
 * @param L Lua State.
 * @param idx Index on lua stack
 * */
void Module::Closures::ToCClosure(lua_State *L, int idx) {
    auto nIdx = idx < 0 ? idx - 1 : idx;
    // We use relative indexes on this function. If we do not do this, we will not correctly register the wrapped
    // closure.
    lua_ref(L, idx); // Avoid collection.
    lua_pushcclosure(L, NewCClosureHandler, nullptr, 0);
    this->AddWrappedClosure(lua_toclosure(L, -1), lua_toclosure(L, nIdx));
}

int NewCClosureHandler(lua_State *L) {
    const auto argc = lua_gettop(L);
    auto closures{Module::Closures::GetSingleton()};

    // Get og closure out of map from this newcclosure closure object.
    // then get the Value and thats the closure we gotta invoke.
    Closure *realClosure = closures->FindWrappedClosure(clvalue(L->ci->func));

    if (realClosure == nullptr) {
        printf("Failed to execute newcclosure handler: Real closure not found.\r\n");
        return 0; // Failed to map, shit.
    }

    luaC_threadbarrier(L);
    L->top->value.p = realClosure;
    L->top->tt = LUA_TFUNCTION;
    L->top++; // Increase top

    lua_insert(L, 1);

    const auto callResult = lua_pcall(L, argc, LUA_MULTRET, 0);
    if (callResult != LUA_OK && callResult != LUA_YIELD &&
        std::strcmp(luaL_optstring(L, -1, ""), "attempt to yield across metamethod/C-call boundary") == 0) {
        printf("\r\n[Closures::NewCClosureHandler] The wrapped lua closure needs to yield!\r\n");
        return lua_yield(L, LUA_MULTRET);
    }

    if (callResult == LUA_ERRRUN)
        lua_error(
                L); // If we don't error, we will leave it as a string at the stack top. This will cause us to get dtc.

    return lua_gettop(L);
}
