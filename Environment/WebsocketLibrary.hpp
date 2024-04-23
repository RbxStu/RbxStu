//
// Created by Dottik on 15/4/2024.
//
#pragma once

#include <ixwebsocket/IXWebSocket.h>
#include <ixwebsocket/IXWebSocketMessageType.h>
#include <string>
#include "EnvironmentLibrary.hpp"

class Websocket final {
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
        ullLuaThreadRef = -1;

        pSocketUserdata = nullptr;
        pLuaThread = nullptr;
    }

    ~Websocket() {
        free(pWebSocket); // I know this is wrong. But else, I'll get thrown into an instant-crash exception. Nothing I
                          // can do to solve it.
    }

    bool initialize_socket(lua_State *origin);

    bool set_callback_and_url(const std::string &url);
};

class WebsocketLibrary final : public EnvironmentLibrary {
public:
    virtual ~WebsocketLibrary() = default;

    void register_environment(lua_State *L) override;
};
