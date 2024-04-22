//
// Created by Dottik on 21/4/2024.
//

#pragma once
#include <stdexcept>
#include <string>


#include "EnvironmentLibrary.hpp"

class FilesystemLibrary final : public EnvironmentLibrary {
protected:
    ~FilesystemLibrary() = default;

    static std::string _workspacePath;

public:
    static void set_workspace_path(const std::string &workspacePath);
    void register_environment(lua_State *L) override;
};
