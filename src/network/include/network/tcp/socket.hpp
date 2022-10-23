#ifndef NETWORK_TCP_SOCKET_HPP
#define NETWORK_TCP_SOCKET_HPP

#include "socket_state_change.hpp"

#include <cstdint>
#include <string_view>

namespace network::tcp {

	class TcpSocket {
	public:
		TcpSocket();
		TcpSocket(const TcpSocket&) = default;
		TcpSocket(TcpSocket&&) = default;
		TcpSocket& operator=(const TcpSocket&) = default;
		TcpSocket& operator=(TcpSocket&&) = default;
		virtual ~TcpSocket() = default;

		// Connect to a host and port
		virtual bool open(const std::string_view& hostName, const std::uint32_t port) = 0;

		// Send data to the server
		virtual bool send(const std::string_view& payload) = 0;

		// Receive N bytes from the server
		virtual bool receive() = 0;

		// Change the socket's state (think of operations such as closing the socket for sending)
		virtual bool changeState(const SocketStateChange state) = 0;

		// Close the socket
		virtual void close() = 0;

	protected:
		std::uint8_t receiveBuffer[8'192];
	};

}

#endif // !NETWORK_TCP_SOCKET_HPP
