#include "tcp/socket.hpp"
#include "tcp/socket_factory.hpp"

int main(int argc, char* argv[]) {
	// Unreferenced formal parameter warnings
	argc;
	argv;

	const auto socket = network::tcp::SocketFactory().create();

	if (!socket) {
		// Failed to create a socket
		return 1;
	}

	if (!socket->open("127.0.0.1", 1337)) {
		socket->close();
		return 1;
	}

	socket->send("Hello, TCP!\n");
	socket->send("Another message...\n");
	socket->send("Goodbye!\n");

	// Wait until the server disconnects the client
	socket->receive();

	socket->close();

	return 0;
}
