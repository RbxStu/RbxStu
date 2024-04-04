//
// Created by Dottik on 25/11/2023.
//
#pragma once


#include "EnvironmentLibrary.hpp"

class DebugLibrary : public EnvironmentLibrary {
public:
    void RegisterEnvironment(lua_State *L) override;
};

