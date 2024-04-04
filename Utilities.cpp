//
// Created by Dottik on 21/11/2023.
//

#include <Windows.h>
#include <string>
#include <random>
#include <oxorany.hpp>
#include "Utilities.hpp"

Module::Utilities *Module::Utilities::sm_pModule = nullptr;

Module::Utilities *Module::Utilities::GetSingleton() {
    if (Module::Utilities::sm_pModule == nullptr)
        Module::Utilities::sm_pModule = new Module::Utilities();

    return Module::Utilities::sm_pModule;
}

std::wstring Module::Utilities::RandomWideString(int length) {
    const wchar_t *alphabet = oxorany(L"abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789");

    std::random_device rngDevice;
    std::mt19937 rng(rngDevice());
    std::uniform_int_distribution<int> distribution(oxorany(0), oxorany(62));

    std::wstring randomString;
    randomString.reserve(length);

    for (int i = 0; i < length; ++i) {
        randomString += alphabet[distribution(rng)];
    }

    return randomString;
}

std::string Module::Utilities::RandomString(int length) {
    const char *alphabet = this->ToChar(oxorany(L"abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789"));

    std::random_device rngDevice;
    std::mt19937 rng(rngDevice());
    std::uniform_int_distribution<int> distribution(oxorany(0), oxorany(62));


    std::string randomString;
    randomString.reserve(length);

    for (int i = 0; i < length; ++i) {
        randomString += alphabet[distribution(rng)];
    }

    delete[] alphabet; // Deallocate.

    return randomString;
}

/// Converts wchar_t into char. Returns heap allocated memory. YOU MUST DISPOSE!
const char *Module::Utilities::ToChar(const wchar_t *szConvert) {
    auto len = (wcslen(szConvert) + 1) * 2;
    char *str = new char[len];
    size_t convertedChars = 0;
    wcstombs_s(&convertedChars, str, len, szConvert, _TRUNCATE);
    return str;
}

/// Converts char into wchar_t. Returns heap allocated memory. YOU MUST DISPOSE!
const wchar_t *Module::Utilities::ToWideCharacter(const char *szConvert) {
    auto charLen = strlen(szConvert) + 1;
    auto *wText = new wchar_t[charLen];
    size_t outSize = 0;
    mbstowcs_s(&outSize, wText, charLen, szConvert, charLen - oxorany_flt(1));
    return wText;
}


std::string Module::Utilities::ToString(const std::wstring& szConvert) {
    if (szConvert.empty()) return {};

    int len = WideCharToMultiByte(CP_UTF8, 0, szConvert.c_str(), (int) szConvert.size(), nullptr, 0, nullptr,
                                  nullptr);
    std::string strTo(len, 0);
    WideCharToMultiByte(CP_UTF8, 0, szConvert.c_str(), (int) szConvert.size(), const_cast<char *>(strTo.c_str()),
                        len, nullptr, nullptr);
    return strTo;
}

std::wstring Module::Utilities::ToWideString(const std::string& szConvert) {
    if (szConvert.empty()) return {};

    int len = MultiByteToWideChar(CP_UTF8, 0, szConvert.c_str(), (int) szConvert.size(), nullptr, 0);
    std::wstring nStr(len, 0);
    MultiByteToWideChar(CP_UTF8, 0, szConvert.c_str(), (int) szConvert.size(), const_cast<wchar_t *>(nStr.c_str()),
                        len);
    return nStr;
}
