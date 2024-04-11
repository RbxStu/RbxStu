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

void Scheduler::schedule_job(const std::string &luaCode) {
    SchedulerJob job{};
    job.bIsLuaCode = true;
    job.szluaCode = luaCode;
    this->m_sjJobs.emplace(job);
}


SchedulerJob Scheduler::get_scheduler_job() {
    if (this->m_sjJobs.empty())
        return {"", false}; // Stub job.
    auto job = this->m_sjJobs.back();
    this->m_sjJobs.pop();
    return job;
}

void Scheduler::execute_job(SchedulerJob *job) {
    if (!job->bIsLuaCode) return;
    if (job->szluaCode.empty()) return;
    auto utilities{Module::Utilities::get_singleton()};
    auto nSzLuaCode = std::string(
            oxorany_pchar(R"(getgenv()["string"]=getrawmetatable("").__index;script=Instance.new("LocalScript");)")) +
                      job->szluaCode;

    wprintf(oxorany(L"[Scheduler::execute_job] Compiling bytecode...\r\n"));
    auto execution{Execution::GetSingleton()};
    auto bytecode = execution->Compile(nSzLuaCode);
    wprintf(oxorany(L"[Scheduler::execute_job] Compiled Bytecode:\r\n"));
    printf(oxorany_pchar(
                   L"\r\n--------------------\r\n-- BYTECODE START --\r\n--------------------\r\n%s\r\n--------------------\r\n--  BYTECODE END  --\r\n--------------------\r\n"),
           bytecode.c_str());

    wprintf(oxorany(L"[Scheduler::execute_job] Pushing closure...\r\n"));
    auto nLs = luaE_newthread(this->m_lsInitialisedWith);
    nLs->global->cb = this->m_lsInitialisedWith->global->cb;
    if (nLs->userdata != nullptr)free(nLs->userdata);
    auto mem = malloc(sizeof(RBX::Lua::ExtraSpace));    // Userdata size on rbx, check callback userthread.
    nLs->userdata = mem;
    memcpy(nLs->userdata, this->m_lsInitialisedWith->userdata, 0x98);
    RBX::Security::Bypasses::set_thread_security(nLs, RBX::Identity::Eight_Seven);
    RBX::Security::MarkThread(nLs);
    std::string chunkName;
    if (chunkName.empty()) chunkName = utilities->RandomString(32);
    if (luau_load(nLs, chunkName.c_str(), bytecode.c_str(), bytecode.size(), 0) != 0) {
        wprintf(oxorany(L"Failed to load bytecode!\r\n"));
        printf(oxorany_pchar(L"%s"), lua_tostring(nLs, -1));
        wprintf(oxorany(L"\r\n"));
        return;
    }
    auto *pClosure = const_cast<Closure *>(reinterpret_cast<const Closure *>(lua_topointer(nLs, -1)));

    RBX::Security::Bypasses::set_luaclosure_security(pClosure);

    wprintf(oxorany(L"[Scheduler::execute_job] Executing on RBX::ScriptContext::taskDefer...\r\n"));
    // lua_pcall(nLs, 0, 0, 0);
    RBX::Studio::Functions::rTask_defer(nLs);   // Not using this causes issues relating to permissions.
}

lua_State *Scheduler::get_global_executor_state() {
    return this->m_lsInitialisedWith;
}

lua_State *Scheduler::get_global_roblox_state() {
    return this->m_lsRoblox;
}

void Scheduler::initialize_with(lua_State *L, lua_State *rL) {
    this->m_lsRoblox = rL;
    this->m_lsInitialisedWith = L;
    std::thread([this]() {
        while (true) {
            auto job = this->get_scheduler_job();

            this->execute_job(&job);

            Sleep(77);
        }
    }).detach();
}

bool Scheduler::is_initialized() {
    return this->m_lsInitialisedWith != nullptr && this->m_lsRoblox != nullptr;
}

void Scheduler::re_initialize() {
    this->m_lsInitialisedWith = nullptr;
    this->m_lsRoblox = nullptr;
}