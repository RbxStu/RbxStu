//
// Created by Dottik on 21/11/2023.
//
#pragma once

#include <sstream>
#include <oxorany.hpp>

#define oxorany_pchar(wstr) (Module::Utilities::get_singleton()->ToChar((wstr)))    // Oxorany has been removed.

namespace Module {
    class Utilities {
        Utilities() = default;

        static Utilities *sm_pModule;
    public:
        static Utilities *get_singleton();

        std::string RandomString(int length);

        std::wstring RandomWideString(int length);

        /// Converts wchar_t into char. Returns heap allocated memory. YOU MUST DISPOSE!
        const char *ToChar(const wchar_t *szConvert);

        /// Converts std::wstring into std::string.
        std::string ToString(const std::wstring &szConvert);

        /// Converts std::sstring into std::wstring.
        std::wstring ToWideString(const std::string &szConvert);

        /// Converts char into wchar_t. Returns heap allocated memory. YOU MUST DISPOSE!
        const wchar_t *ToWideCharacter(const char *szConvert);

    };
}

