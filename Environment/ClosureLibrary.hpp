//
// Created by Dottik on 25/11/2023.
//

#pragma once

#include "EnvironmentLibrary.hpp"

class ClosureLibrary final : public EnvironmentLibrary {
public:
    virtual ~ClosureLibrary() = default;

    void register_environment(lua_State *L) override;
};
