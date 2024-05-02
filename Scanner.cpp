//
// Created by Dottik on 1/5/2024.
//
#include "Scanner.hpp"
#include <StudioOffsets.h>
#include <TBS.hpp>
#include <cstdint>
#include <cstdlib>
#include <string>

Scanner *Scanner::m_pSingleton = nullptr;

Scanner *Scanner::get_singleton() {
    if (m_pSingleton == nullptr)
        m_pSingleton = new Scanner();
    return m_pSingleton;
}

void Scanner::scan_for_addresses() {
    TBS::State<> scannerState;
    wprintf(L"Beginning scans...\r\n");
    wprintf(L"Loading AOBs into scanner...\r\n");
    scannerState.AddPattern(
            scannerState.PatternBuilder()
                    .setUID("luau_execute")
                    .setPattern("80 79 06 00 0F 85 ? ? ? ? E9 ? ? ? ?")
                    .setScanStart(reinterpret_cast<std::uintptr_t>(GetModuleHandleA("RobloxStudioBeta.exe")))
                    .setScanEnd(reinterpret_cast<std::uintptr_t>(GetModuleHandleA("RobloxStudioBeta.exe")) +
                                0x8f57000) // Roblox Studio ImageBase + ImageSize + 0x10000 (Just for good measure)
                    .Build());

    scannerState.AddPattern(
            scannerState.PatternBuilder()
                    .setUID("pseudo2addr")
                    .setPattern("0F 84 88 00 00 00 81 FA EF D8 FF FF 74 45 81 FA F0 D8 FF FF 74 32 48 8B 41 ? 44 2B CA "
                                "48 8B 48 ? 4C 8B 01 41 0F B6 40 ? 44 3B C8 7F 12 41 8D 41 ? 48 98 48 83 C0 ? 48 C1 E0 "
                                "? 49 03 C0 C3")
                    .setScanStart(reinterpret_cast<std::uintptr_t>(GetModuleHandleA("RobloxStudioBeta.exe")))
                    .setScanEnd(reinterpret_cast<std::uintptr_t>(GetModuleHandleA("RobloxStudioBeta.exe")) +
                                0x8f57000) // Roblox Studio ImageBase + ImageSize + 0x10000 (Just for good measure)
                    .Build());

    scannerState.AddPattern(
            scannerState.PatternBuilder()
                    .setUID("freeblock")
                    .setPattern(
                            "4C 8B 51 ? 49 83 E8 ? 44 8B CA 4C 8B D9 49 8B 10 48 83 7A 28 00 75 22 83 7A 30 00 7D 1C "
                            "49 63 C1 49 8D 0C C2 49 8B 44 C2 ? 48 89 42 ? 48 85 C0 74 03 48 89 10 48 89 51 ? 48 8B 42 "
                            "? 49 89 00 83 6A ? ? 4C 89 42 ? 75 4B 48 8B 4A ? 48 85 C9 74 06 48 8B 02 48 89 01 48 8B "
                            "0A 48 85 C9 74 0A 48 8B 42 ? 48 89 41 ? EB 17 41 0F B6 C1 49 39 54 C2 60 49 8D 0C C2 75 "
                            "08 48 8B 42 ? 48 89 41 ? 49 8B 43 ? 45 33 C9 4C 63 42 ? 48 8B 48 ? 48 FF 60 ?")
                    .setScanStart(reinterpret_cast<std::uintptr_t>(GetModuleHandleA("RobloxStudioBeta.exe")))
                    .setScanEnd(reinterpret_cast<std::uintptr_t>(GetModuleHandleA("RobloxStudioBeta.exe")) +
                                0x8f57000) // Roblox Studio ImageBase + ImageSize + 0x10000 (Just for good measure)
                    .Build());


    scannerState.AddPattern(
            scannerState.PatternBuilder()
                    .setUID("luaC_step")
                    .setPattern(
                            "48 8B 59 ? B8 ? ? ? ? 0F B6 F2 0F 29 74 24 ? 4C 8B F1 44 8B 43 ? 44 0F AF 43 ? 48 8B 6B ? "
                            "48 2B 6B ? 41 F7 E8 8B FA C1 FF ? 8B C7 C1 E8 ? 03 F8 48 8B 83 ? ? ? ? 48 85 C0 74 04")
                    .setScanStart(reinterpret_cast<std::uintptr_t>(GetModuleHandleA("RobloxStudioBeta.exe")))
                    .setScanEnd(reinterpret_cast<std::uintptr_t>(GetModuleHandleA("RobloxStudioBeta.exe")) +
                                0x8f57000) // Roblox Studio ImageBase + ImageSize + 0x10000 (Just for good measure)
                    .Build());

    scannerState.AddPattern(
            scannerState.PatternBuilder()
                    .setUID("lua_newthread")
                    .setPattern("48 8B 51 ? 48 8B D9 48 8B 42 ? 48 39 42 48 72 07 B2 ? E8 ? ? ? ? F6 43 01 04 74 0F 4C "
                                "8D 43 ? 48 8B D3 48 8B CB E8 ? ? ? ? 48 8B CB E8 ? ? ? ? 48 8B 4B ? 48 8B F8 48 89 01 "
                                "C7 41 ? ? ? ? ? 48 83 43 08 ? 48 8B 4B ? 48 8B 81 ? ? ? ? 48 85 C0 74 08 48 8B D7 48 "
                                "8B CB FF D0 48 8B 5C 24 ? 48 8B C7 48 83 C4 ? 5F C3")
                    .setScanStart(reinterpret_cast<std::uintptr_t>(GetModuleHandleA("RobloxStudioBeta.exe")))
                    .setScanEnd(reinterpret_cast<std::uintptr_t>(GetModuleHandleA("RobloxStudioBeta.exe")) +
                                0x8f57000) // Roblox Studio ImageBase + ImageSize + 0x10000 (Just for good measure)
                    .Build());

    scannerState.AddPattern(
            scannerState.PatternBuilder()
                    .setUID("luaE_newthread")
                    .setPattern("E8 ? ? ? ? 48 8B 57 ? 48 8B D8 44 0F B6 42 ? C6 00 ? 41 80 E0 ? 44 88 40 ? 0F B6 57 ? 88 50 ? 48 8B D7 48 8B 4F ? 48 89 48 ? 33 C0 89 43 ? 48 8B CB 48 89 43 ? 48 89 43 ? 48 89 43 ? 48 89 43 ? 48 89 43 ? 48 89 43 ? 48 89 43 ? 48 89 43 ? 48 89 43 ? 0F B6 47 ? 88 43 ? E8 ? ? ? ? 48 8B 47 ? 48 89 43 ? 0F B6 47 ? 88 43 ?")
                    .setScanStart(reinterpret_cast<std::uintptr_t>(GetModuleHandleA("RobloxStudioBeta.exe")))
                    .setScanEnd(reinterpret_cast<std::uintptr_t>(GetModuleHandleA("RobloxStudioBeta.exe")) +
                                0x8f57000) // Roblox Studio ImageBase + ImageSize + 0x10000 (Just for good measure)
                    .Build());
}
