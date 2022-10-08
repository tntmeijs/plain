#ifdef WIN32

#ifndef NETWORK_TCP_WINDOWS_SOCKET_HPP
#define NETWORK_TCP_WINDOWS_SOCKET_HPP

#include "tcp/socket.hpp"

#include <WinSock2.h>

namespace network::tcp::windows {

	class WindowsTcpSocket final : public TcpSocket {
	public:
		WindowsTcpSocket();
		~WindowsTcpSocket();

		// Connect to a host and port
		bool open(const std::string_view& hostName, const std::uint32_t port) final;

		// Send data to the server
		bool send(const std::string_view& payload) final;

		// Receive N bytes from the server
		bool receive();

		// Change the socket's state (think of operations such as closing the socket for sending)
		bool changeState(const SocketStateChange state) final;

		// Close the socket
		void close() final;

	private:
		SOCKET socket;
	};

}

#endif // !NETWORK_TCP_WINDOWS_SOCKET_HPP

#endif // !WIN32
