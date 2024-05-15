// C
#include <MinHook.h>
#include <StudioOffsets.h>
#include <Windows.h>
// C++
#include <DbgHelp.h>
#include <Log.hpp>
#include <iostream>
#include <thread>
#include "Environment/Environment.hpp"
#include "Execution.hpp"
#include "Hook.hpp"
#include "Scheduler.hpp"
#include "Utilities.hpp"
#include "oxorany.hpp"


long exception_filter(PEXCEPTION_POINTERS pExceptionPointers) {
    const auto function_name = "ExceptionFilter::UnhandledExceptionFilter";
    const auto *pContext = pExceptionPointers->ContextRecord;
    LOG_TO_FILE_AND_CONSOLE(function_name, "WARNING: Exception caught! Exception Code: %lx",
                            pExceptionPointers->ExceptionRecord->ExceptionCode);

    LOG_TO_FILE_AND_CONSOLE(function_name, "Exception Information:");

    LOG_TO_FILE_AND_CONSOLE(function_name, "Exception Caught         @ %llx", pExceptionPointers->ContextRecord->Rip);
    LOG_TO_FILE_AND_CONSOLE(function_name, "Module.dll               @ %llx",
                            reinterpret_cast<std::uintptr_t>(GetModuleHandleA("Module.dll")));
    LOG_TO_FILE_AND_CONSOLE(function_name, "Rebased Module           @ 0x%llx",
                            pExceptionPointers->ContextRecord->Rip -
                                    reinterpret_cast<std::uintptr_t>(GetModuleHandleA("Module.dll")));
    LOG_TO_FILE_AND_CONSOLE(function_name, "RobloxStudioBeta.exe     @ %llx",
                            reinterpret_cast<std::uintptr_t>(GetModuleHandleA("RobloxStudioBeta.exe")));

    LOG_TO_FILE_AND_CONSOLE(function_name, "Rebased Studio           @ 0x%llx",
                            pContext->Rip - reinterpret_cast<std::uintptr_t>(GetModuleHandleA("RobloxStudioBeta.exe")));

    LOG_TO_FILE_AND_CONSOLE(function_name, "-- START REGISTERS STATE --\r\n");

    LOG_TO_FILE_AND_CONSOLE(function_name, "-- START GP REGISTERS --");

    LOG_TO_FILE_AND_CONSOLE(function_name, "RAX: 0x%llx", pContext->Rax);
    LOG_TO_FILE_AND_CONSOLE(function_name, "RBX: 0x%llx", pContext->Rbx);
    LOG_TO_FILE_AND_CONSOLE(function_name, "RCX: 0x%llx", pContext->Rcx);
    LOG_TO_FILE_AND_CONSOLE(function_name, "RDX: 0x%llx", pContext->Rdx);
    LOG_TO_FILE_AND_CONSOLE(function_name, "RDI: 0x%llx", pContext->Rdi);
    LOG_TO_FILE_AND_CONSOLE(function_name, "RSI: 0x%llx", pContext->Rsi);
    LOG_TO_FILE_AND_CONSOLE(function_name, "-- R8 - R15 --");
    LOG_TO_FILE_AND_CONSOLE(function_name, "R08: 0x%llx", pContext->R8);
    LOG_TO_FILE_AND_CONSOLE(function_name, "R09: 0x%llx", pContext->R9);
    LOG_TO_FILE_AND_CONSOLE(function_name, "R10: 0x%llx", pContext->R10);
    LOG_TO_FILE_AND_CONSOLE(function_name, "R11: 0x%llx", pContext->R11);
    LOG_TO_FILE_AND_CONSOLE(function_name, "R12: 0x%llx", pContext->R12);
    LOG_TO_FILE_AND_CONSOLE(function_name, "R13: 0x%llx", pContext->R13);
    LOG_TO_FILE_AND_CONSOLE(function_name, "R14: 0x%llx", pContext->R14);
    LOG_TO_FILE_AND_CONSOLE(function_name, "R15: 0x%llx", pContext->R15);
    LOG_TO_FILE_AND_CONSOLE(function_name, "-- END GP REGISTERS --\r\n");

    LOG_TO_FILE_AND_CONSOLE(function_name, "-- START STACK POINTERS --");
    LOG_TO_FILE_AND_CONSOLE(function_name, "RBP: 0x%llx", pContext->Rbp);
    LOG_TO_FILE_AND_CONSOLE(function_name, "RSP: 0x%llx", pContext->Rsp);
    LOG_TO_FILE_AND_CONSOLE(function_name, "-- END STACK POINTERS --\r\n");

    LOG_TO_FILE_AND_CONSOLE(function_name, "-- END REGISTERS STATE --\r\n");


    LOG_TO_FILE_AND_CONSOLE(function_name, "-- Stack Trace:");
    SymInitialize(GetCurrentProcess(), nullptr, TRUE);

    void *stack[256];
    const unsigned short frameCount = RtlCaptureStackBackTrace(0, 100, stack, nullptr);

    for (unsigned short i = 0; i < frameCount; ++i) {
        auto address = reinterpret_cast<DWORD64>(stack[i]);
        char symbolBuffer[sizeof(SYMBOL_INFO) + MAX_SYM_NAME * sizeof(TCHAR)];
        auto *symbol = reinterpret_cast<SYMBOL_INFO *>(symbolBuffer);
        symbol->SizeOfStruct = sizeof(SYMBOL_INFO);
        symbol->MaxNameLen = MAX_SYM_NAME;

        DWORD value{};
        DWORD *pValue = &value;
        if (SymFromAddr(GetCurrentProcess(), address, nullptr, symbol) && ((*pValue = symbol->Address - address)) &&
            SymFromAddr(GetCurrentProcess(), address, reinterpret_cast<PDWORD64>(pValue), symbol)) {
            LOG_TO_FILE_AND_CONSOLE(
                    function_name, "[Stack Frame %d] Inside %s @ 0x%llx; Studio Rebase: 0x%llx", i, symbol->Name,
                    address,
                    address - reinterpret_cast<std::uintptr_t>(GetModuleHandleA("RobloxStudioBeta.exe")) + 0x140000000);
        } else {
            LOG_TO_FILE_AND_CONSOLE(
                    function_name, "[Stack Frame %d] Unknown Subroutine @ 0x%llx; Studio Rebase: 0x%llx", i,
                    symbol->Name, address,
                    address - reinterpret_cast<std::uintptr_t>(GetModuleHandleA("RobloxStudioBeta.exe")) + 0x140000000);
        }
    }
    std::cout << std::endl;
    std::stringstream sstream{};
    for (unsigned short i = 0; i < frameCount; ++i) {
        sstream << ("0x") << std::hex << reinterpret_cast<std::uintptr_t>(stack[i]);
        if (i < frameCount) {
            sstream << (" -> ");
        } else {
            sstream << ("\r\n");
        }
    }

    LOG_TO_FILE_AND_CONSOLE(function_name, "%s", sstream.str().c_str());

    // Clean up
    SymCleanup(GetCurrentProcess());
    LOG_TO_FILE_AND_CONSOLE(
            function_name,
            "Stack frames captured. Log flushed to disk as file '%s'! Send to developer for more information! Roblox "
            "Studio will now close, this is NOT a Studio Bug! It is probably caused by RbxStu!",
            Log::get_singleton()->get_log_path().c_str());
    Log::get_singleton()->flush_to_disk();
    Sleep(30000);
    exit(-1);
    return EXCEPTION_EXECUTE_HANDLER;
}

int main(int argc, char **argv, char **envp) {
    // TODO: Fix Garbage Collection causing crashes.
    // FIXME: Avoid hooking to fix it, but seems not possible due to the fact it may be caused by an Engine-level bug.

    SetUnhandledExceptionFilter(exception_filter);
    AllocConsole();
    freopen_s(reinterpret_cast<FILE **>(stdin), ("CONOUT$"), ("w"), stdout);
    freopen_s(reinterpret_cast<FILE **>(stdin), ("CONIN$"), ("r"), stdin);

    LOG_TO_FILE_AND_CONSOLE("main", "Initializing log writer on exit");
    atexit([] {
        LOG_TO_FILE_AND_CONSOLE("main::reinit", "Flushing log to disk...");
        Log::get_singleton()->flush_to_disk();
    });
    LOG_TO_FILE_AND_CONSOLE("main", "Initializing hook...");
    const auto pHook{Hook::get_singleton()};

    pHook->initialize();
    pHook->install_additional_hooks();
    Sleep(500);
    pHook->install_hook();
    pHook->wait_until_initialised();

    LOG_TO_FILE_AND_CONSOLE("main", "Hook initialization completed. lua_State* obtained.");

    LOG_TO_FILE_AND_CONSOLE("main", "Initializing executor environment...");
    auto scheduler{Scheduler::get_singleton()};
    auto environment = Environment::get_singleton();
    auto schedulerKey = 0;
    environment->register_env(scheduler->get_global_executor_state(), true, &schedulerKey);
    pHook->remove_hook();

    std::string buffer{};

    while (oxorany(true)) {
        buffer.clear();
        printf("\r\n[main] Input lua code: ");
        getline(std::cin, buffer);
        if (strcmp(buffer.c_str(), "reinit()") == 0) {
            LOG_TO_FILE_AND_CONSOLE("main::reinit", "Re-Initializing execution environment.");

            scheduler->re_initialize();
            pHook->install_hook();
            pHook->wait_until_initialised();
            environment->register_env(scheduler->get_global_executor_state(), true, &schedulerKey);
            pHook->remove_hook();

            LOG_TO_FILE_AND_CONSOLE("main::reinit", "Re-Initialization successful!");
            continue;
        }
        printf("\r\nPushing to StudioExecutor::Scheduler...\r\n");
        scheduler->schedule_job(buffer);
        Sleep(2000);
    }

    return 0;
}

BOOL WINAPI DllMain(HINSTANCE hinstDLL, // handle to DLL module
                    const DWORD fdwReason, // reason for calling function
                    const LPVOID lpvReserved) // reserved
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
    return oxorany(TRUE); // Successful DLL_PROCESS_ATTACH.
}
