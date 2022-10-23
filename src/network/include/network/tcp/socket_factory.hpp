#ifndef NETWORK_TCP_SOCKET_FACTORY_HPP
#define NETWORK_TCP_SOCKET_FACTORY_HPP

#include <memory>

namespace network::tcp {

	class TcpSocket;

	class SocketFactory final {
	public:
		// Create a new socket for the current platform the project is compiled on
		std::unique_ptr<TcpSocket> create();
	};

}

#endif // !NETWORK_TCP_SOCKET_FACTORY_HPP
