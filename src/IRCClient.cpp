#include "IRCClient.h"
#include <iostream>

IRCClient::IRCClient() {
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2, 2), &wsaData);
}

IRCClient::~IRCClient() {
    StopReceiving();
    Disconnect();
    WSACleanup();
}

bool IRCClient::Connect(const std::string& server, int port, const std::string& nickname, const std::string& channel) {
    m_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (m_socket == INVALID_SOCKET) return false;

    hostent* host = gethostbyname(server.c_str());
    if (!host) return false;

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = *(u_long*)host->h_addr;

    if (connect(m_socket, (sockaddr*)&addr, sizeof(addr)) == SOCKET_ERROR) {
        closesocket(m_socket);
        return false;
    }

    m_connected = true;
    m_channel = channel;

    std::string nickCmd = "NICK " + nickname + "\r\n";
    std::string userCmd = "USER " + nickname + " 8 * :" + nickname + "\r\n";
    send(m_socket, nickCmd.c_str(), nickCmd.size(), 0);
    send(m_socket, userCmd.c_str(), userCmd.size(), 0);

    Sleep(1000);

    std::string joinCmd = "JOIN " + channel + "\r\n";
    send(m_socket, joinCmd.c_str(), joinCmd.size(), 0);

    return true;
}

void IRCClient::SendMessage(const std::string& message) {
    if (!m_connected) return;
    std::string msg = "PRIVMSG " + m_channel + " :" + message + "\r\n";
    send(m_socket, msg.c_str(), msg.size(), 0);
}

void IRCClient::Disconnect() {
    if (m_connected) {
        closesocket(m_socket);
        m_socket = INVALID_SOCKET;
        m_connected = false;
    }
}

bool IRCClient::IsConnected() const {
    return m_connected;
}

void IRCClient::SetOnMessage(std::function<void(const std::string&)> callback) {
    onMessage = std::move(callback);
}

void IRCClient::StartReceiving() {
    if (m_running) return;
    m_running = true;

    m_recvThread = std::thread([this]() { ReceiveLoop(); });
    m_recvThread.detach();
}

void IRCClient::StopReceiving() {
    m_running = false;
}

void IRCClient::ReceiveLoop() {
    char buffer[512];
    while (m_running && m_connected) {
        int bytes = recv(m_socket, buffer, sizeof(buffer) - 1, 0);
        if (bytes > 0) {
            buffer[bytes] = '\0';
            std::string msg(buffer);
            printf("IRC RECV: %s\n", buffer);

            // Auto PING -> PONG
            if (msg.find("PING") != std::string::npos) {
                size_t pos = msg.find(":");
                if (pos != std::string::npos) {
                    std::string pingData = msg.substr(pos + 1);
                    std::string pong = "PONG :" + pingData + "\r\n";
                    send(m_socket, pong.c_str(), pong.size(), 0);
                    printf(">>> Sent PONG: %s\n", pingData.c_str());
                }
            }

            if (onMessage)
                onMessage(msg);
        } else if (bytes == 0) {
            printf("IRC disconnected.\n");
            m_connected = false;
            break;
        } else {
            int err = WSAGetLastError();
            if (err != WSAEWOULDBLOCK && err != 10035)
                printf("IRC recv error: %d\n", err);
            Sleep(100);
        }
    }
}
