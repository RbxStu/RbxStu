//
// Created by Dottik on 15/4/2024.
//
#pragma once


#include "EnvironmentLibrary.hpp"

class WebsocketLibrary : public EnvironmentLibrary {
public:
    virtual void RegisterEnvironment(lua_State *L) override;
};
