//
// Created by Dottik on 25/11/2023.
//
#pragma once


#include "lstate.h"

/*
 *  This class defines the contract required to export environment functions as a "library".
 */
class EnvironmentLibrary {
protected:
    ~EnvironmentLibrary() = default;

public:
    virtual void register_environment(lua_State *L);
};

