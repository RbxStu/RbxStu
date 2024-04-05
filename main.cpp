#define _AMD64_
// C
#include <Windows.h>
#include <MinHook.h>
#include <StudioOffsets.h>
// C++
#include <iostream>
#include <thread>
#include <utility>
#include "oxorany.hpp"
#include "Environment/Environment.hpp"
#include "Execution.hpp"
#include "Utilities.hpp"
#include "Scheduler.hpp"
#include "Hook.hpp"

int main(int argc, char **argv, char **envp) {
    //AllocConsole();
    //freopen_s(reinterpret_cast<FILE **>(stdin), ("CONOUT$"), "w", stdout);
    //freopen_s(reinterpret_cast<FILE **>(stdin), ("CONIN$"), "r", stdin);

    wprintf(oxorany(L"[main] Initializing hook..."));

    auto hook{Hook::get_singleton()};

    hook->install_hook();
    hook->wait_until_initialised();
    hook->remove_hook();
    wprintf(oxorany(L"[main] Hook initialized. State grabbed."));

    wprintf(oxorany(L"[main] Initializing environment.\r\n"));
    auto scheduler{Scheduler::GetSingleton()};
    auto environmentSingleton = Environment::GetSingleton();
    environmentSingleton->Register(scheduler->GetGlobalState(), true);
    std::string str{};

    while (true) {
        str.clear();
        wprintf(L"[main] Input lua code: ");
        getline(std::cin, str);
        wprintf(oxorany(L"Scheduling...\r\n"));
        scheduler->ScheduleJob(str);
        Sleep(1000);
    }

    return 0;
}

BOOL WINAPI DllMain(
        HINSTANCE hinstDLL,  // handle to DLL module
        DWORD fdwReason,     // reason for calling function
        LPVOID lpvReserved)  // reserved
{
    // Perform actions based on the reason for calling.
    switch (fdwReason) {
        case DLL_PROCESS_ATTACH:
            CreateThread(nullptr, 0x1000, reinterpret_cast<LPTHREAD_START_ROUTINE>(main), 0, 0, nullptr);
            break;

        case DLL_THREAD_ATTACH:
            // Do thread-specific initialization.
            break;

        case DLL_THREAD_DETACH:
            // Do thread-specific cleanup.
            break;

        case DLL_PROCESS_DETACH:

            if (lpvReserved != nullptr) {
                break; // do not do cleanup if process termination scenario
            }

            // Perform any necessary cleanup.
            break;
    }
    return TRUE;  // Successful DLL_PROCESS_ATTACH.
}