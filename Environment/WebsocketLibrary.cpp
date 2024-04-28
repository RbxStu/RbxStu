//
// Created by Dottik on 15/4/2024.
//

#include "WebsocketLibrary.hpp"
#include <map>
#include <string>

#include "Environment.hpp"
#include "ldebug.h"
#include "lualib.h"

std::map<void *, Websocket *> websockets{};

int websocket_connect(lua_State *L) {
    luaL_checktype(L, 1, LUA_TSTRING);
    const std::string targetUrl = lua_tostring(L, 1);
    if (Environment::get_singleton()->get_instrumentation_status()) {
        printf("--- WebSocket::connect invoked ---\r\n");
        printf("WebSocket connecting to \'%s\'...\r\n", targetUrl.c_str());
    }

    if (targetUrl.find("ws://") == std::string::npos && targetUrl.find("wss://") == std::string::npos) {
        luaG_runerror(L, ("Invalid protocol (expected 'ws://' or 'wss://')"));
    }
    auto *socket = new Websocket{};
    if (!socket->set_callback_and_url(targetUrl)) {
        luaG_runerror(L, ("Failed to connect to remote address!"));
    }

    socket->initialize_socket(L);
    if (const auto con = socket->pWebSocket->connect(10); con.success) {
        socket->pWebSocket->start();
    } else {
        printf("websocket.connect Failed (Code: %i): %s", con.http_status, con.errorStr.c_str());
        luaG_runerror(L, ("Failed to connect to remote address!"));
    }

    lua_pushthread(L);
    lua_ref(L, -1);
    lua_pop(L, 1);
    return 1;
}

void WebsocketLibrary::register_environment(lua_State *L) {
    static const luaL_Reg libreg[] = {
            {"connect", websocket_connect},
            {nullptr, nullptr},
    };

    lua_newtable(L);
    luaL_register(L, nullptr, libreg);
    lua_setreadonly(L, -1, true);
    lua_setfield(L, LUA_GLOBALSINDEX, "WebSocket");

    lua_pop(L, 1);
}


bool Websocket::set_callback_and_url(const std::string &url) {
    this->pWebSocket->setUrl(url);
    this->pWebSocket->setOnMessageCallback([this](const ix::WebSocketMessagePtr &msg) {
        switch (msg->type) {
            case ix::WebSocketMessageType::Message: {
                if (this->EventReferences.onMessage_ref == -1)
                    return;
                lua_rawgeti(this->pLuaThread, LUA_REGISTRYINDEX, this->EventReferences.onMessage_ref);
                if (this->pLuaThread->top->tt != LUA_TUSERDATA) {
                    printf("[Websocket::OnMessageCallback] Cowardly refusing to fire an event due to a lua_ref "
                           "problem\r\n");
                    break;
                }
                lua_getfield(this->pLuaThread, -1, "Fire");
                lua_pushvalue(this->pLuaThread, -2);
                lua_pushlstring(this->pLuaThread, msg->str.c_str(), msg->str.size());
                lua_pcall(this->pLuaThread, 2, 0, 0);
                this->pLuaThread->top = this->pLuaThread->base;
                break;
            };
            case ix::WebSocketMessageType::Error: {
                if (this->EventReferences.onError_ref == -1)
                    return;
                lua_rawgeti(this->pLuaThread, LUA_REGISTRYINDEX, (this->EventReferences.onError_ref));
                if (this->pLuaThread->top->tt != LUA_TUSERDATA) {
                    printf("[Websocket::OnMessageCallback] Cowardly refusing to fire an event due to a lua_ref "
                           "problem\r\n");
                    break;
                }
                lua_getfield(this->pLuaThread, -1, "Fire");
                lua_pushvalue(this->pLuaThread, -2);
                lua_pushlstring(this->pLuaThread, msg->str.c_str(), msg->str.size());
                lua_pcall(this->pLuaThread, 2, 0, 0);
                this->pLuaThread->top = this->pLuaThread->base;

                std::stringstream ss;
                ss << "Status Code: " << msg->errorInfo.http_status << std::endl;
                ss << "Error: " << msg->errorInfo.reason << std::endl;
                ss << "retries: " << msg->errorInfo.retries << std::endl;
                ss << "Wait time(ms): " << msg->errorInfo.wait_time << std::endl;

                if (this->EventReferences.onClose_ref != -1)
                    lua_unref(this->pLuaThread, this->EventReferences.onClose_ref);
                if (this->EventReferences.onMessage_ref != -1)
                    lua_unref(this->pLuaThread, this->EventReferences.onMessage_ref);
                if (this->ullLuaThreadRef != -1)
                    lua_unref(this->pLuaThread, this->ullLuaThreadRef);

                luaG_runerrorL(this->pLuaThread, "Websocket Error! \r\n\r\n%s", ss.str().c_str());
            };
            case ix::WebSocketMessageType::Close: {
                if (this->EventReferences.onClose_ref == -1)
                    return;
                lua_rawgeti(this->pLuaThread, LUA_REGISTRYINDEX, (this->EventReferences.onClose_ref));
                if (this->pLuaThread->top->tt != LUA_TUSERDATA) {
                    printf("[Websocket::OnMessageCallback] Cowardly refusing to fire an event due to a lua_ref "
                           "problem\r\n");
                    break;
                }
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
                if (this->ullLuaThreadRef != -1)
                    lua_unref(this->pLuaThread, this->ullLuaThreadRef);

                break;
            };
            default:
                printf("\r\n[Websocket::try_connect_websocket::message_callback] Unhandled message type! %d\r\n",
                       msg->type);
        }
    });
    return true;
}

int Websocket::close(lua_State *L) {
    if (Environment::get_singleton()->get_instrumentation_status()) {
        printf("--- WebSocket::close invoked ---\r\n");
    }
    luaL_checktype(L, 1, LUA_TUSERDATA);
    void *userdata = lua_touserdata(L, 1);
    if (!websockets.contains(userdata)) {
        luaG_runerror(L, ("Socket has been freed/not initialized."));
    }

    const auto socket = websockets[userdata];
    websockets.erase(userdata);

    // Unsafe as hell, we pray we don't crash while doing this...
    if (socket->EventReferences.onClose_ref != -1)
        lua_unref(socket->pLuaThread, socket->EventReferences.onClose_ref);
    if (socket->EventReferences.onError_ref != -1)
        lua_unref(socket->pLuaThread, socket->EventReferences.onError_ref);
    if (socket->EventReferences.onMessage_ref != -1)
        lua_unref(socket->pLuaThread, socket->EventReferences.onMessage_ref);
    if (socket->ullLuaThreadRef != -1)
        lua_unref(socket->pLuaThread, socket->ullLuaThreadRef);

    delete socket;

    return 0;
}

int Websocket::send(lua_State *L) {
    luaL_checktype(L, 1, LUA_TUSERDATA);
    luaL_checktype(L, 2, LUA_TSTRING);
    void *userdata = lua_touserdata(L, 1);
    const std::string msg = lua_tostring(L, 2);
    if (Environment::get_singleton()->get_instrumentation_status()) {
        printf("--- WebSocket::send invoked ---\r\n");
        printf("Payload sent by websocket: \'%s\'\r\n", msg.c_str());
    }
    if (!websockets.contains(userdata)) {
        luaG_runerror(L, ("Socket has been freed/not initialized."));
    }

    if (const auto socket = websockets[userdata]; socket->pWebSocket->getReadyState() == ix::ReadyState::Open) {
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

    if (Environment::get_singleton()->get_instrumentation_status()) {
        printf("--- WebSocket::__index invoked ---\r\n");
        printf("Indexed with key: \'%s\'\r\n", key);
    }

    if (!websockets.contains(userdata)) {
        luaG_runerror(L, "Socket has been freed/not initialized.");
    }

    auto socket = websockets[userdata];
    if (strcmp(key, "Send") == 0) {
        lua_pushvalue(L, 1);
        lua_pushcclosure(L, Websocket::send, 0, 1);
    } else if (strcmp(key, "Close") == 0) {
        lua_pushvalue(L, 1);
        lua_pushcclosure(L, Websocket::close, 0, 1);
    } else if (strcmp(key, "OnMessage") == 0) {
        lua_getref(L, socket->EventReferences.onMessage_ref);
        lua_getfield(L, -1, "Event");
    } else if (strcmp(key, "OnClose") == 0) {
        lua_getref(L, socket->EventReferences.onClose_ref);
        lua_getfield(L, -1, "Event");
    } else if (strcmp(key, "OnError") == 0) {
        lua_getref(L, socket->EventReferences.onError_ref);
        lua_getfield(L, -1, "Event");
    }

    return 1;
}

// Calls Instance.new(instanceClassName).
#define INSTANCE_NEW(L, instanceClassName)                                                                             \
    {                                                                                                                  \
        lua_getfield(L, LUA_GLOBALSINDEX, "Instance");                                                                 \
        lua_getfield(L, -1, "new");                                                                                    \
        lua_pushstring(L, instanceClassName);                                                                          \
        lua_pcall(L, 1, 1, 0);                                                                                         \
    }

bool Websocket::initialize_socket(lua_State *origin) {
    // Not Pure.
    this->pLuaThread = lua_newthread(origin);
    this->ullLuaThreadRef = lua_ref(origin, -1);
    lua_pop(origin, 1);

    INSTANCE_NEW(this->pLuaThread, "BindableEvent");
    this->EventReferences.onMessage_ref = lua_ref(this->pLuaThread, -1);
    lua_pop(this->pLuaThread, 1);
    INSTANCE_NEW(this->pLuaThread, "BindableEvent");
    this->EventReferences.onClose_ref = lua_ref(this->pLuaThread, -1);
    lua_pop(this->pLuaThread, 1);
    INSTANCE_NEW(this->pLuaThread, "BindableEvent");
    this->EventReferences.onError_ref = lua_ref(this->pLuaThread, -1);
    lua_pop(this->pLuaThread, 1);

    // References initialized, initialize Userdata && Metatable.

    this->pSocketUserdata = static_cast<Websocket *>(lua_newuserdata(origin, sizeof(Websocket)));

    lua_newtable(origin); // __metatable

    lua_pushstring(origin, "WebSocket");
    lua_setfield(origin, -2, "__type");

    lua_pushcclosure(origin, Websocket::__index, nullptr, 0);
    lua_setfield(origin, -2, "__index");

    lua_setmetatable(origin, -2);

    websockets.insert({static_cast<void *>(this->pSocketUserdata), this});

    return true;
}
