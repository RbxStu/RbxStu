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

    std::string _workspacePath;

public:
    void set_workspace_path(const std::string &workspacePath) {
        if (workspacePath.length() == 0) {
            throw std::runtime_error("Your workspace directory may not be that of an empty string.");
        }

        if (workspacePath.find("..")) {
            throw std::runtime_error(
                    "You cannot set your workspace path to be any that contains \'..\' in it due to safety concerns.");
        }


        this->_workspacePath = workspacePath;
    }
    void register_environment(lua_State *L) override;
};
