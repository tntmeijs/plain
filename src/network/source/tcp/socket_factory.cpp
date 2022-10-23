#include "network/tcp/socket_factory.hpp"
#include "network/tcp/windows/windows_socket.hpp"

#include "spdlog/spdlog.h"

#include <memory>

using namespace network::tcp;

std::unique_ptr<TcpSocket> SocketFactory::create() {
#if WIN32
	spdlog::debug("Creating new Windows TCP socket");
	return std::make_unique<windows::WindowsTcpSocket>();
#else
	return nullptr;
#endif
}
