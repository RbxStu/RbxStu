//
// Created by Dottik on 25/11/2023.
//
#include "ClosureLibrary.hpp"
#include <Closures.hpp>
#include <Utilities.hpp>
#include <lgc.h>
#include <lobject.h>
#include <lua.h>
#include <oxorany.hpp>
#include <string>
#include "Execution.hpp"


int iscclosure(lua_State *L) {
    luaL_checktype(L, 1, LUA_TFUNCTION);
    lua_pushboolean(L, lua_iscfunction(L, 1));
    return 1;
}

int islclosure(lua_State *L) {
    luaL_checktype(L, 1, LUA_TFUNCTION);
    lua_pushboolean(L, lua_isLfunction(L, 1));
    return 1;
}

int newcclosure(lua_State *L) {
    const auto closures{Module::Closures::GetSingleton()};
    luaL_checktype(L, 1, LUA_TFUNCTION);


    if (lua_iscfunction(L, 1) == 1) {
        lua_pushvalue(L, 1); // Why do you give me a C closure already?.
        return 1;
    }

    // Use wrapper.
    closures->ToCClosure(L, 1);
    return 1;
}

int newlclosure(lua_State *L) {
    const auto closures{Module::Closures::GetSingleton()};
    luaL_checktype(L, 1, LUA_TFUNCTION);


    if (lua_isLfunction(L, 1) == 1) {
        lua_pushvalue(L, 1); // Why do you give me an L closure already?.
        return 1;
    }

    // Use wrapper.
    closures->ToLClosure(L, 1);
    return 1;
}

int hookfunction(lua_State *L) {
    // Being honest, I'm lazy to write all the statement for error handling I did before lol.
    luaL_checktype(L, 1, LUA_TFUNCTION);
    luaL_checktype(L, 2, LUA_TFUNCTION);
    const auto closures{Module::Closures::GetSingleton()};
    auto *toHook = lua_toclosure(L, 1);
    auto *hookWith = lua_toclosure(L, 2);
    const auto hookWithUpvalues = hookWith->nupvalues;

    // Strip some data from the proto to prevent detections on hooks.
    if (!hookWith->isC)
        RBX::Security::Bypasses::wipe_proto(hookWith);

    // L->L
    if (!toHook->isC && !hookWith->isC) {
        lua_clonefunction(L, 1); // Clone original LClosure

        toHook->env = hookWith->env; // If we crash, we will leak env on an xpcall, terrible idea.
        toHook->stacksize = hookWith->stacksize;
        toHook->preload = hookWith->preload;

        for (int i = 0; i < hookWithUpvalues; i++)
            // Set urfs
            setobj2n(L, &toHook->l.uprefs[i], &hookWith->l.uprefs[i]);

        toHook->nupvalues = hookWith->nupvalues;
        toHook->l.p = hookWith->l.p;

        return 1;
    }

    // NC->NC
    if (closures->IsCClosureHandler(toHook) && closures->IsCClosureHandler(hookWith)) {
        // In this case we want to duplicate our original newcclosure handler for cloning, after which we manipulate the
        // toHook with our hookWith->c.f.
        auto originalWrapped = closures->FindWrappedClosure(toHook);
        L->top->tt = LUA_TFUNCTION;
        L->top->value.p = originalWrapped;
        L->top++;
        closures->ToCClosure(L, -1);

        auto *nWrapped = lua_toclosure(L, -1); // Clone the NewCClosure.

        closures->AddWrappedClosure(toHook, closures->FindWrappedClosure(hookWith));

        L->top->tt = LUA_TFUNCTION;
        L->top->value.p = nWrapped;
        L->top++;
        return 1;
    }

    // L->NC
    if (!toHook->isC && closures->IsCClosureHandler(hookWith)) {
        // We want to grab the backing L closure of the newcclosure, this way we can just hook L->L easily.
        // We just need to get our hookWith to be the original L closure.

        if (closures->FindWrappedClosure(hookWith)->isC) {
            // We must convert the closure it wraps into an L closure.
            auto nCl = closures->FindWrappedClosure(hookWith);
            L->top->tt = LUA_TFUNCTION;
            L->top->value.p = nCl;
            L->top++;
            closures->ToLClosure(L, -1);
        } else {
            auto nCl = closures->FindWrappedClosure(hookWith);
            L->top->tt = LUA_TFUNCTION;
            L->top->value.p = nCl;
            L->top++;
        }
        // ReSharper disable once CppDeclarationHidesLocal
        const auto *hookWith = lua_toclosure(L, -1); // L Closure at stack top, variable shadowing on top!

        // L->L
        lua_clonefunction(L, 1); // Clone original LClosure

        toHook->env = hookWith->env;
        toHook->stacksize = hookWith->stacksize;
        toHook->preload = hookWith->preload;

        for (int i = 0; i < hookWithUpvalues; i++)
            // Set urfs
            setobj2n(L, &toHook->l.uprefs[i], &hookWith->l.uprefs[i]);

        toHook->nupvalues = hookWith->nupvalues;
        toHook->l.p = hookWith->l.p;

        return 1;
    }

    // L->C
    if (!toHook->isC && !closures->IsCClosureHandler(hookWith) && hookWith->isC) {
        closures->ToLClosure(L, 2);
        // ReSharper disable once CppDeclarationHidesLocal
        const auto *hookWith = lua_toclosure(L, -1); // L Closure at stack top, variable shadowing on top!

        // L->L
        lua_clonefunction(L, 1); // Clone original LClosure

        toHook->env = hookWith->env;
        toHook->stacksize = hookWith->stacksize;
        toHook->preload = hookWith->preload;

        for (int i = 0; i < hookWithUpvalues; i++)
            // Set urfs
            setobj2n(L, &toHook->l.uprefs[i], &hookWith->l.uprefs[i]);

        toHook->nupvalues = hookWith->nupvalues;
        toHook->l.p = hookWith->l.p;

        return 1;
    }

    // C->L
    if (!closures->IsCClosureHandler(toHook) && toHook->isC && !hookWith->isC) {
        closures->AddWrappedClosure(toHook, hookWith); // Link toHook with hookWith
        closures->CloneClosure(L, toHook); // Clone original Closure.
        toHook->c.f = NewCClosureHandler; // Set handler
        return 1;
    }

    // C->NC
    if (!closures->IsCClosureHandler(toHook) && toHook->isC && closures->IsCClosureHandler(hookWith)) {
        if (closures->FindWrappedClosure(hookWith)->isC) {
            // We must get original.
            const auto nCl = closures->FindWrappedClosure(hookWith);
            L->top->tt = LUA_TFUNCTION;
            L->top->value.p = nCl;
            L->top++;
        } else {
            // Original is L closure, we cannot really hook it. We must channel it through the NC.
            const auto nCl = closures->FindWrappedClosure(hookWith);
            L->top->tt = LUA_TFUNCTION;
            L->top->value.p = nCl;
            L->top++;
        }
        auto *original = lua_toclosure(L, -1); // C Closure at stack top, variable shadowing on top!

        // C->C
        const auto cl = closures->CloneClosure(L, toHook); // Clone original Closure.

        if (toHook->nupvalues < hookWith->nupvalues)
            luaG_runerror(L, "Hookfunction: Cannot hook. Too many upvalues.\r\n");

        toHook->c.f = []([[maybe_unused]] lua_State *L) -> int { return 0; };
        /* we don't wanna break while we set upvalues */
        for (int i = 0; i < hookWith->nupvalues; i++)
            setobj2n(L, &toHook->c.upvals[i], &hookWith->c.upvals[i]);

        toHook->nupvalues = hookWith->nupvalues;
        toHook->c.f = NewCClosureHandler; // Newcclosure handler.
        closures->AddWrappedClosure(toHook, original);
        L->top->value.p = const_cast<Closure *>(cl);
        L->top->tt = LUA_TFUNCTION;
        L->top++;
        return 1;
    }

    // NC->C/L
    if (closures->IsCClosureHandler(toHook) && !closures->IsCClosureHandler(hookWith)) {
        /*
         *  1 - toHook
         *  2 - hookWith
         *  3 - newcclosureehandler
         * */
        auto originalToHook = closures->FindWrappedClosure(toHook);
        if (!hookWith->isC)
            lua_ref(L, 2);
        closures->AddWrappedClosure(toHook, hookWith);

        lua_pushcclosure(L, NewCClosureHandler, nullptr, 0);
        closures->AddWrappedClosure(lua_toclosure(L, -1), originalToHook);
        return 1;
    }

    if (!closures->IsCClosureHandler(toHook) && !closures->IsCClosureHandler(hookWith)) {
        // Simple C->C hooking.
        const auto cl = closures->CloneClosure(L, toHook); // Clone original Closure.

        // Not required, we are wrapping using newcclosure, and upvalues do not matter thanks to this.
        // if (toHook->nupvalues < hookWith->nupvalues)
        //    luaG_runerror(L, "Hookfunction: Cannot hook. Too many upvalues.\r\n");

        toHook->c.f = []([[maybe_unused]] lua_State *L) -> int { return 0; };
        /* we don't wanna break while we set upvalues */
        for (int i = 0; i < hookWith->nupvalues; i++)
            setobj2n(L, &toHook->c.upvals[i], &hookWith->c.upvals[i]);

        toHook->nupvalues = hookWith->nupvalues;
        toHook->c.f = NewCClosureHandler; // Newcclosure handler.
        closures->AddWrappedClosure(toHook, hookWith);
        L->top->value.p = const_cast<Closure *>(cl);
        L->top->tt = LUA_TFUNCTION;
        L->top++;
        return 1;
    }

    printf("hookf: Not implemented\r\n");
    lua_pushnil(L); // Should never happen, but in the remote case it does... uhmmm, cope!
    return 1;
}

int loadstring(lua_State *L) {
    const auto execution{Execution::get_singleton()};
    const auto utilities{Module::Utilities::get_singleton()};

    luaL_checktype(L, 1, LUA_TSTRING);

    const std::string scriptText = lua_tostring(L, 1);
    const char *const chunkName = luaL_optstring(L, 2, utilities->get_random_string(16).c_str());

    if (std::string(scriptText).empty() || lua_type(L, 1) != LUA_TSTRING) {
        lua_pushnil(L);
        return 1;
    }


    return execution->lua_loadstring(L, scriptText, chunkName, RBX::Identity::Eight_Seven);
    // Return the Execution implementation of the custom luau_loadstring.
}

int clonefunction(lua_State *L) {
    // TODO: Implement
    luaL_checktype(L, 1, LUA_TFUNCTION);
    const auto closures{Module::Closures::GetSingleton()};
    const auto newClosure = closures->CloneClosure(L, static_cast<Closure *>(const_cast<void *>(lua_topointer(L, -1))));

    if (newClosure == nullptr)
        luaL_error(L, "Failed to clone closure!");

    return 1;
}

int isourclosure(lua_State *L) {
    luaL_checktype(L, 1, LUA_TFUNCTION);

    if (const auto *cl = static_cast<const Closure *>(lua_topointer(L, 1)); cl->isC) {
        lua_pushboolean(L, NewCClosureHandler == cl->c.f || (cl->c.debugname == nullptr || cl->c.debugname == nullptr));
    } else {
        lua_pushboolean(L, cl->l.p->linedefined == -1);
    }
    return 1;
}

void ClosureLibrary::register_environment(lua_State *L) {
    static const luaL_Reg reg[] = {
            {"isourclosure", isourclosure},
            {"isexecutorclosure", isourclosure},
            {"checkclosure", isourclosure},

            {"iscclosure", iscclosure},
            {"islclosure", islclosure},

            {"newcclosure", newcclosure},
            {"newlclosure", newlclosure},

            {"hookfunction", hookfunction},
            {"hookfunc", hookfunction},
            {"replaceclosure", hookfunction},

            {"clonefunction", clonefunction},

            {"loadstring", loadstring},
            {"pushstring", loadstring},
            {"compile", loadstring},

            {nullptr, nullptr},
    };

    lua_pushvalue(L, LUA_GLOBALSINDEX);
    luaL_register(L, nullptr, reg);
    lua_pop(L, 1);
}
