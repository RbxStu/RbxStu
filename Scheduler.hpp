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
    std::queue<SchedulerJob> m_sjJobs;

    SchedulerJob GetSchedulerJob();

public:
    //#region Static Members
    static Scheduler *GetSingleton();
    //#endregion Static Members

    void Execute(SchedulerJob *job);

    void ScheduleJob(const std::string &luaCode);

    void InitializeWith(lua_State *L);

    bool IsInitialized();

    lua_State *get_global_executor_state();

    void ReInitialize();
};
