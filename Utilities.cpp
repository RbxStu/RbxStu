//
// Created by Dottik on 21/11/2023.
//

#define _CRT_SECURE_NO_WARNINGS

#include "Utilities.hpp"
#include <Windows.h>
#include <filesystem>
#include <oxorany.hpp>
#include <random>
#include <string>
#include <tlhelp32.h>


Module::Utilities *Module::Utilities::sm_pModule = nullptr;

EXTERN_C IMAGE_DOS_HEADER __ImageBase;

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

std::string Module::Utilities::replace(std::string subject, std::string search, std::string replace) {
    size_t pos = 0;

    while ((pos = subject.find(search, pos)) != std::string::npos)
    {
        subject.replace(pos, search.length(), replace);
        pos += replace.length();
    }

    return subject;
}

BOOL Module::Utilities::equals_ignore_case(std::string_view a, std::string_view b) {
    return std::equal(a.begin(), a.end(),
        b.begin(), b.end(),
        [](char a, char b) {
            return tolower(a) == tolower(b);
        }
    );
}

std::string Module::Utilities::read_file(std::string file_location) {
    auto close_file = [](FILE* f) { fclose(f); };
    auto holder = std::unique_ptr<FILE, decltype(close_file)>(fopen(file_location.c_str(), "rb"), close_file);

    if (!holder)
        return "";

    FILE* f = holder.get();

    if (fseek(f, 0, SEEK_END) < 0)
        return "";

    const long size = ftell(f);

    if (size < 0)
        return "";

    if (fseek(f, 0, SEEK_SET) < 0)
        return "";

    std::string res;
    res.resize(size);
    fread(const_cast<char*>(res.data()), 1, size, f);

    return res;
}

std::string Module::Utilities::GetLocation() {
    char DllPath[MAX_PATH] = {0};
    GetModuleFileNameA((HINSTANCE)&__ImageBase, DllPath, _countof(DllPath));

    std::string dll_path = DllPath;

    std::string exploit_path = dll_path.substr(0, dll_path.rfind('\\'));

    this->location = exploit_path;

    return this->location;
}

void Module::Utilities::create_workspace() {
    if (!std::filesystem::exists(this->location + ("\\workspace")))
            std::filesystem::create_directory(this->location + ("\\workspace"));
}
