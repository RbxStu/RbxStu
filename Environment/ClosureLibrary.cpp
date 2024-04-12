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


int iscclosure(lua_State *L) {
    auto utilities{Module::Utilities::get_singleton()};
    if (lua_gettop(L) < 1 || lua_type(L, 1) != LUA_TFUNCTION) {
        luaL_error(L,   // Cursed tower of pain!
                   utilities->ToString(
                           std::wstring((oxorany(L"iscclosure: bad parameter #1. Expected function, got ")) +
                                        utilities->ToWideString(lua_typename(L, 1)))).c_str());
    }

    lua_pushboolean(L, lua_iscfunction(L, 1));
    return 1;
}

int islclosure(lua_State *L) {
    auto utilities{Module::Utilities::get_singleton()};
    if (lua_gettop(L) < 1 || lua_type(L, 1) != LUA_TFUNCTION) {
        luaL_error(L,   // Cursed tower of pain!
                   utilities->ToString(
                           std::wstring((oxorany(L"islclosure: bad parameter #1. Expected function, got ")) +
                                        utilities->ToWideString(lua_typename(L, 1)))).c_str());
    }
    lua_pushboolean(L, lua_isLfunction(L, 1));
    return 1;
}

int newcclosure(lua_State *L) {
    auto utilities{Module::Utilities::get_singleton()};
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

int newlclosure(lua_State *L) {
    auto utilities{Module::Utilities::get_singleton()};
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

int hookfunction(lua_State *L) {
    // Being honest, I'm lazy to write all the statement for error handling I did before lol.
    luaL_checktype(L, 1, LUA_TFUNCTION);
    luaL_checktype(L, 2, LUA_TFUNCTION);
    auto closures{Module::Closures::GetSingleton()};
    auto *toHook = lua_toclosure(L, 1);
    auto *hookWith = lua_toclosure(L, 2);
    auto toHookUpvalues = toHook->nupvalues;
    auto hookWithUpvalues = hookWith->nupvalues;

    // L->L
    if (!toHook->isC && !hookWith->isC) {
        lua_clonefunction(L, 1); // Clone original LClosure

        toHook->env = hookWith->env; // If we crash, we will leak env on an xpcall, terrible idea.
        toHook->stacksize = hookWith->stacksize;
        toHook->preload = hookWith->preload;

        for (int i = 0; i < hookWithUpvalues; i++)
            // Set urfs
                setobj2n (L, &toHook->l.uprefs[i], &hookWith->l.uprefs[i]);

        toHook->nupvalues = hookWith->nupvalues;
        toHook->l.p = hookWith->l.p;

        return 1;
    }

    // NC->NC
    if (closures->IsCClosureHandler(toHook) && closures->IsCClosureHandler(hookWith)) {
        // In this case we want to duplicate our original newcclosure handler for cloning, after which we manipulate the toHook with our hookWith->c.f.
        auto originalWrapped = closures->FindWrappedClosure(toHook);
        L->top->tt = LUA_TFUNCTION;
        L->top->value.p = originalWrapped;
        L->top++;
        closures->ToCClosure(L, -1);

        auto *nWrapped = lua_toclosure(L, -1);  // Clone the NewCClosure.

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
        auto *hookWith = lua_toclosure(L, -1);   // L Closure at stack top, variable shadowing on top!

        // L->L
        lua_clonefunction(L, 1); // Clone original LClosure

        toHook->env = hookWith->env;
        toHook->stacksize = hookWith->stacksize;
        toHook->preload = hookWith->preload;

        for (int i = 0; i < hookWithUpvalues; i++)
            // Set urfs
                setobj2n (L, &toHook->l.uprefs[i], &hookWith->l.uprefs[i]);

        toHook->nupvalues = hookWith->nupvalues;
        toHook->l.p = hookWith->l.p;

        return 1;
    }

    // L->C
    if (!toHook->isC && !closures->IsCClosureHandler(hookWith) && hookWith->isC) {
        closures->ToLClosure(L, 2);
        auto *hookWith = lua_toclosure(L, -1);   // L Closure at stack top, variable shadowing on top!

        // L->L
        lua_clonefunction(L, 1); // Clone original LClosure

        toHook->env = hookWith->env;
        toHook->stacksize = hookWith->stacksize;
        toHook->preload = hookWith->preload;

        for (int i = 0; i < hookWithUpvalues; i++)
            // Set urfs
                setobj2n (L, &toHook->l.uprefs[i], &hookWith->l.uprefs[i]);

        toHook->nupvalues = hookWith->nupvalues;
        toHook->l.p = hookWith->l.p;

        return 1;
    }

    // C->L
    if (!closures->IsCClosureHandler(toHook) && toHook->isC && !hookWith->isC) {
        closures->AddWrappedClosure(toHook, hookWith);  // Link toHook with hookWith
        closures->CloneClosure(L, toHook); // Clone original Closure.
        toHook->c.f = NewCClosureHandler;    // Set handler
        return 1;
    }

    // C->NC
    if (!closures->IsCClosureHandler(toHook) && toHook->isC && closures->IsCClosureHandler(hookWith)) {
        if (closures->FindWrappedClosure(hookWith)->isC) {
            // We must get original.
            auto nCl = closures->FindWrappedClosure(hookWith);
            L->top->tt = LUA_TFUNCTION;
            L->top->value.p = nCl;
            L->top++;
        } else {
            auto nCl = closures->FindWrappedClosure(hookWith);
            L->top->tt = LUA_TFUNCTION;
            L->top->value.p = nCl;
            L->top++;
        }
        auto *original = lua_toclosure(L, -1);   // C Closure at stack top, variable shadowing on top!

        // C->C
        auto cl = closures->CloneClosure(L, toHook); // Clone original Closure.

        toHook->c.f = [](lua_State *L) -> int { return 0; }; /* we don't wanna break while we set upvalues */
        for (int i = 0; i < hookWith->nupvalues; i++)
                setobj2n (L, &toHook->c.upvals[i], &hookWith->c.upvals[i]);

        toHook->nupvalues = hookWith->nupvalues;
        toHook->c.f = NewCClosureHandler;    // Newcclosure handler.
        closures->AddWrappedClosure(toHook, original);
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
            TValue *cl_tval = &hookWith->c.upvals[i];
            TValue *ncl_tval = &toHook->c.upvals[i];

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

int loadstring(lua_State *L) {
    auto execution{Execution::GetSingleton()};
    auto utilities{Module::Utilities::get_singleton()};

    luaL_checktype(L, 1, LUA_TSTRING);

    const std::string scriptText = lua_tostring(L, 1);
    const char *const chunkName = luaL_optstring(L, 2, utilities->RandomString(16).c_str());

    if (std::string(scriptText).empty() || lua_type(L, 1) != LUA_TSTRING) {
        lua_pushnil(L);
        return 1;
    }


    return execution->lua_loadstring(L, scriptText,
                                     chunkName,
                                     RBX::Identity::Eight_Seven);    // Return the Execution implementation of the custom luau_loadstring.
}

int clonefunction(lua_State *L) {
    //TODO: Implement
    luaL_checktype(L, 1, LUA_TFUNCTION);
    auto closures{Module::Closures::GetSingleton()};
    auto newClosure = closures->CloneClosure(L, reinterpret_cast<Closure *>(const_cast <void *>(lua_topointer(L, -1))));

    if (newClosure == nullptr)
        luaL_error(L, oxorany_pchar(L"Failed to clone closure!"));

    return 1;
}

int isourclosure(lua_State *L) {
    luaL_checktype(L, 1, LUA_TFUNCTION);

    auto *cl = reinterpret_cast<Closure *>(const_cast<void *>(lua_topointer(L, 1)));

    if (cl->isC) {
        if (NewCClosureHandler == cl->c.f) {
            lua_pushboolean(L, true);
        } else {
            // Cheap solution for isourclosure without doing warcrimes..
            lua_pushboolean(L, cl->c.debugname == nullptr);
        }
    } else {
        if (cl->l.p->linedefined == -1) {
            lua_pushboolean(L, true);
        }
    }
    return 1;

}

void ClosureLibrary::RegisterEnvironment(lua_State *L) {
    static const luaL_Reg reg[] = {
            {oxorany_pchar(L"isourclosure"),      isourclosure},
            {oxorany_pchar(L"isexecutorclosure"), isourclosure},

            {oxorany_pchar(L"iscclosure"),        iscclosure},
            {oxorany_pchar(L"islclosure"),        islclosure},
            {oxorany_pchar(L"newcclosure"),       newcclosure},
            {oxorany_pchar(L"newlclosure"),       newlclosure},

            {oxorany_pchar(L"hookfunction"),      hookfunction},
            {oxorany_pchar(L"hookfunc"),          hookfunction},
            {oxorany_pchar(L"replaceclosure"),    hookfunction},
            {oxorany_pchar(L"clonefunction"),     clonefunction},

            {oxorany_pchar(L"loadstring"),        loadstring},
            {oxorany_pchar(L"pushstring"),        loadstring},
            {oxorany_pchar(L"compile"),           loadstring},

            {nullptr,                             nullptr},
    };

    lua_pushvalue(L, oxorany(LUA_GLOBALSINDEX));
    luaL_register(L, nullptr, reg);
    lua_pop(L, oxorany(1));
}
