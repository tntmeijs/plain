#include "tcp/socket_factory.hpp"
#include "tcp/windows/windows_socket.hpp"

#include <memory>

using namespace network::tcp;

std::unique_ptr<TcpSocket> SocketFactory::create() {
#if WIN32
	return std::make_unique<windows::WindowsTcpSocket>();
#else
	return nullptr;
#endif
}
