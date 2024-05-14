//
// Created by Dottik on 2/1/2024.
//
#pragma once

#include <queue>
#include "lstate.h"

#include <functional>
#include <memory>
#include <thread>

#include "lua.h"

namespace std {
    class shared_mutex;
}
namespace RBX {
    struct LiveThreadRef {
        int32_t __atomic__refs;
        lua_State *thread;
        int32_t thread_ref;
        int32_t objectId;
    };

} // namespace RBX

struct SchedulerYieldContext {
    lua_State *plsexecutionThread;
    volatile unsigned int ulnargs;
    volatile unsigned int ulnrets;
    volatile bool bIsWorkCompleted;

    // Constructs a LiveThreadRef for invoking ScriptContext::resume
    // Has side effects on the Lua Registry due to making a thread reference!
    [[nodiscard]] std::unique_ptr<RBX::LiveThreadRef> construct_live_thread_ref_from_yield_context() const {
        // Build threadref with the args we know we can obtain.
        auto threadRef = std::make_unique<RBX::LiveThreadRef>();

        lua_pushthread(this->plsexecutionThread);
        threadRef->thread = this->plsexecutionThread;
        threadRef->thread_ref = lua_ref(this->plsexecutionThread, -1);
        threadRef->objectId = 0;
        threadRef->__atomic__refs = 0;
        lua_pop(this->plsexecutionThread, 1); // Reset stack.

        return threadRef;
    }
};


class SchedulerJob {
public: // TODO: Implement yielding correctly.
    struct lJob {
        std::string szluaCode;
    } luaJob;
    struct yieldJob {
        SchedulerYieldContext *pyieldContext;
        void *thJobRunner;
    } yieldJob;
    bool bIsLuaCode;
    bool bIsYieldingJob;

    SchedulerJob(const SchedulerJob &job) {
        if (job.bIsLuaCode) {
            this->bIsLuaCode = true;
            this->bIsYieldingJob = false;
            this->luaJob = {};
            this->luaJob.szluaCode = job.luaJob.szluaCode;
        } else {
            this->bIsLuaCode = false;
            this->bIsYieldingJob = true;
            this->yieldJob = {};
            this->yieldJob.pyieldContext = job.yieldJob.pyieldContext;
            this->yieldJob.thJobRunner = (job.yieldJob.thJobRunner);
        }
    };

    explicit SchedulerJob(const std::string &luaCode) {
        this->bIsLuaCode = true;
        this->bIsYieldingJob = false;

        this->luaJob = {};
        this->yieldJob = {};

        this->luaJob.szluaCode = luaCode;
    }
    SchedulerJob(SchedulerYieldContext *ctx, std::thread *thJobrunner) {
        this->bIsLuaCode = false;
        this->bIsYieldingJob = true;

        this->luaJob = {};
        this->yieldJob = {};

        this->yieldJob.pyieldContext = ctx;
        this->yieldJob.thJobRunner = (thJobrunner);
    }

    ~SchedulerJob() = default;
};

class Scheduler {
    static Scheduler *singleton;

    lua_State *m_lsInitialisedWith;
    lua_State *m_lsRoblox;
    std::queue<SchedulerJob> m_sjJobs;

    int m_lcurrent_scheduler_key;

    SchedulerJob get_scheduler_job();

public:
    static Scheduler *get_singleton();

    void execute_job(lua_State *runOn, SchedulerJob *job);

    void set_scheduler_key(int newSchedulerKey);

    void schedule_job(const std::string &luaCode);

    void initialize_with(lua_State *L, lua_State *rL);

    bool is_initialized();

    lua_State *get_global_executor_state() const;

    lua_State *get_global_roblox_state() const;

    void scheduler_step(lua_State *runner, int lschedulerKey);

    void
    yield_job(lua_State *L,
              const std::function<std::function<int(lua_State *)>(SchedulerYieldContext *yieldContext)> yield_func) {
        // TODO: Write YieldJob.
    }

    void re_initialize();

    void lock_scheduler();
    void unlock_scheduler();
};
