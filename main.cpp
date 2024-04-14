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
#include "RBXScheduler.h"


long exception_filter(PEXCEPTION_POINTERS pExceptionPointers) {
    auto pContext = pExceptionPointers->ContextRecord;
    wprintf(oxorany(L"\r\n-- WARNING: Exception handler caught an exception\r\n"));

    if (pExceptionPointers->ExceptionRecord->ExceptionCode == EXCEPTION_ACCESS_VIOLATION) {
        wprintf(oxorany(L"Exception Identified: EXCEPTION_ACCESS_VIOLATION\r\n"));
    }

    wprintf(oxorany(L"Exception Caught         @ %p\r\n"), pExceptionPointers->ContextRecord->Rip);
    wprintf(oxorany(L"Module.dll               @ %p\r\n"),
            reinterpret_cast<std::uintptr_t>(GetModuleHandleA(oxorany_pchar(L"Module.dll"))));
    wprintf(oxorany(L"Rebased Module           @ 0x%p\r\n"), pExceptionPointers->ContextRecord->Rip -
                                                             reinterpret_cast<std::uintptr_t>(GetModuleHandleA(
                                                                     oxorany_pchar(L"Module.dll"))));
    wprintf(oxorany(L"RobloxStudioBeta.exe     @ %p\r\n"),
            reinterpret_cast<std::uintptr_t>(GetModuleHandleA(oxorany_pchar(L"RobloxStudioBeta.exe"))));

    wprintf(oxorany(L"Rebased Studio           @ 0x%p\r\n"),
            pContext->Rip -
            reinterpret_cast<std::uintptr_t>(GetModuleHandleA(oxorany_pchar(L"RobloxStudioBeta.exe"))));

    wprintf(oxorany(L"-- START REGISTERS STATE --\r\n\r\n"));

    wprintf(oxorany(L"-- START GP REGISTERS --\r\n"));

    wprintf(oxorany(L"RAX: 0x%p\r\n"), pContext->Rax);
    wprintf(oxorany(L"RBX: 0x%p\r\n"), pContext->Rbx);
    wprintf(oxorany(L"RCX: 0x%p\r\n"), pContext->Rcx);
    wprintf(oxorany(L"RDX: 0x%p\r\n"), pContext->Rdx);
    wprintf(oxorany(L"RDI: 0x%p\r\n"), pContext->Rdi);
    wprintf(oxorany(L"RSI: 0x%p\r\n"), pContext->Rsi);
    wprintf(oxorany(L"-- R8 - R15 --\r\n"));
    wprintf(oxorany(L"R08: 0x%p\r\n"), pContext->R8);
    wprintf(oxorany(L"R09: 0x%p\r\n"), pContext->R9);
    wprintf(oxorany(L"R10: 0x%p\r\n"), pContext->R10);
    wprintf(oxorany(L"R11: 0x%p\r\n"), pContext->R11);
    wprintf(oxorany(L"R12: 0x%p\r\n"), pContext->R12);
    wprintf(oxorany(L"R13: 0x%p\r\n"), pContext->R13);
    wprintf(oxorany(L"R14: 0x%p\r\n"), pContext->R14);
    wprintf(oxorany(L"R15: 0x%p\r\n"), pContext->R15);
    wprintf(oxorany(L"-- END GP REGISTERS --\r\n\r\n"));

    wprintf(oxorany(L"-- START STACK POINTERS --\r\n"));
    wprintf(oxorany(L"RBP: 0x%p\r\n"), pContext->Rbp);
    wprintf(oxorany(L"RSP: 0x%p\r\n"), pContext->Rsp);
    wprintf(oxorany(L"-- END STACK POINTERS --\r\n\r\n"));

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
            printf(oxorany_pchar(L"[Stack Frame %d] Inside %s @ 0x%p; Studio Rebase: 0x%p\r\n"), i, symbol->Name,
                   address,
                   address -
                   reinterpret_cast<std::uintptr_t>(GetModuleHandleA(("RobloxStudioBeta.exe"))) +
                   0x140000000);
        } else {
            printf(oxorany_pchar(L"[Stack Frame %d] Unknown Subroutine @ 0x%p; Studio Rebase: 0x%p\r\n"), i,
                   symbol->Name,
                   address,
                   address -
                   reinterpret_cast<std::uintptr_t>(GetModuleHandleA(("RobloxStudioBeta.exe"))) +
                   0x140000000);
        }
    }
    std::cout << std::endl;
    std::stringstream sstream{};
    for (unsigned short i = oxorany(0); i < frameCount; ++i) {
        sstream << ("0x") << std::hex << reinterpret_cast<std::uintptr_t>(stack[i]);
        if (i < frameCount) {
            sstream << (" -> ");
        } else {
            sstream << ("\r\n");
        }
    }

    std::cout << sstream.str();

    // Clean up
    SymCleanup(GetCurrentProcess());
    MessageBoxA(nullptr, ("ERROR"), ("ERROR. LOOK AT CLI."), oxorany(MB_OK));
    wprintf(oxorany(L"Stack frames captured - Waiting for 30s before exiting... \r\n"));
    Sleep(oxorany(30000));
    exit(oxorany(-1));
    return EXCEPTION_EXECUTE_HANDLER;
}

int main(int argc, char **argv, char **envp) {
    // TODO: Fix Garbage Collection causing crashes.
    // FIXME: Avoid hooking to fix it, but seems not possible due to the fact it may be caused by an Engine-level bug.

    SetUnhandledExceptionFilter(exception_filter);
    AllocConsole();
    freopen_s(reinterpret_cast<FILE **>(stdin), ("CONOUT$"), ("w"), stdout);
    freopen_s(reinterpret_cast<FILE **>(stdin), ("CONIN$"), ("r"), stdin);

    wprintf(oxorany(L"[main] Initializing hook...\r\n"));
    auto hook{Hook::get_singleton()};

    hook->initialize();
    wprintf(oxorany(L"Attached to RBX::Studio::Lua::freeblock for call instrumentation and for anti-crashing.\r\n"));
    hook->install_additional_hooks();
    hook->install_hook();
    hook->wait_until_initialised();

    wprintf(oxorany(L"[main] Hook initialized. State grabbed.\r\n"));

    wprintf(oxorany(L"[main] Initializing environment.\r\n"));
    auto scheduler{Scheduler::get_singleton()};
    auto environmentSingleton = Environment::GetSingleton();
    wprintf(oxorany(L"[main] Attaching to RunService.Stepped for custom scheduler stepping.\r\n"));
    environmentSingleton->Register(scheduler->get_global_executor_state(), true);

    hook->remove_hook();
    std::string str{};

    while (oxorany(true)) {
        str.clear();
        wprintf(oxorany(L"\r\n[main] Input lua code: "));
        getline(std::cin, str);
        if (strcmp(str.c_str(), "reinit()") == 0) {
            wprintf(oxorany(L"Detected reinitialization request! Re-Initializing...\r\n"));
            wprintf(oxorany(L"Running re-init...\r\n"));
            auto hook = Hook::get_singleton();
            auto scheduler = Scheduler::get_singleton();
            auto environment = Environment::GetSingleton();

            scheduler->re_initialize();
            hook->install_hook();
            hook->wait_until_initialised();
            environment->Register(scheduler->get_global_executor_state(), true);
            wprintf(oxorany(L"Attaching to RunService.Heartbeat for custom scheduler stepping.\r\n"));
            hook->complete_initialization();
            hook->remove_hook();

            MessageBoxA(nullptr,
                        (
                                "Obtained new lua_State. Scheduler re-initialized and Environment re-registered. Enjoy!"),
                        ("Reinitialization Completed"), MB_OK);
            continue;
        }
        wprintf(oxorany(L"\r\nPushing to StudioExecutor::Scheduler...\r\n"));
        scheduler->schedule_job(str);
        Sleep(oxorany(2000));
        // scheduler->scheduler_step(scheduler->get_global_executor_state());
    }

    return 0;
}

BOOL WINAPI DllMain(
        HINSTANCE hinstDLL,  // handle to DLL module
        DWORD fdwReason,     // reason for calling function
        LPVOID lpvReserved)  // reserved
{
    if (oxorany(0) != oxorany(0)) {
        auto a = oxorany(10);
        auto b = oxorany(10);
        auto c = oxorany(10);
        auto d = oxorany(10);
        auto e = oxorany(10);
        auto f = oxorany(10);
        auto g = oxorany(10);
        auto h = oxorany(10);
        if (h == oxorany(9)) {
            b = oxorany(10);
            c = oxorany(10);
            d = oxorany(10);
            e = oxorany(10);
            f = oxorany(10);
            g = oxorany(10);
            h = oxorany(10);
            if (a == oxorany(9)) {
                b = oxorany(10);
                c = oxorany(10);
                d = oxorany(10);
                e = oxorany(10);
                f = oxorany(10);
                g = oxorany(10);
                h = oxorany(10);
                if (b == oxorany(9)) {
                    b = oxorany(10);
                    c = oxorany(10);
                    d = oxorany(10);
                    e = oxorany(10);
                    f = oxorany(10);
                    g = oxorany(10);
                    h = oxorany(10);
                    if (c == oxorany(9)) {
                        b = oxorany(10);
                        c = oxorany(10);
                        d = oxorany(10);
                        e = oxorany(10);
                        f = oxorany(10);
                        g = oxorany(10);
                        h = oxorany(10);
                        if (d == oxorany(9)) {
                            b = oxorany(10);
                            c = oxorany(10);
                            d = oxorany(10);
                            e = oxorany(10);
                            f = oxorany(10);
                            g = oxorany(10);
                            h = oxorany(10);
                            if (e == oxorany(9)) {
                                b = oxorany(10);
                                c = oxorany(10);
                                d = oxorany(10);
                                e = oxorany(10);
                                f = oxorany(10);
                                g = oxorany(10);
                                h = oxorany(10);
                                if (f == oxorany(9)) {
                                    if (g == oxorany(9)) {
                                        b = oxorany(10);
                                        c = oxorany(10);
                                        d = oxorany(10);
                                        e = oxorany(10);
                                        f = oxorany(10);
                                        g = oxorany(10);
                                        h = oxorany(10);
                                        if (h == oxorany(9)) {
                                            b = oxorany(10);
                                            c = oxorany(10);
                                            d = oxorany(10);
                                            e = oxorany(10);
                                            f = oxorany(10);
                                            g = oxorany(10);
                                            h = oxorany(10);
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    // Perform actions based on the reason for calling.
    switch (fdwReason) {
        case DLL_PROCESS_ATTACH:
            CreateThread(nullptr, oxorany(0x1000),
                         reinterpret_cast<LPTHREAD_START_ROUTINE>(main), 0, 0, nullptr);
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
    return oxorany(TRUE);  // Successful DLL_PROCESS_ATTACH.
}