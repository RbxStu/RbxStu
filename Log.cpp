//
// Created by Dottik on 13/5/2024.
//

#include "Log.hpp"

#include <chrono>
#include <fstream>

Log *Log::sm_singleton = nullptr;

Log *Log::get_singleton() {
    if (Log::sm_singleton == nullptr)
        Log::sm_singleton = new Log();

    return Log::sm_singleton;
}

void Log::write_to_buffer(const std::string &moduleName, const std::string &functionName,
                          const std::string &logContent) {
    this->m_szBuffer +=
            std::format("\n[{} @ {}::{}] {}", (std::format("{:%d/%m/%Y %H:%M:%S}", std::chrono::system_clock::now())),
                        moduleName, functionName, logContent);
}

void Log::flush_to_disk() const {
    std::ofstream ofStream(this->m_szFileName);
    if (!ofStream.is_open()) {
        printf("WARNING: Log file not open!\n");
    }
    ofStream << this->m_szBuffer;
    ofStream.flush();
    ofStream.close();
}
