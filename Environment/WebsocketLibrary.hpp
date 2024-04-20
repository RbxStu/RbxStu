//
// Created by Dottik on 15/4/2024.
//
#pragma once

#include "EnvironmentLibrary.hpp"
#include <string>
#include <ixwebsocket/IXWebSocket.h>
#include <ixwebsocket/IXWebSocketMessageType.h>

class Websocket {
    static int __index(lua_State *L);

    static int close(lua_State *L);

    static int send(lua_State *L);

public:
    Websocket *pSocketUserdata; // Self (as ud).
    ix::WebSocket *pWebSocket;

    lua_State *pLuaThread;
    long ullLuaThreadRef;

    struct {
        long onClose_ref;
        long onMessage_ref;
        long onError_ref;
    } EventReferences;

    Websocket() {
        pWebSocket = new ix::WebSocket{};
        EventReferences = {-1, -1, -1};

        pSocketUserdata = nullptr;
        pLuaThread = nullptr;
        ullLuaThreadRef = -1;
    }

    bool initialize_socket(lua_State *origin);

    bool try_connect_websocket(const std::string &url);
};

class WebsocketLibrary final : public EnvironmentLibrary {
public:
    virtual ~WebsocketLibrary() = default;

    void register_environment(lua_State *L) override;
};
