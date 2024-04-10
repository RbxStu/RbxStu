//
// Created by Dottik on 2/1/2024.
//
#pragma once

#include "lstate.h"
#include <queue>

class SchedulerJob {
public: // TODO: Implement yielding correctly.
    std::string szluaCode;
    // void *threadReference;
    bool bIsLuaCode;
    // bool bIsYieldingJob;
};

class Scheduler {
    //#region Static Fields
    static Scheduler *singleton;
    //#endregion Static Fields

    lua_State *m_lsInitialisedWith;
    lua_State *m_lsRoblox;
    std::queue<SchedulerJob> m_sjJobs;

    SchedulerJob get_scheduler_job();

public:
    //#region Static Members
    static Scheduler *get_singleton();
    //#endregion Static Members

    void execute_job(SchedulerJob *job);

    void schedule_job(const std::string &luaCode);

    void initialize_with(lua_State *L, lua_State *rL);

    bool is_initialized();

    lua_State *get_global_executor_state();
    lua_State *get_global_roblox_state();

    void re_initialize();
};
