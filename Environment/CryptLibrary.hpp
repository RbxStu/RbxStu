//
// Created by Dottik on 28/4/2024.
//
#pragma once

#include "EnvironmentLibrary.hpp"

class CryptoLibrary final : public EnvironmentLibrary {
protected:
    ~CryptoLibrary() = default;

public:
    void register_environment(lua_State *L) override;
};
