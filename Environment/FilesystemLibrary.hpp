//
// Created by Dottik on 21/4/2024.
//

#pragma once
#include "EnvironmentLibrary.hpp"

class FilesystemLibrary final : public EnvironmentLibrary {
protected:
    ~FilesystemLibrary() = default;

public:
    void register_environment(lua_State *L) override;
};