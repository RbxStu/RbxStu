//
// Created by Dottik on 21/4/2024.
//

#include "Communication.hpp"

#include "Environment/FilesystemLibrary.hpp"

void Communication::handshake() {
    char buf[0x9] = "00000000";
    buf[0x1] = Communication::OpCodes::GetWorkspace;
    const auto sendData = ix::IXWebSocketSendData{buf, 1};
    this->m_pWebsocket->sendBinary(sendData);
    this->m_pWebsocket->setOnMessageCallback([](const ix::WebSocketMessagePtr &message) {
        if (message->type == ix::WebSocketMessageType::Message) {
            auto workspace = message->str;
            FilesystemLibrary::set_workspace_path(workspace);
        }
    });
}

Communication::Communication(const std::string &websocketUrl) {
    this->m_pWebsocket = new ix::WebSocket{};
    this->m_pWebsocket->setUrl(websocketUrl);
}

void Communication::connect() {
    this->m_pWebsocket->connect(5);
    this->handshake();
}
