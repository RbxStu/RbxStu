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
    static Scheduler *singleton;

    lua_State *m_lsInitialisedWith;
    lua_State *m_lsRoblox;
    std::queue<SchedulerJob> m_sjJobs;

    SchedulerJob get_scheduler_job();

public:
    static Scheduler *get_singleton();

    void execute_job(lua_State *runOn, SchedulerJob *job);

    void schedule_job(std::string luaCode);

    void initialize_with(lua_State *L, lua_State *rL);

    bool is_initialized() const;

    lua_State *get_global_executor_state() const;

    lua_State *get_global_roblox_state() const;

    void scheduler_step(lua_State *runner);

    void re_initialize();
};
