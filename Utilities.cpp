//
// Created by Dottik on 21/11/2023.
//

#include "Utilities.hpp"
#include <Windows.h>
#include <filesystem>
#include <oxorany.hpp>
#include <random>
#include <string>
#include <tlhelp32.h>


Module::Utilities *Module::Utilities::sm_pModule = nullptr;


Module::Utilities *Module::Utilities::get_singleton() {
    if (sm_pModule == nullptr)
        sm_pModule = new Utilities();

    return sm_pModule;
}

std::wstring Module::Utilities::get_random_wstring(const int length) {
    constexpr auto alphabet = (L"abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789");

    std::random_device rngDevice;
    std::mt19937 rng(rngDevice());
    std::uniform_int_distribution<int> distribution(0, lstrlenW(alphabet));

    std::wstring randomString{};

    for (int i = 0; i < length; ++i) {
        randomString += alphabet[distribution(rng)];
    }

    return randomString;
}

std::string Module::Utilities::get_random_string(const int length) {
    constexpr char alphabet[] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";

    std::random_device rngDevice;
    std::mt19937 rng(rngDevice());
    std::uniform_int_distribution<int> distribution(0, strlen(alphabet));


    std::string randomString{};

    for (int i = 0; i < length; ++i) {
        randomString += alphabet[distribution(rng)];
    }

    return randomString;
}

int Module::Utilities::get_proc_id(const char* name) {
    PROCESSENTRY32 entry;
    entry.dwSize = sizeof(PROCESSENTRY32);

    HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, NULL);

    if (Process32First(snapshot, &entry) == TRUE)
    {
        while (Process32Next(snapshot, &entry) == TRUE)
        {
            if (_stricmp(entry.szExeFile, name) == 0)
            {
                CloseHandle(snapshot);
                return entry.th32ProcessID;
            }
        }
    }

    CloseHandle(snapshot);
    return 0;
}

BOOL Module::Utilities::roblox_active() {
    int proc_id = this->get_proc_id("RobloxStudioBeta.exe");
    HWND foreground = GetForegroundWindow();

    DWORD fproc_id;
    GetWindowThreadProcessId(foreground, &fproc_id);

    return (fproc_id == proc_id);
}


/// Converts wchar_t into char. Returns heap allocated memory. YOU MUST DISPOSE!
const char *Module::Utilities::to_char(const wchar_t *szConvert) {
    const auto len = (wcslen(szConvert) + 1) * 2;
    const auto str = new char[len];
    size_t convertedChars = 0;
    wcstombs_s(&convertedChars, str, len, szConvert, _TRUNCATE);
    return str;
}

/// Converts char into wchar_t. Returns heap allocated memory. YOU MUST DISPOSE!
const wchar_t *Module::Utilities::to_wchar(const char *szConvert) {
    auto charLen = strlen(szConvert) + 1;
    auto *wText = new wchar_t[charLen];
    size_t outSize = 0;
    mbstowcs_s(&outSize, wText, charLen, szConvert, charLen - oxorany_flt(1));
    return wText;
}


std::string Module::Utilities::to_string(const std::wstring &szConvert) {
    if (szConvert.empty())
        return {};

    const int len = WideCharToMultiByte(CP_UTF8, 0, szConvert.c_str(), static_cast<int>(szConvert.size()), nullptr, 0,
                                        nullptr, nullptr);
    std::string strTo(len, 0);
    WideCharToMultiByte(CP_UTF8, 0, szConvert.c_str(), static_cast<int>(szConvert.size()),
                        const_cast<char *>(strTo.c_str()), len, nullptr, nullptr);
    return strTo;
}

std::wstring Module::Utilities::to_wstring(const std::string &szConvert) {
    if (szConvert.empty())
        return {};

    const int len = MultiByteToWideChar(CP_UTF8, 0, szConvert.c_str(), static_cast<int>(szConvert.size()), nullptr, 0);
    std::wstring nStr(len, 0);
    MultiByteToWideChar(CP_UTF8, 0, szConvert.c_str(), static_cast<int>(szConvert.size()),
                        const_cast<wchar_t *>(nStr.c_str()), len);
    return nStr;
}

