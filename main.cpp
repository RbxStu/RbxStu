// C
#include <Windows.h>
#include <MinHook.h>
#include <StudioOffsets.h>
// C++
#include <iostream>
#include <thread>
#include <utility>
#include <DbgHelp.h>
#include "oxorany.hpp"
#include "Environment/Environment.hpp"
#include "Execution.hpp"
#include "Utilities.hpp"
#include "Scheduler.hpp"
#include "Hook.hpp"


long exception_filter(PEXCEPTION_POINTERS pExceptionPointers) {
    auto pContext = pExceptionPointers->ContextRecord;
    printf("\r\n-- WARNING: Exception handler caught an exception\r\n");

    if (pExceptionPointers->ExceptionRecord->ExceptionCode == EXCEPTION_ACCESS_VIOLATION) {
        printf("Exception Identified: EXCEPTION_ACCESS_VIOLATION\r\n");
    }

    printf("Exception Caught         @ %p\r\n", pExceptionPointers->ContextRecord->Rip);
    printf("Module.dll               @ %p\r\n",
           reinterpret_cast<std::uintptr_t>(GetModuleHandleA("Module.dll")));
    printf("Rebased Module           @ 0x%p\r\n", pExceptionPointers->ContextRecord->Rip -
                                                  reinterpret_cast<std::uintptr_t>(GetModuleHandleA("Module.dll")));
    printf("RobloxStudioBeta.exe     @ %p\r\n",
           reinterpret_cast<std::uintptr_t>(GetModuleHandleA("RobloxStudioBeta.exe")));

    printf("Rebased Studio           @ 0x%p\r\n",
           pContext->Rip -
           reinterpret_cast<std::uintptr_t>(GetModuleHandleA("RobloxStudioBeta.exe")));

    printf("-- START REGISTERS STATE --\r\n\r\n");

    printf("-- START GP REGISTERS --\r\n");

    printf("RAX: 0x%p\r\n", pContext->Rax);
    printf("RBX: 0x%p\r\n", pContext->Rbx);
    printf("RCX: 0x%p\r\n", pContext->Rcx);
    printf("RDX: 0x%p\r\n", pContext->Rdx);
    printf("RDI: 0x%p\r\n", pContext->Rdi);
    printf("RSI: 0x%p\r\n", pContext->Rsi);
    printf("-- R8 - R15 --\r\n");
    printf("R08: 0x%p\r\n", pContext->R8);
    printf("R09: 0x%p\r\n", pContext->R9);
    printf("R10: 0x%p\r\n", pContext->R10);
    printf("R11: 0x%p\r\n", pContext->R11);
    printf("R12: 0x%p\r\n", pContext->R12);
    printf("R13: 0x%p\r\n", pContext->R13);
    printf("R14: 0x%p\r\n", pContext->R14);
    printf("R15: 0x%p\r\n", pContext->R15);
    printf("-- END GP REGISTERS --\r\n\r\n");

    printf("-- START STACK POINTERS --\r\n");
    printf("RBP: 0x%p\r\n", pContext->Rbp);
    printf("RSP: 0x%p\r\n", pContext->Rsp);
    printf("-- END STACK POINTERS --\r\n\r\n");

    printf("-- END REGISTERS STATE --\r\n\r\n");


    printf("    -- Stack Trace:\r\n");
    SymInitialize(GetCurrentProcess(), nullptr, TRUE);

    void *stack[256];
    unsigned short frameCount = RtlCaptureStackBackTrace(0, 100, stack, nullptr);

    for (unsigned short i = 0; i < frameCount; ++i) {
        auto address = reinterpret_cast<DWORD64>(stack[i]);
        char symbolBuffer[sizeof(SYMBOL_INFO) + MAX_SYM_NAME * sizeof(TCHAR)];
        auto *symbol = reinterpret_cast<SYMBOL_INFO *>(symbolBuffer);
        symbol->SizeOfStruct = sizeof(SYMBOL_INFO);
        symbol->MaxNameLen = MAX_SYM_NAME;

        DWORD value{};
        DWORD *pValue = &value;
        if (SymFromAddr(GetCurrentProcess(), address, nullptr, symbol) && (*pValue = symbol->Address - address) &&
            SymFromAddr(GetCurrentProcess(),
                        address,
                        reinterpret_cast<PDWORD64>(pValue),
                        symbol)) {
            printf("[Stack Frame %d] Inside %s @ 0x%p; Studio Rebase: 0x%p\r\n", i, symbol->Name, address,
                   address - reinterpret_cast<std::uintptr_t>(GetModuleHandleA("RobloxStudioBeta.exe")) + 0x140000000);
        } else {
            printf("[Stack Frame %d] Unknown Subroutine @ 0x%p; Studio Rebase: 0x%p\r\n", i, symbol->Name, address,
                   address - reinterpret_cast<std::uintptr_t>(GetModuleHandleA("RobloxStudioBeta.exe")) + 0x140000000);
        }
    }
    std::cout << std::endl;
    std::stringstream sstream{};
    for (unsigned short i = 0; i < frameCount; ++i) {
        sstream << "0x" << std::hex << reinterpret_cast<std::uintptr_t>(stack[i]);
        if (i < frameCount) {
            sstream << " -> ";
        } else {
            sstream << "\r\n";
        }
    }

    std::cout << sstream.str();

    // Clean up
    SymCleanup(GetCurrentProcess());
    MessageBoxA(nullptr, "ERROR", "ERROR. LOOK AT CLI.", MB_OK);
    printf("Stack frames captured - Waiting for 30s before exiting... \r\n");
    Sleep(30000);
    exit(-1);
    return EXCEPTION_EXECUTE_HANDLER;
}

int main(int argc, char **argv, char **envp) {
    // TODO: Fix Garbage Collection causing crashes.
    // FIXME: Avoid hooking to fix it, but seems not possible due to the fact it may be caused by an Engine-level bug.

    SetUnhandledExceptionFilter(exception_filter);
    AllocConsole();
    freopen_s(reinterpret_cast<FILE **>(stdin), ("CONOUT$"), "w", stdout);
    freopen_s(reinterpret_cast<FILE **>(stdin), ("CONIN$"), "r", stdin);

    wprintf(oxorany(L"[main] Initializing hook...\r\n"));
    auto hook{Hook::get_singleton()};

    wprintf(L"Attached to RBX::Studio::Lua::freeblock for call instrumentation and for anti-crashing.\r\n");
    hook->initialize();
    hook->install_additional_hooks();
    hook->install_hook();
    hook->wait_until_initialised();
    hook->remove_hook();


    wprintf(oxorany(L"[main] Hook initialized. State grabbed.\r\n"));

    wprintf(oxorany(L"[main] Initializing environment.\r\n"));
    auto scheduler{Scheduler::GetSingleton()};
    auto environmentSingleton = Environment::GetSingleton();
    environmentSingleton->Register(scheduler->get_global_executor_state(), true);
    std::string str{};

    while (true) {
        str.clear();
        wprintf(L"\r\n[main] Input lua code: ");
        getline(std::cin, str);
        if (strcmp(str.c_str(), "reinit()") == 0) {
            wprintf(L"Detected reinitialization request! Re-Initializing...\r\n");
            printf("Running re-init...\r\n");
            auto hook = Hook::get_singleton();
            auto scheduler = Scheduler::GetSingleton();
            auto environment = Environment::GetSingleton();

            scheduler->ReInitialize();
            hook->install_hook();
            hook->wait_until_initialised();
            hook->remove_hook();

            environment->Register(scheduler->get_global_executor_state(), true);

            MessageBoxA(nullptr,
                        "Obtained new lua_State. Scheduler re-initialized and Environment re-registered. Enjoy!",
                        "Reinitialization Completed", MB_OK);
            continue;
        }
        wprintf(oxorany(L"\r\nPushing to StudioExecutor::Scheduler...\r\n"));
        scheduler->ScheduleJob(str);
        Sleep(2000);
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