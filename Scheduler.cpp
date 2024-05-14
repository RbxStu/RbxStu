//
// Created by Dottik on 2/1/2024.
//
#define _AMD64_

#include "Scheduler.hpp"
#include <StudioOffsets.h>
#include <shared_mutex>
#include <synchapi.h>
#include <thread>
#include "Execution.hpp"
#include "Security.hpp"
#include "Utilities.hpp"
#include "oxorany.hpp"

static volatile bool scheduler_locked;
Scheduler *Scheduler::singleton = nullptr;

Scheduler *Scheduler::get_singleton() {
    if (Scheduler::singleton == nullptr)
        Scheduler::singleton = new Scheduler();

    return Scheduler::singleton;
}

void Scheduler::schedule_job(const std::string &luaCode) {
    SchedulerJob job(luaCode);
    this->m_sjJobs.emplace(job);
}

SchedulerJob Scheduler::get_scheduler_job() {
    if (this->m_sjJobs.empty())
        return SchedulerJob(""); // Stub job.
    auto job = this->m_sjJobs.back();
    this->m_sjJobs.pop();
    return job;
}

void Scheduler::execute_job(lua_State *runOn, SchedulerJob *job) {
    if (job->bIsLuaCode) {
        if (job->luaJob.szluaCode.empty())
            return;
        const auto pUtilities{Module::Utilities::get_singleton()};
        const auto pExecution{Execution::get_singleton()};
        const auto nSzLuaCode =
                std::string((
                        R"(getgenv()["string"] = getrawmetatable("").__index;script = Instance.new("LocalScript");)")) +
                job->luaJob.szluaCode;

        wprintf(L"[Scheduler::execute_job] Compiling bytecode...\r\n");
        const auto bytecode = pExecution->compile_to_bytecode(nSzLuaCode);
        wprintf(L"[Scheduler::execute_job] Compiled Bytecode:\r\n");
        printf("\r\n--------------------\r\n-- BYTECODE START "
               "--\r\n--------------------\r\n%s\r\n--------------------\r\n--  BYTECODE END  "
               "--\r\n--------------------\r\n",
               bytecode.c_str());

        wprintf(L"[Scheduler::execute_job] Pushing closure...\r\n");
        auto nLs = luaE_newthread(runOn);
        RBX::Security::Bypasses::reallocate_extraspace(nLs);
        memcpy(nLs->userdata, runOn->userdata,
               0x98); // Copy extra space of runon into nLs, both pointers are guaranteed to be valid.
        RBX::Security::Bypasses::set_thread_security(nLs, RBX::Identity::Eight_Seven);
        std::string chunkName;
        if (chunkName.empty())
            chunkName = pUtilities->get_random_string(32);
        if (luau_load(nLs, chunkName.c_str(), bytecode.c_str(), bytecode.size(), 0) != 0) {
            printf("[Scheduler::execute_job] Failed to load bytecode!\r\n");
            printf("%s\r\n", lua_tostring(nLs, -1));
            return;
        }
        wprintf(L"[Scheduler::execute_job] Setting closure capabilities...\r\n");
        auto *pClosure = const_cast<Closure *>(static_cast<const Closure *>(lua_topointer(nLs, -1)));

        RBX::Security::Bypasses::set_luaclosure_security(pClosure, RBX::Identity::Eight_Seven);
        wprintf(L"[Scheduler::execute_job] Executing on RBX::ScriptContext::taskDefer...\r\n");
        // lua_pcall(nLs, 0, 0, 0);
        RBX::Studio::Functions::rTask_defer(nLs); // Not using this causes issues relating to permissions.
    } else if (job->bIsYieldingJob) {
        // Yield step.
        if (job->yieldJob.pyieldContext == nullptr)
            return; // Stub job.

        if (!job->yieldJob.pyieldContext->bIsWorkCompleted) {
            this->m_sjJobs.emplace(std::move(*job));
            return; // Skip step; Job is not completed, due to this, we skip a step for the next job.
        }

        // auto threadRef = job->yieldJob.pyieldContext->construct_live_thread_ref_from_yield_context();
        lua_resume(runOn, job->yieldJob.pyieldContext->plsexecutionThread, job->yieldJob.pyieldContext->ulnargs);
        // RBX::Studio::Functions::ScriptContext_resume(static_cast<RBX::Lua::ExtraSpace>(job->yieldJob.pyieldContext->plsexecutionThread->userdata).sharedExtraSpace->scriptContext,threadRef);
    }
}

void Scheduler::lock_scheduler() {
    while (scheduler_locked)
        Sleep(16);

    scheduler_locked = true;
}
void Scheduler::unlock_scheduler() { scheduler_locked = false; }

lua_State *Scheduler::get_global_executor_state() const { return this->m_lsInitialisedWith; }

lua_State *Scheduler::get_global_roblox_state() const { return this->m_lsRoblox; }

void Scheduler::set_scheduler_key(int newSchedulerKey) { this->m_lcurrent_scheduler_key = newSchedulerKey; }
void Scheduler::initialize_with(lua_State *L, lua_State *rL) {
    this->lock_scheduler();
    this->m_lsRoblox = rL;
    this->m_lsInitialisedWith = L;
    this->unlock_scheduler();
}

void Scheduler::scheduler_step(lua_State *runner, int lschedulerKey) {
    if (this->m_lcurrent_scheduler_key != lschedulerKey)
        return;
    this->lock_scheduler();
    auto job = this->get_scheduler_job();
    this->execute_job(runner, &job);
    this->unlock_scheduler();
}

bool Scheduler::is_initialized() { return this->m_lsInitialisedWith != nullptr && this->m_lsRoblox != nullptr; }

void Scheduler::re_initialize() {
    this->lock_scheduler();
    this->m_lsInitialisedWith = nullptr;
    this->m_lsRoblox = nullptr;
    this->unlock_scheduler();
}
