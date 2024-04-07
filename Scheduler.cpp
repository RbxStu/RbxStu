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

Scheduler *Scheduler::singleton = nullptr;

Scheduler *Scheduler::GetSingleton() {
    if (Scheduler::singleton == nullptr)
        Scheduler::singleton = new Scheduler();

    return Scheduler::singleton;
}

void Scheduler::ScheduleJob(const std::string &luaCode) {
    SchedulerJob job{};
    job.bIsLuaCode = true;
    job.szluaCode = luaCode;
    this->m_sjJobs.emplace(job);
}


SchedulerJob Scheduler::GetSchedulerJob() {
    if (this->m_sjJobs.empty())
        return {"", false}; // Stub job.
    auto job = this->m_sjJobs.back();
    this->m_sjJobs.pop();
    return job;
}

void Scheduler::Execute(SchedulerJob *job) {
    if (!job->bIsLuaCode) return;
    if (job->szluaCode.empty()) return;
    wprintf(oxorany(L"[Scheduler::Execute] Compiling bytecode...\r\n"));
    auto execution{Execution::GetSingleton()};
    auto bytecode = execution->Compile(job->szluaCode);
    wprintf(oxorany(L"[Scheduler::Execute] Compiled Bytecode:\r\n"));
    printf(oxorany_pchar(
                   L"\r\n--------------------\r\n-- BYTECODE START --\r\n--------------------\r\n%s\r\n--------------------\r\n--  BYTECODE END  --\r\n--------------------\r\n"),
           bytecode.c_str());

    wprintf(oxorany(L"[Scheduler::Execute] Pushing closure...\r\n"));
    auto nLs = luaE_newthread(this->m_lsInitialisedWith);
    nLs->global->cb = this->m_lsInitialisedWith->global->cb;
    auto mem = malloc(0x98);    // Userdata size on rbx, check callback userthread.
    nLs->userdata = mem;
    memcpy(nLs->userdata, this->m_lsInitialisedWith->userdata, 0x98);
    RBX::Security::Bypasses::SetLuastateCapabilities(nLs);
    if (luau_load(nLs, "RLuau", bytecode.c_str(), bytecode.size(), 0) != 0) {
        wprintf(oxorany(L"Failed to load bytecode!\r\n"));
        printf(oxorany_pchar(L"%s"), lua_tostring(nLs, -1));
        wprintf(oxorany(L"\r\n"));
        return;
    }
    auto *pClosure = const_cast<Closure *>(reinterpret_cast<const Closure *>(lua_topointer(nLs, -1)));

    RBX::Security::Bypasses::SetClosureCapabilities(pClosure);

    wprintf(oxorany(L"[Scheduler::Execute] Executing on RBX::ScriptContext::taskDefer...\r\n"));
    // lua_pcall(nLs, 0, 0, 0);
    RBX::Studio::Functions::rTask_defer(nLs);   // Not using this causes issues relating to permissions.
}

lua_State *Scheduler::GetGlobalState() {
    return this->m_lsInitialisedWith;
}

void Scheduler::InitializeWith(lua_State *L) {
    std::thread([this]() {
        while (true) {
            auto job = this->GetSchedulerJob();

            this->Execute(&job);

            Sleep(77);
        }
    }).detach();
    this->m_lsInitialisedWith = L;
}

bool Scheduler::IsInitialized() {
    return m_lsInitialisedWith != nullptr;
}

