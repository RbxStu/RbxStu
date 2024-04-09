//
// Created by Dottik on 22/11/2023.
//
#include <map>
#include <lapi.h>
#include <lua.h>
#include <lualib.h>
#include <lstate.h>

#include "Utilities.hpp"
#include "oxorany.hpp"

#pragma once

namespace Module {
    class Closures {
    private:
        static Closures *singleton;
        std::map<Closure *, Closure *> closureMap;

        Closures() = default;

    public:
        static Closures *GetSingleton();

        /*
         *  Adds a wrapper into the wrapped list of closures
         *  @param L Lua State.
         *  @param wrapper The closure that wraps the original closure.
         *  @param original The closure that is being wrapped.
         */
        void AddWrappedClosure(Closure *wrapper, Closure *original);

        /*
         *  Finds a wrapped closure given its wrapper
         *  @param wrapper The closure that wraps the closure.
         *  @return The closure the given closure wraps.
         */
        Closure *FindWrappedClosure(Closure *closure);

        void ToLClosure(lua_State *L, int idx) const;

        void ToCClosure(lua_State *L, int idx);

        /*
         *  Clones the given C closure, and pushes it into the top of the lua stack.
         *  Returns the -1 index at the top (The newly cloned closure)
         * */
        const Closure *CloneClosure(lua_State *L, Closure *cl);
    };
}

/*
 *  Handler for the newcclosure function.
 */
int NewCClosureHandler(lua_State *L);