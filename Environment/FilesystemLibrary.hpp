//
// Created by nhisoka on 21/4/2024.
//

#pragma once
#include <stdexcept>
#include <string>


#include "EnvironmentLibrary.hpp"

class FilesystemLibrary final : public EnvironmentLibrary {
    static FilesystemLibrary *m_singleton;
    std::string location;

protected:
    ~FilesystemLibrary() = default;

public:
    static FilesystemLibrary *get_singleton();

    [[nodiscard]] std::string get_workspace_path() const { return this->location; }

    void register_environment(lua_State *L) override;
};
