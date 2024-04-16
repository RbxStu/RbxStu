//
// Created by Dottik on 15/4/2024.
//

#include "WebsocketLibrary.hpp"
#include "ldebug.h"
#include "lualib.h"
#include <string>
#include <map>

std::map<void *, Websocket *> websockets{};

int websocket_connect(lua_State *L) {
    luaL_checktype(L, 1, LUA_TSTRING);
    std::string targetUrl = lua_tostring(L, 1);
    if (targetUrl.find(("ws://")) != 0 && targetUrl.find(("wss://")) != 0) {
        luaG_runerror(L, ("Invalid protocol (expected 'ws://' or 'wss://')"));
    }
    auto socket = new Websocket{};
    if (!socket->try_connect_websocket(targetUrl)) {
        luaG_runerror(L, ("Failed to connect to remote address!"));
    }

    socket->initialize_socket(L);
    lua_pushthread(L);
    lua_ref(L, -1);
    lua_pop(L, 1);
    return 1;
}

void WebsocketLibrary::RegisterEnvironment(lua_State *L) {
    static const luaL_Reg libreg[] = {
            {"connect", websocket_connect},
            {nullptr,   nullptr},
    };

    lua_newtable(L);
    luaL_register(L, nullptr, libreg);
    lua_setreadonly(L, -1, true);
    lua_setfield(L, LUA_GLOBALSINDEX, "WebSocket");

    lua_pop(L, 1);
}


bool Websocket::try_connect_websocket(const std::string &url) {
    this->pWebSocket->setUrl(url);
    this->pWebSocket->setOnMessageCallback([this](const ix::WebSocketMessagePtr &msg) {
        switch (msg->type) {
            case ix::WebSocketMessageType::Message: {
                if (this->EventReferences.onMessage_ref == -1) return;
                lua_getref(this->pLuaThread, this->EventReferences.onMessage_ref);
                lua_getfield(this->pLuaThread, -1, "Fire");
                lua_pushvalue(this->pLuaThread, -2);
                lua_pushlstring(this->pLuaThread, msg->str.c_str(), msg->str.size());
                lua_pcall(this->pLuaThread, 2, 0, 0);
                this->pLuaThread->top = this->pLuaThread->base;
                break;
            };
            case ix::WebSocketMessageType::Error: {
                if (this->EventReferences.onError_ref == -1) return;
                lua_getref(this->pLuaThread, this->EventReferences.onError_ref);
                lua_getfield(this->pLuaThread, -1, "Fire");
                lua_pushvalue(this->pLuaThread, -2);
                lua_pushlstring(this->pLuaThread, msg->str.c_str(), msg->str.size());
                lua_pcall(this->pLuaThread, 2, 0, 0);
                this->pLuaThread->top = this->pLuaThread->base;

                std::stringstream ss;
                ss << ("Status Code: ") << msg->errorInfo.http_status << std::endl;
                ss << ("Error: ") << msg->errorInfo.reason << std::endl;
                ss << ("retries: ") << msg->errorInfo.retries << std::endl;
                ss << ("Wait time(ms): ") << msg->errorInfo.wait_time << std::endl;

                if (this->EventReferences.onClose_ref != -1)
                    lua_unref(this->pLuaThread, this->EventReferences.onClose_ref);
                if (this->EventReferences.onMessage_ref != -1)
                    lua_unref(this->pLuaThread, this->EventReferences.onMessage_ref);
                if (this->pLuaThreadRef != -1)
                    lua_unref(this->pLuaThread, this->pLuaThreadRef);

                luaG_runerrorL(this->pLuaThread, "Websocket Error! \r\n\r\n%s", ss.str().c_str());
            };
            case ix::WebSocketMessageType::Close: {
                if (this->EventReferences.onClose_ref == -1) return;
                lua_getref(this->pLuaThread, this->EventReferences.onClose_ref);
                lua_getfield(this->pLuaThread, -1, ("Fire"));
                lua_pushvalue(this->pLuaThread, -2);
                lua_pcall(this->pLuaThread, 1, 0, 0);
                this->pLuaThread->top = this->pLuaThread->base;

                if (this->EventReferences.onClose_ref != -1)
                    lua_unref(this->pLuaThread, this->EventReferences.onClose_ref);
                if (this->EventReferences.onError_ref != -1)
                    lua_unref(this->pLuaThread, this->EventReferences.onError_ref);
                if (this->EventReferences.onMessage_ref != -1)
                    lua_unref(this->pLuaThread, this->EventReferences.onMessage_ref);
                if (this->pLuaThreadRef != -1)
                    lua_unref(this->pLuaThread, this->pLuaThreadRef);

                break;
            };
        }
    });

    auto con = this->pWebSocket->connect(10);
    if (con.success) {
        this->pWebSocket->start();
        return true;
    } else {
        printf("websocket.connect Failed (Code: %i): %s", con.http_status, con.errorStr.c_str());
        return false;
    }
}

int Websocket::close(lua_State *L) {
    luaL_checktype(L, 1, LUA_TUSERDATA);
    void *userdata = lua_touserdata(L, 1);
    if (websockets.find(userdata) == websockets.end()) {
        luaG_runerror(L, ("Socket has been freed/not initialized."));
    }

    auto socket = websockets[userdata];
    websockets.erase(userdata);

    socket->pWebSocket->close();

    if (socket->EventReferences.onClose_ref != -1)
        lua_unref(socket->pLuaThread, socket->EventReferences.onClose_ref);
    if (socket->EventReferences.onError_ref != -1)
        lua_unref(socket->pLuaThread, socket->EventReferences.onError_ref);
    if (socket->EventReferences.onMessage_ref != -1)
        lua_unref(socket->pLuaThread, socket->EventReferences.onMessage_ref);
    if (socket->pLuaThreadRef != -1)
        lua_unref(socket->pLuaThread, socket->pLuaThreadRef);

    delete socket->pWebSocket;

    return 0;
}

int Websocket::send(lua_State *L) {
    luaL_checktype(L, 1, LUA_TUSERDATA);
    luaL_checktype(L, 2, LUA_TSTRING);
    void *userdata = lua_touserdata(L, 1);
    std::string msg = lua_tostring(L, 2);
    if (websockets.find(userdata) == websockets.end()) {
        luaG_runerror(L, ("Socket has been freed/not initialized."));
    }
    auto socket = websockets[userdata];

    if (socket->pWebSocket->getReadyState() == ix::ReadyState::Open) {
        socket->pWebSocket->send(msg);
    } else {
        const char *socketStatus;
        switch (socket->pWebSocket->getReadyState()) {
            case ix::ReadyState::Open: {
                socketStatus = "Open";
                break;
            }
            case ix::ReadyState::Closed: {
                socketStatus = "Closed";
                break;
            }
            case ix::ReadyState::Closing: {
                socketStatus = "Closing";
                break;
            }
            default:
                socketStatus = "Unknown";
                break;
        }

        luaG_runerror(L, "Cannot send message! The socket is currently %s!", socketStatus);
    }

    return 0;
}

int Websocket::__index(lua_State *L) {
    luaL_checktype(L, 1, LUA_TUSERDATA);
    luaL_checktype(L, 2, LUA_TSTRING);
    void *userdata = lua_touserdata(L, 1);
    const char *key = lua_tostring(L, 2);

    if (websockets.find(userdata) == websockets.end()) {
        luaG_runerror(L, ("Socket has been freed/not initialized."));
    }

    auto socket = websockets[userdata];
    if (strcmp(key, ("Send")) == 0) {
        lua_pushvalue(L, 1);
        lua_pushcclosure(L, Websocket::send, 0, 1);
    } else if (strcmp(key, ("Close")) == 0) {
        lua_pushvalue(L, 1);
        lua_pushcclosure(L, Websocket::close, 0, 1);
    } else if (strcmp(key, ("OnMessage")) == 0) {
        lua_getref(L, socket->EventReferences.onMessage_ref);
        lua_getfield(L, -1, "Event");
    } else if (strcmp(key, ("OnClose")) == 0) {
        lua_getref(L, socket->EventReferences.onClose_ref);
        lua_getfield(L, -1, "Event");
    } else if (strcmp(key, ("OnError")) == 0) {
        lua_getref(L, socket->EventReferences.onError_ref);
        lua_getfield(L, -1, "Event");
    }

    return 1;
}

// Calls Instance.new(instanceClassName).
#define INSTANCE_NEW(L, instanceClassName) \
    { lua_getfield(L, LUA_GLOBALSINDEX, "Instance"); lua_getfield(L, -1, "new"); lua_pushstring(L, instanceClassName); lua_pcall(L, 1, 1, 0); }

bool Websocket::initialize_socket(lua_State *origin) {  // Not Pure.
    this->pLuaThread = lua_newthread(origin);
    this->pLuaThreadRef = lua_ref(origin, -1);
    lua_pop(origin, 1);

    INSTANCE_NEW(origin, "BindableEvent");
    this->EventReferences.onMessage_ref = lua_ref(origin, -1);
    lua_pop(origin, 1);
    INSTANCE_NEW(origin, "BindableEvent");
    this->EventReferences.onClose_ref = lua_ref(origin, -1);
    lua_pop(origin, 1);
    INSTANCE_NEW(origin, "BindableEvent");
    this->EventReferences.onError_ref = lua_ref(origin, -1);
    lua_pop(origin, 1);

    // References initialized, initialize Userdata && Metatable.

    this->pSocketUserdata = reinterpret_cast<Websocket *>(lua_newuserdata(origin, sizeof(Websocket)));

    lua_newtable(origin);   // __metatable

    lua_pushstring(origin, "WebSocket");
    lua_setfield(origin, -2, "__type");

    lua_pushcclosure(origin, Websocket::__index, nullptr, 0);
    lua_setfield(origin, -2, "__index");

    lua_setmetatable(origin, -2);

    websockets.insert({static_cast<void *>(this->pSocketUserdata), this});

    return true;
}
