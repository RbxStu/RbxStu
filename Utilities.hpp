//
// Created by Dottik on 21/11/2023.
//
#pragma once

#include <sstream>
#include <oxorany.hpp>

namespace Module {
    class Utilities {
        Utilities() = default;

        static Utilities *sm_pModule;

    public:
        static Utilities *get_singleton();

        std::string get_random_string(int length);

        std::wstring get_random_wstring(int length);

        /// Converts wchar_t into char. Returns heap allocated memory. YOU MUST DISPOSE!
        const char *to_char(const wchar_t *szConvert);

        /// Converts std::wstring into std::string.
        std::string to_string(const std::wstring &szConvert);

        /// Converts std::sstring into std::wstring.
        std::wstring to_wstring(const std::string &szConvert);

        /// Converts char into wchar_t. Returns heap allocated memory. YOU MUST DISPOSE!
        const wchar_t *to_wchar(const char *szConvert);
    };
}
