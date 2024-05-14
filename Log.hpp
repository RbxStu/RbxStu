//
// Created by Dottik on 13/5/2024.
//
#pragma once
#include <chrono>
#include <filesystem>
#include <string>

/// Logger class.
class Log {
    static Log *sm_singleton;
    std::string m_szFileName;
    std::string m_szBuffer;
    Log() {
        char buf[0xffff];
        size_t readCount{0};
        getenv_s(&readCount, buf, sizeof(buf), "APPDATA");
        auto folder = std::format("{}\\RbxStu\\", buf);
        std::filesystem::create_directories(folder);
        this->m_szFileName = std::format("{}\\RbxStu\\RbxStuLog-{}.log", buf,
                                         (std::format("{:%d-%m-%Y_%H.%M.%S}", std::chrono::system_clock::now())));

        printf("%s -> filename for log \n", this->m_szFileName.c_str());
    };

public:
    static Log *get_singleton();

    void write_to_buffer(const std::string &moduleName, const std::string &functionName, const std::string &logContent);

    void flush_to_disk() const;
};
