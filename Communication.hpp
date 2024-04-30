//
// Created by Dottik on 21/4/2024.
//

#pragma once
#include "ixwebsocket/IXWebSocket.h"


class Communication final {
    ix::WebSocket *m_pWebsocket;

    void handshake();

public:
    enum OpCodes : unsigned char {
        /*
            Obtains the workspace used for the Filesystem library. Provided by the external GUI.
         */
        GetWorkspace = 0x0,
        /*
            Used to enqueue a script into the modules' task scheduler.
         */
        ExecuteScript = 0x4,
        /*
            Used to signal the module to reinitialize and grab a new lua_State *.
         */
        Reinitialize = 0x8,
    };
    explicit Communication(const std::string &websocketUrl);
    void connect();
};
