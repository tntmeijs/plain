#include "tcp/socket.hpp"
#include "tcp/socket_factory.hpp"
#include "http/request_message_builder.hpp"

#include <iostream>

using namespace network::tcp;
using namespace network::http;

constexpr const char* const USER_AGENT_NAME = "Plain/0.1";

int main(int argc, char* argv[]) {
	// Unreferenced formal parameter warnings
	argc;
	argv;

	const auto socket = SocketFactory().create();

	if (!socket) {
		// Failed to create a socket
		return 1;
	}

	if (!socket->open("eu.httpbin.org", 80)) {
		socket->close();
		return 1;
	}

	// Construct a simple HTTP message
	const auto& request = RequestMessageBuilder()
		.withTarget("/")
		.withHost("www.eu.httpbin.org:80")
		.withHeader(Header::UserAgent, USER_AGENT_NAME)
		.withHttpMethod(HttpMethod::Get)
		.build();

	const auto message = request.generate();

	std::cout << message << std::endl;

	socket->send(message);

	// Wait until the server disconnects the client
	socket->receive();

	socket->close();

	return 0;
}
