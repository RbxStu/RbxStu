//
// Created by Dottik on 25/11/2023.
//
#pragma once


#include "EnvironmentLibrary.hpp"

class DebugLibrary final : public EnvironmentLibrary {
protected:
    ~DebugLibrary() = default;

public:
    void register_environment(lua_State *L) override;
};
