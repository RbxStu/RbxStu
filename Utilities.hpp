//
// Created by Dottik on 21/11/2023.
//
#pragma once

#include <Windows.h>
#include <sstream>

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

        template<typename T>
        static bool is_pointer_valid(T *tValue) {
            // Templates fuck themselves if you don't have it like this lol

            const auto ptr = reinterpret_cast<void *>(tValue);
            auto buf = MEMORY_BASIC_INFORMATION{};

            if (const auto read = VirtualQuery(ptr, &buf, sizeof(T)); sizeof(T) != read) {
                // I honestly dont care.
            } else if (read == 0) {
                // Failure.
                printf("[Module::Utilities::is_pointer_valid<T>] Failed to query information for ptr %p. Last Error: "
                       "%lx",
                       ptr, GetLastError());
                return false;
            }

            if (buf.State == MEM_FREE)
                return false; // The memory is not owned by the process, no need to do anything, we can already assume
                              // we cannot read it.

            switch (buf.Protect) {
                case PAGE_READWRITE:
                case PAGE_READONLY:
                case PAGE_EXECUTE_READWRITE:
                    return true;
                default:
                case PAGE_NOACCESS:
                    return false;
            }
        }
    };
} // namespace Module
