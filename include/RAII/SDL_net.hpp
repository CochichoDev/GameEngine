#ifndef SDL_NET_H
#define SDL_NET_H

#include "SDL3_net/SDL_net.h"

#include <iostream>

class SDLNet {
    public:
        SDLNet() {
            if (!NET_Init()) {
                std::cerr << "[ERROR] SDLNet::SDLNet -> SDL_Net: " << SDL_GetError() << std::endl;
                throw std::runtime_error("Failed to initialiaze SDL_Net");
            }
        }

        SDLNet(const SDLNet&) = delete;
        SDLNet& operator=(const SDLNet&) = delete;

        SDLNet(SDLNet&& other) noexcept = delete;
        SDLNet& operator=(SDLNet&&) noexcept = delete;

        ~SDLNet() {
            NET_Quit();
        }
};

class NETStreamSocket {
    public:
        explicit NETStreamSocket(NET_StreamSocket* stream_socket) {
            if (stream_socket == nullptr) {
                std::cerr << "[ERROR] NETStreamSocket::NETStreamSocket -> Cannot initialize NETStreamSocket with a nullptr" << std::endl;
                throw std::runtime_error("Failed to initialiaze NETStreamSocket");
            }
            m_stream_socket = stream_socket;
        }

        NETStreamSocket(const NETStreamSocket&) = delete;
        NETStreamSocket& operator=(const NETStreamSocket&) = delete;

        NETStreamSocket(NETStreamSocket&& other) noexcept {
            m_stream_socket = other.m_stream_socket;
            other.m_stream_socket = nullptr;
        }
        NETStreamSocket& operator=(NETStreamSocket&& other) noexcept {
            if (this != &other) {
                m_stream_socket = other.m_stream_socket;
                other.m_stream_socket = nullptr;
            }
            return *this;
        }

        NET_StreamSocket* get() {
            return m_stream_socket;
        }

        ~NETStreamSocket() {
            if (m_stream_socket != nullptr)
                NET_DestroyStreamSocket(m_stream_socket);
        }
        
    private:
        NET_StreamSocket* m_stream_socket{nullptr};
};

class NETServer {
    public:
        explicit NETServer(NET_Server* server_socket) {
            if (server_socket == nullptr) {
                std::cerr << "[ERROR] NETServer::NETServer -> Cannot initialize NETServer with a nullptr" << std::endl;
                throw std::runtime_error("Failed to initialiaze NETServer");
            }
            m_server_socket = server_socket;
        }

        NETServer(const NETServer&) = delete;
        NETServer& operator=(const NETServer&) = delete;

        NETServer(NETServer&& other) noexcept {
            m_server_socket = other.m_server_socket;
            other.m_server_socket = nullptr;
        }
        NETServer& operator=(NETServer&& other) noexcept {
            if (this != &other) {
                m_server_socket = other.m_server_socket;
                other.m_server_socket = nullptr;
            }
            return *this;
        }

        NET_Server* get() {
            return m_server_socket;
        }

        ~NETServer() {
            if (m_server_socket != nullptr)
                NET_DestroyServer(m_server_socket);
        }
        
    private:
        NET_Server* m_server_socket{nullptr};
};

class NETDatagramSocket {
    public:
        explicit NETDatagramSocket(NET_DatagramSocket* dgram_socket) {
            if (dgram_socket == nullptr) {
                std::cerr << "[ERROR] NETDatagramSocket::NETDatagramSocket -> Cannot initialize NETDatagramSocket with a nullptr" << std::endl;
                throw std::runtime_error("Failed to initialiaze NETDatagramSocket");
            }
            m_dgram_socket = dgram_socket;
        }

        NETDatagramSocket(const NETDatagramSocket&) = delete;
        NETDatagramSocket& operator=(const NETDatagramSocket&) = delete;

        NETDatagramSocket(NETDatagramSocket&& other) noexcept {
            m_dgram_socket = other.m_dgram_socket;
            other.m_dgram_socket = nullptr;
        }
        NETDatagramSocket& operator=(NETDatagramSocket&& other) noexcept {
            if (this != &other) {
                m_dgram_socket = other.m_dgram_socket;
                other.m_dgram_socket = nullptr;
            }
            return *this;
        }

        NET_DatagramSocket* get() {
            return m_dgram_socket;
        }

        ~NETDatagramSocket() {
            if (m_dgram_socket != nullptr)
                NET_DestroyDatagramSocket(m_dgram_socket);
        }
        
    private:
        NET_DatagramSocket* m_dgram_socket{nullptr};
};

class NETDatagram {
    public:
        explicit NETDatagram(NET_DatagramSocket* socket) noexcept {
            if(!NET_ReceiveDatagram(socket, &m_dgram)) {
                std::cerr << "[ERROR] NETDatagram::NETDatagram -> Could not recover datagram from the socket" << std::endl;
                m_dgram = nullptr;
            }
        }

        NETDatagram(const NETDatagram&) = delete;
        NETDatagram& operator=(const NETDatagram&) = delete;

        NETDatagram(NETDatagram&& other) noexcept {
            m_dgram = other.m_dgram;
            other.m_dgram = nullptr;
        }
        NETDatagram& operator=(NETDatagram&& other) noexcept {
            if (this != &other) {
                if (m_dgram != nullptr) NET_DestroyDatagram(m_dgram);
                m_dgram = other.m_dgram;
                other.m_dgram = nullptr;
            }
            return *this;
        }

        NET_Datagram* get() {
            return m_dgram;
        }

        ~NETDatagram() {
            if (m_dgram != nullptr)
                NET_DestroyDatagram(m_dgram);
        }
        
    private:
        NET_Datagram* m_dgram{nullptr};
};

class NETAddress {
    public:
        explicit NETAddress(NET_Address* addr) {
            if (addr == nullptr) {
                std::cerr << "[ERROR] NETAddress::NETAddress-> Cannot initialize NETAddress with a nullptr" << std::endl;
                throw std::runtime_error("Failed to initialiaze NETAddress");
            }
            m_addr = addr;
        }

        /* This RAII Class is copiable since it allows multiple references */
        NETAddress(const NETAddress& other) {
            m_addr = other.m_addr;
            NET_RefAddress(m_addr);
        }
        NETAddress& operator=(const NETAddress& other) {
            if (this != &other) {
                m_addr = other.m_addr;
                NET_RefAddress(m_addr);
            }
            return *this;
        }

        NETAddress(NETAddress&& other) noexcept {
            m_addr = other.m_addr;
            other.m_addr = nullptr;
        }
        NETAddress& operator=(NETAddress&& other) noexcept {
            if (this != &other) {
                m_addr = other.m_addr;
                other.m_addr = nullptr;
            }
            return *this;
        }

        NET_Address* get() {
            return m_addr;
        }

        const NET_Address* get() const {
            return m_addr;
        }

        ~NETAddress() {
            if (m_addr != nullptr)
                NET_UnrefAddress(m_addr);
        }
        
    private:
        NET_Address* m_addr{nullptr};
};

#endif
