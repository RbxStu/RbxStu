//
// Created by Dottik on 14/11/2023.
//

#pragma once

// Lua
#include "Dependencies/Luau/VM/include/lua.h"


class Environment {
    static Environment *sm_pSingleton;

    bool m_bInstrumentEnvironment = false;

public:
    static Environment *get_singleton();

    [[nodiscard]] bool get_instrumentation_status() const;
    void set_instrumentation_status(bool bState);
    int register_env(lua_State *L, bool useInitScript, _In_ _Out_ int *schedulerKey);
};
