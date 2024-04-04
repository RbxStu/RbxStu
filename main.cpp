#define _AMD64_
// C
#include <Windows.h>
#include <MinHook.h>

// C++
#include <iostream>
#include <thread>
#include <utility>
#include "oxorany.hpp"
#include "Environment/Environment.hpp"
#include "Execution.hpp"
#include "Utilities.hpp"
#include "Scheduler.hpp"

int main(int argc, char **argv, char **envp) {
    wprintf(oxorany(L"[main] Creating and initializing main Lua State.\r\n"));
    auto *luaState = luaL_newstate();
    luaopen_base(luaState);
    luaL_openlibs(luaState);

    wprintf(oxorany(L"[main] Initializing environment.\r\n"));
    auto environmentSingleton = Environment::GetSingleton();
    environmentSingleton->Register(luaState, true);

    // luaL_sandbox(luaState);
    wprintf(oxorany(L"[main] Initializing scheduler.\r\n"));
    auto schedulerSingleton = Scheduler::GetSingleton();
    schedulerSingleton->InitializeWith(luaState);

    std::string str{};
    while (true) {
        str.clear();
        wprintf(L"[main] Input lua code: ");
        getline(std::cin, str);
        wprintf(oxorany(L"Scheduling...\r\n"));
        schedulerSingleton->ScheduleJob(str);
        Sleep(1000);
    }

    return 0;
}