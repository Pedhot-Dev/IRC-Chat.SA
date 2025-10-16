#pragma once
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define WIN32_LEAN_AND_MEAN
#include <string>
#include <functional>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <thread>

#pragma comment(lib, "ws2_32.lib")

class IRCClient {
public:
    IRCClient();
    ~IRCClient();

    bool Connect(const std::string& server, int port, const std::string& nickname, const std::string& channel);
    void SendMessage(const std::string& message, const std::string& global) const;
    void Disconnect();
    bool IsConnected() const;

    void SetOnMessage(std::function<void(const std::string&)> callback);

    void StartReceiving();  // Thread-based receiver loop
    void StopReceiving();

private:
    void ReceiveLoop();

    SOCKET m_socket = INVALID_SOCKET;
    bool m_connected = false;
    bool m_running = false;
    std::string m_channel;
    std::thread m_recvThread;
    std::function<void(const std::string&)> onMessage;
};
