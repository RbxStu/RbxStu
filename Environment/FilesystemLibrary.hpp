//
// Created by nhisoka on 21/4/2024.
//

#pragma once
#include <stdexcept>
#include <string>


#include "EnvironmentLibrary.hpp"

class FilesystemLibrary final : public EnvironmentLibrary {
protected:
    ~FilesystemLibrary() = default;

public:
    void register_environment(lua_State *L) override;

   static std::string location;
};
