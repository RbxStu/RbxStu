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

public:
    ~FilesystemLibrary() = default;
    static FilesystemLibrary *get_singleton();

    void set_workspace_path(const std::string &path) { this->location = path; }
    [[nodiscard]] std::string get_workspace_path() const { return this->location; }

    void register_environment(lua_State *L) override;
};
