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
        .AddTransformer([](TBS::Pattern::Description& desc, TBS::Pattern::Result result) -> TBS::Pattern::Result {
            RBX::Studio::Offsets::luau_execute = result;

            return result;
            })
        .Build());

    scannerState.AddPattern(
        scannerState.PatternBuilder()
        .setUID("pseudo2addr")
        .setPattern("41 B9 EE D8 FF FF 4C 8B C1 41 3B D1 0F 84 88 00 00 00 81 FA EF D8 FF FF 74 45 81 FA F0 D8 FF FF 74 32 48 8B 41 ? 44 2B CA 48 8B 48 ? 4C 8B 01 41 0F B6 40 ? 44 3B C8 7F 12 41 8D 41 ? 48 98 48 83 C0 ? 48 C1 E0 ? 49 03 C0 C3 48 8D 05 ? ? ? ? C3 48 8B 41 ? 48 05 ? ? ? ? C3 48 8B 41 ? 48 8B 51 ? 48 3B 41 40 75 06 48 8B 41 ? EB 0B 48 8B 40 ? 48 8B 08 48 8B 41 ? 48 89 82 ? ? ? ? C7 82 ? ? ? ? ? ? ? ? 49 8B 40 ? 48 05 ? ? ? ? C3 49 8B 40 ? 48 8B 49 ? 48 89 81 ? ? ? ? C7 81 ? ? ? ? ? ? ? ? 49 8B 40 ? 48 05 ? ? ? ? C3")
        .setScanStart(reinterpret_cast<std::uintptr_t>(GetModuleHandleA("RobloxStudioBeta.exe")))
        .setScanEnd(reinterpret_cast<std::uintptr_t>(GetModuleHandleA("RobloxStudioBeta.exe")) +
            0x8f57000) // Roblox Studio ImageBase + ImageSize + 0x10000 (Just for good measure)
        .AddTransformer([](TBS::Pattern::Description& desc, TBS::Pattern::Result result) -> TBS::Pattern::Result {
            RBX::Studio::Offsets::pseudo2addr = result;

            return result;
            })
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
        .AddTransformer([](TBS::Pattern::Description& desc, TBS::Pattern::Result result) -> TBS::Pattern::Result {
            RBX::Studio::Offsets::rFreeBlock = result;

            return result;
            })
        .Build());


    scannerState.AddPattern(
        scannerState.PatternBuilder()
        .setUID("luaC_step")
        .setPattern("48 89 5C 24 ? 48 89 6C 24 ? 48 89 74 24 ? 57 41 56 41 57 48 83 EC ? 48 8B 59 ? B8 ? ? ? ? 0F B6 F2 0F 29 74 24 ? 4C 8B F1 44 8B 43 ? 44 0F AF 43 ? 48 8B 6B ? 48 2B 6B ? 41 F7 E8 8B FA C1 FF ? 8B C7 C1 E8 ? 03 F8 48 8B 83 ? ? ? ? 48 85 C0 74 04 33 D2 FF D0 0F B6 43 ? 84 C0 75 32 E8 ? ? ? ? 0F B6 43 ? F2 0F 11 83 ? ? ? ? 84 C0 75 1D E8 ? ? ? ? F2 0F 11 83 ? ? ? ? F2 0F 5C 83 ? ? ? ? F2 0F 11 83 ? ? ? ? E8 ? ? ? ? 44 0F B6 7B ? 49 8B CE 48 63 D7 0F 28 F0 E8 ? ? ? ? 48 8B F8 E8 ? ? ? ? 0F 28 C8 41 8B D7 F2 0F 5C CE 45 85 FF 0F 84 99 00 00 ? 83 EA ? 74 60 83 EA ? 74 5B 83 EA ? 74 41 83 FA 01 0F 85 87 00 00 00 48 01 BB ? ? ? ? 0F 28 C1 F2 0F 58 83 ? ? ? ? F2 0F 11 83 ? ? ? ? 40 84 F6 0F 84 82 00 00 ? 0F 28 C1 F2 0F 58 83 ? ? ? ? F2 0F 11 83 ? ? ? ? EB 54 0F 28 C1 F2 0F 58 83 ? ? ? ? F2 0F 11 83 ? ? ? ? EB 3A 48 01 BB ? ? ? ? 0F 28 C1 F2 0F 58 83 ? ? ? ? F2 0F 11 83 ? ? ? ? 40 84 F6 74 39 0F 28 C1 F2 0F 58 83 ? ? ? ? F2 0F 11 83 ? ? ? ? EB 0B 80 7B 21 01 74 CD 40 84 F6 74 19 F2 0F 58 8B ? ? ? ? 48 01 BB ? ? ? ? F2 0F 11 8B ? ? ? ? EB 17 F2 0F 58 8B ? ? ? ? 48 01 BB ? ? ? ? F2 0F 11 8B ? ? ? ? 48 63 4B ? 33 D2 48 6B C7 ? 48 F7 F1 80 7B 21 00 48 8B 4B ? 48 8B F8 0F 85 45 01 00 00 F2 0F 10 93 ? ? ? ? 48 B8 15 AE 47 E1 7A 14 AE 47 4C 63 43 ? 0F 28 CA F2 0F 5C 8B ? ? ? ? F2 0F 10 05 ? ? ? ? 48 F7 E1 48 2B CA 48 D1 E9 48 03 CA 48 C1 E9 ? 4C 0F AF C1 66 0F 2F C1 76 08 49 8B C0 E9 CD 00 00 00 4C 8B 8B ? ? ? ? 0F 57 C0 49 8B C9 48 2B 8B ? ? ? ? 78 07 F2 48 0F 2A C1 EB 15 48 8B C1 83 E1 ? 48 D1 E8 48 0B C1 F2 48 0F 2A C0 F2 0F 58 C0 8B 83 ? ? ? ? 4C 2B 8B ? ? ? ? 83 E0 ? F2 0F 5C 93 ? ? ? ? F2 0F 5E C1 49 C1 E9 ? 8B 8C 83 ? ? ? ? 44 89 8C 83 ? ? ? ? 41 8B C1 FF 83 ? ? ? ? 2B C1 01 83 ? ? ? ? 49 8B C8 66 41 0F 6E C9 F2 0F 59 C2 F3 0F E6 C9 F2 48 0F 2C D0 F2 0F 59 0D ? ? ? ? 66 0F 6E 83 ? ? ? ? F3 0F E6 C0 F2 0F 59 05 ? ? ? ? F2 0F 58 C8 F2 0F 59 0D ? ? ? ? F2 48 0F 2C C1 48 2B C8 48 8B 43 ? 48 2B CA 48 3B C8 7C 0A 49 3B C8 48 8B C1 49 0F 4F C0 48 89 43 ? 4C 89 83 ? ? ? ? E8 ? ? ? ? 48 8B 43 ? 48 8B CB 48 89 83 ? ? ? ? F2 0F 11 83 ? ? ? ? E8 ? ? ? ? EB 13 48 03 C1 48 89 43 ? 48 3B C5 72 07 48 2B C5 48 89 43 ? 48 8B 83 ? ? ? ? 48 85 C0 74 08 41 8B D7 49 8B CE FF D0 48 8B 5C 24 ? 48 8B C7 48 8B 6C 24 ? 48 8B 74 24 ? 0F 28 74 24 ? 48 83 C4 ? 41 5F 41 5E 5F C3")
        .setScanStart(reinterpret_cast<std::uintptr_t>(GetModuleHandleA("RobloxStudioBeta.exe")))
        .setScanEnd(reinterpret_cast<std::uintptr_t>(GetModuleHandleA("RobloxStudioBeta.exe")) +
            0x8f57000) // Roblox Studio ImageBase + ImageSize + 0x10000 (Just for good measure)
        .AddTransformer([](TBS::Pattern::Description& desc, TBS::Pattern::Result result) -> TBS::Pattern::Result {
            RBX::Studio::Offsets::rLuaC_Step = result;

            return result;
            })
        .Build());


    scannerState.AddPattern(
        scannerState.PatternBuilder()
        .setUID("lua_newthread")
        .setPattern("48 89 5C 24 ? 57 48 83 EC ? 48 8B 51 ? 48 8B D9 48 8B 42 ? 48 39 42 48 72 07 B2 ? E8 ? ? ? ? F6 43 01 04 74 0F 4C 8D 43 ? 48 8B D3 48 8B CB E8 ? ? ? ? 48 8B CB E8 ? ? ? ? 48 8B 4B ? 48 8B F8 48 89 01 C7 41 ? ? ? ? ? 48 83 43 08 ? 48 8B 4B ? 48 8B 81 ? ? ? ? 48 85 C0 74 08 48 8B D7 48 8B CB FF D0 48 8B 5C 24 ? 48 8B C7 48 83 C4 ? 5F C3")
        .setScanStart(reinterpret_cast<std::uintptr_t>(GetModuleHandleA("RobloxStudioBeta.exe")))
        .setScanEnd(reinterpret_cast<std::uintptr_t>(GetModuleHandleA("RobloxStudioBeta.exe")) +
            0x8f57000) // Roblox Studio ImageBase + ImageSize + 0x10000 (Just for good measure)
        .AddTransformer([](TBS::Pattern::Description& desc, TBS::Pattern::Result result) -> TBS::Pattern::Result {
            RBX::Studio::Offsets::rlua_newthread = result;
            return result;
            })
        .Build());

    scannerState.AddPattern(
        scannerState.PatternBuilder()
        .setUID("luaE_newthread")
        .setPattern("48 89 5C 24 ? 57 48 83 EC ? 44 0F B6 41 ? BA ? ? ? ? 48 8B F9 E8 ? ? ? ? 48 8B 57 ? 48 8B D8 44 0F B6 42 ? C6 00 ? 41 80 E0 ? 44 88 40 ? 0F B6 57 ? 88 50 ? 48 8B D7 48 8B 4F ? 48 89 48 ? 33 C0 89 43 ? 48 8B CB 48 89 43 ? 48 89 43 ? 48 89 43 ? 48 89 43 ? 48 89 43 ? 48 89 43 ? 48 89 43 ? 48 89 43 ? 48 89 43 ? 0F B6 47 ? 88 43 ? E8 ? ? ? ? 48 8B 47 ? 48 89 43 ? 0F B6 47 ? 88 43 ? 48 8B C3 48 8B 5C 24 ? 48 83 C4 ? 5F C3")
        .setScanStart(reinterpret_cast<std::uintptr_t>(GetModuleHandleA("RobloxStudioBeta.exe")))
        .setScanEnd(reinterpret_cast<std::uintptr_t>(GetModuleHandleA("RobloxStudioBeta.exe")) +
            0x8f57000) // Roblox Studio ImageBase + ImageSize + 0x10000 (Just for good measure)
        .AddTransformer([](TBS::Pattern::Description& desc, TBS::Pattern::Result result) -> TBS::Pattern::Result {
            RBX::Studio::Offsets::rluaE_newthread = result;
            return result;
            })
        .Build());

    scannerState.AddPattern(
        scannerState.PatternBuilder()
        .setUID("fireproximityprompt")
        .setPattern("48 89 5C 24 ? 55 56 57 41 54 41 55 41 56 41 57 48 8D 6C 24 ? 48 81 EC ? ? ? ? 4C 8B F9 E8 ? ? ? ? 48 8B F8 48 85 C0 0F 84 C6 02 00 ? 48 8B 50 ? 48 85 D2 0F 84 D4 02 00 ? 8B 42 ? 85 C0 0F 84 C9 02 00 ? 0F 1F 84 00 00 00 00 00 8D 48 ? F0 0F B1 4A ? 74 0A 85 C0 0F 84 AF 02 00 ? EB EC 4C 8B 6F ? 48 8B 7F ? 4C 89 6D ? 48 89 7D ? 0F 57 C0 F3 0F 7F 45 ? 48 85 FF 74 04 F0 FF 47 ? 4C 89 6D ? 48 89 7D ? 4C 8D 45 ? 49 8B D7 48 8B 0D ? ? ? ? E8 ? ? ? ? 0F 57 C0 F3 0F 7F 45 ? 48 85 FF 74 04 F0 FF 47 ? 48 8D 45 ? 48 89 45 ? 48 C7 45 ? ? ? ? ? C7 45 ? ? ? ? ? 4C 89 6D ? 48 89 7D ? F3 0F 7F 45 ? 4C 8D 4D ? 4C 8D 45 ? 49 8B D7 48 8B 0D ? ? ? ? E8 ? ? ? ? 90 41 BC FF FF FF FF 48 8B 5D ? 48 85 DB 74 2B 41 8B C4 F0 0F C1 43 ? 83 F8 01 75 1E 48 8B 03 48 8B CB FF 10 41 8B C4 F0 0F C1 43 ? 83 F8 01 75 09 48 8B 03 48 8B CB FF 50 ? 4D 85 FF 0F 84 C9 00 00 ? 49 8B 77 ? 48 85 F6 74 13 48 8B 4E ? 48 85 C9 74 0D E8 ? ? ? ? 48 8B F0 EB 03 49 8B F7 48 85 F6 0F 84 A1 00 00 ? 48 8B 5E ? E8 ? ? ? ? 0F B7 93 ? ? ? ? 0F B7 88 ? ? ? ? 2B D1 0F B7 80 ? ? ? ? 3B D0 77 7D 48 8B CE E8 ? ? ? ? 4C 8B C8 48 85 C0 74 6D 48 8D 45 ? 48 89 45 ? 0F 57 C0 F3 0F 7F 45 ? 48 85 FF 74 04 F0 FF 47 ? 4C 89 6D ? 48 89 7D ? 49 8B 57 ? 48 85 D2 0F 84 5D 01 00 ? 8B 42 ? 85 C0 0F 84 52 01 00 ? 8D 48 ? F0 0F B1 4A ? 74 0A 85 C0 0F 84 40 01 00 ? EB EC 49 8B 47 ? 49 8B 4F ? 48 89 45 ? 48 89 4D ? 4C 8D 45 ? 48 8D 55 ? 49 8B C9 E8 ? ? ? ? 80 3D 08 08 57 05 00 0F ? ? ? ? ? E8 ? ? ? ? 33 D2 F7 35 ? ? ? ? 85 D2 0F 85 AC 00 00 ? 48 C7 45 C7 ? ? ? ? 48 C7 45 D7 ? ? ? ? 48 C7 45 DF ? ? ? ? 88 55 ? 8D 4A ? E8 ? ? ? ? 48 C7 45 D7 ? ? ? ? 48 C7 45 DF ? ? ? ? 0F 10 05 ? ? ? ? 0F 11 00 F2 0F 10 0D ? ? ? ? F2 0F 11 48 ? 0F B6 0D ? ? ? ? 88 48 ? C6 40 ? ? 48 89 45 ? 45 33 C0 41 8D 50 ? 48 8D 4D ? E8 ? ? ? ? 90 48 8B 55 ? 48 83 FA 10 72 35 48 FF C2 48 8B 4D ? 48 8B C1 48 81 FA 00 10 00 00 72 1C 48 83 C2 ? 48 8B 49 ? 48 2B C1 48 83 C0 ? 48 83 F8 1F 76 07 FF 15 ? ? ? ?")
        .setScanStart(reinterpret_cast<std::uintptr_t>(GetModuleHandleA("RobloxStudioBeta.exe")))
        .setScanEnd(reinterpret_cast<std::uintptr_t>(GetModuleHandleA("RobloxStudioBeta.exe")) +
            0x8f57000) // Roblox Studio ImageBase + ImageSize + 0x10000 (Just for good measure)
        .AddTransformer([](TBS::Pattern::Description& desc, TBS::Pattern::Result result) -> TBS::Pattern::Result {
            RBX::Studio::Offsets::fireproximityprompt = result;
            return result;
            })
        .Build());

    scannerState.AddPattern(
        scannerState.PatternBuilder()
        .setUID("pushinstance")
        .setPattern("48 89 4C 24 ? 55 53 56 57 41 54 41 55 41 56 41 57 48 8D 6C 24 ? 48 81 EC ? ? ? ? 4C 8B E1 BA ? ? ? ? E8 ? ? ? ? 48 8B F8 48 89 45 ? 48 8B C8 E8 ? ? ? ? 48 8B D8 49 8B CC E8 ? ? ? ? 4C 8B F8 48 8B CB E8 ? ? ? ? 48 8B D8 41 BD FF FF FF FF 48 85 C0 74 72 48 8B 00 48 8B CB FF 50 ? 84 C0 74 65 48 8B 03 45 8D 45 ? 48 8D 55 ? 48 8B CB FF 10 48 8B C8 48 8B 30 48 8B 40 ? 48 85 C0 74 08 F0 FF 40 ? 48 8B 41 ? 48 89 75 ? 48 89 45 ? 48 8B 5D ? 48 85 DB 74 39 41 8B C5 F0 0F C1 43 ? 83 F8 01 75 28 48 8B 03 48 8B CB FF 10 41 8B C5 F0 0F C1 43 ? 83 F8 01 75 13 48 8B 03 48 8B CB FF 50 ? EB 08 0F 57 C9 F3 0F 7F 4D ? 48 8B 75 ? 48 85 F6 0F 84 95 04 00 ? 45 33 F6 4D 85 E4")
        .setScanStart(reinterpret_cast<std::uintptr_t>(GetModuleHandleA("RobloxStudioBeta.exe")))
        .setScanEnd(reinterpret_cast<std::uintptr_t>(GetModuleHandleA("RobloxStudioBeta.exe")) +
            0x8f57000) // Roblox Studio ImageBase + ImageSize + 0x10000 (Just for good measure)
        .AddTransformer([](TBS::Pattern::Description& desc, TBS::Pattern::Result result) -> TBS::Pattern::Result {
            RBX::Studio::Offsets::pushinstance = result;
            return result;
            })
        .Build());

    scannerState.AddPattern(
        scannerState.PatternBuilder()
        .setUID("rFromLuaState")
        .setPattern("48 89 5C 24 ? 55 56 57 48 83 EC ? 48 8B FA 48 8B E9 8B 0D ? ? ? ? E8 ? ? ? ? 8B D8 89 44 24 ? 48 85 ED 74 4A 48 8B CD E8 ? ? ? ? 48 8B F0 B9 ? ? ? ? E8 ? ? ? ? 48 89 44 24 ? 48 85 C0 74 10 48 8B D6 48 8B C8 E8 ? ? ? ? 48 8B F0 EB 02 33 F6 48 8B CD E8 ? ? ? ? 48 89 46 ? 48 8B D6 48 8B CF E8 ? ? ? ? EB 2B 48 85 FF 74 26 48 8B CF E8 ? ? ? ? 48 8B F8 48 85 C0 74 16 48 8B C8 E8 ? ? ? ? BA ? ? ? ? 48 8B CF E8 ? ? ? ? 90 8B CB E8 ? ? ? ? 90 48 8B 5C 24 ? 48 83 C4 ? 5F 5E 5D C3")
        .setScanStart(reinterpret_cast<std::uintptr_t>(GetModuleHandleA("RobloxStudioBeta.exe")))
        .setScanEnd(reinterpret_cast<std::uintptr_t>(GetModuleHandleA("RobloxStudioBeta.exe")) +
            0x8f57000) // Roblox Studio ImageBase + ImageSize + 0x10000 (Just for good measure)
        .AddTransformer([](TBS::Pattern::Description& desc, TBS::Pattern::Result result) -> TBS::Pattern::Result {
            RBX::Studio::Offsets::rFromLuaState = result;
            return result;
            })
        .Build());

    scannerState.AddPattern(
        scannerState.PatternBuilder()
        .setUID("luaD_rawunprotected")
        .setPattern("48 89 4C 24 ? 48 83 EC ? 48 8B C2 49 8B D0 FF D0 33 C0 EB 04")
        .setScanStart(reinterpret_cast<std::uintptr_t>(GetModuleHandleA("RobloxStudioBeta.exe")))
        .setScanEnd(reinterpret_cast<std::uintptr_t>(GetModuleHandleA("RobloxStudioBeta.exe")) +
            0x8f57000) // Roblox Studio ImageBase + ImageSize + 0x10000 (Just for good measure)
        .AddTransformer([](TBS::Pattern::Description& desc, TBS::Pattern::Result result) -> TBS::Pattern::Result {
            RBX::Studio::Offsets::rLuaD_rawrununprotected = result;
            return result;
            })
        .Build());


    TBS::Scan(scannerState);

    printf("[main] Finished Scan\r\n");
}
