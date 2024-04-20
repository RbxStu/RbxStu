//
// Created by Dottik on 2/1/2024.
//
#define _AMD64_

#include <thread>
#include <synchapi.h>
#include "Scheduler.hpp"
#include "Execution.hpp"
#include "oxorany.hpp"
#include "Utilities.hpp"
#include "Security.hpp"
#include <StudioOffsets.h>

Scheduler *Scheduler::singleton = nullptr;

Scheduler *Scheduler::get_singleton() {
    if (Scheduler::singleton == nullptr)
        Scheduler::singleton = new Scheduler();

    return Scheduler::singleton;
}

void Scheduler::schedule_job(std::string luaCode) {
    SchedulerJob job{};
    job.bIsLuaCode = true;
    job.szluaCode = std::move(luaCode);
    this->m_sjJobs.emplace(job);
}


SchedulerJob Scheduler::get_scheduler_job() {
    if (this->m_sjJobs.empty())
        return {"", false}; // Stub job.
    auto job = this->m_sjJobs.back();
    this->m_sjJobs.pop();
    return job;
}

void Scheduler::execute_job(lua_State *runOn, SchedulerJob *job) {
    if (!job->bIsLuaCode) return;
    if (job->szluaCode.empty()) return;
    auto utilities{Module::Utilities::get_singleton()};
    auto nSzLuaCode = std::string(
                          (
                              R"(getgenv()["string"] = getrawmetatable("").__index;script = Instance.new("LocalScript");)"))
                      +
                      job->szluaCode;

    wprintf(L"[Scheduler::execute_job] Compiling bytecode...\r\n");
    auto execution{Execution::get_singleton()};
    auto bytecode = execution->compile_to_bytecode(nSzLuaCode);
    wprintf(L"[Scheduler::execute_job] Compiled Bytecode:\r\n");
    printf(
        "\r\n--------------------\r\n-- BYTECODE START --\r\n--------------------\r\n%s\r\n--------------------\r\n--  BYTECODE END  --\r\n--------------------\r\n",
        bytecode.c_str());

    wprintf(L"[Scheduler::execute_job] Pushing closure...\r\n");
    auto nLs = luaE_newthread(runOn);
    RBX::Security::Bypasses::set_thread_security(nLs, RBX::Identity::Eight_Seven);
    RBX::Security::MarkThread(nLs);
    std::string chunkName;
    if (chunkName.empty()) chunkName = utilities->get_random_string(32);
    if (luau_load(nLs, chunkName.c_str(), bytecode.c_str(), bytecode.size(), 0) != 0) {
        printf("[Scheduler::execute_job] Failed to load bytecode!\r\n");
        printf("%s\r\n", lua_tostring(nLs, -1));
        return;
    }
    auto *pClosure = const_cast<Closure *>(static_cast<const Closure *>(lua_topointer(nLs, -1)));

    RBX::Security::Bypasses::set_luaclosure_security(pClosure, RBX::Identity::Eight_Seven);

    wprintf(L"[Scheduler::execute_job] Executing on RBX::ScriptContext::taskDefer...\r\n");
    // lua_pcall(nLs, 0, 0, 0);
    RBX::Studio::Functions::rTask_defer(nLs); // Not using this causes issues relating to permissions.
}

lua_State *Scheduler::get_global_executor_state() const {
    return this->m_lsInitialisedWith;
}

lua_State *Scheduler::get_global_roblox_state() const {
    return this->m_lsRoblox;
}

void Scheduler::initialize_with(lua_State *L, lua_State *rL) {
    this->m_lsRoblox = rL;
    this->m_lsInitialisedWith = L;
}

void Scheduler::scheduler_step(lua_State *runner) {
    auto job = this->get_scheduler_job();
    this->execute_job(runner, &job);
}

bool Scheduler::is_initialized() const {
    return this->m_lsInitialisedWith != nullptr && this->m_lsRoblox != nullptr;
}

void Scheduler::re_initialize() {
    this->m_lsInitialisedWith = nullptr;
    this->m_lsRoblox = nullptr;
}
