#ifdef WIN32

#include "tcp/windows/windows_socket.hpp"

#include <WS2tcpip.h>

#include <iostream>
#include <string>
#include <vector>

using namespace network;
using namespace network::tcp::windows;

// Helper to convert to IPv4
std::string convertIntoIPv4(const sockaddr* const address) {
	char buffer[16] = {};
	return ::InetNtop(AF_INET, &reinterpret_cast<const sockaddr_in* const>(address)->sin_addr, buffer, sizeof(buffer));
};

// Helper to convert to IPv6
std::string convertIntoIPv6(const sockaddr* const address) {
	char buffer[46] = {};
	return ::InetNtop(AF_INET6, &reinterpret_cast<const sockaddr_in6* const>(address)->sin6_addr, buffer, sizeof(buffer));
};

WindowsTcpSocket::WindowsTcpSocket() :
	socket(INVALID_SOCKET) {
	WSAData wsaData;
	const auto result = ::WSAStartup(MAKEWORD(2, 2), &wsaData);

	if (result != 0) {
		std::cerr << "WSAStartup failed with code: " << result << std::endl;
	} else {
		std::cout << "WinSock startup successful" << std::endl;
	}
}

WindowsTcpSocket::~WindowsTcpSocket() {
	const auto result = ::WSACleanup();

	if (result == SOCKET_ERROR) {
		std::cerr << "WSACleanup failed with code: " << WSAGetLastError() << std::endl;
	} else {
		std::cout << "WinSock cleanup successful" << std::endl;
	}
}

bool WindowsTcpSocket::open(const std::string_view& hostName, std::uint32_t port) {
	ADDRINFO hints;
	PADDRINFOA addrinfo;

	::ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;

	// Address information will be deallocated in WindowsTcpSocket::close()
	const auto successCode = ::GetAddrInfo(hostName.data(), std::to_string(port).c_str(), &hints, &addrinfo);

	if (successCode != 0) {
		std::cerr << "Failed to get address info with error code: " << successCode << std::endl;
		::freeaddrinfo(addrinfo);
		return false;
	}

	std::cout << "Successfully translated host name into IP address" << std::endl;

	// Some IP addresses may not work so it is important to keep trying until no more addresses are available
	for (auto ptr = addrinfo; ptr != nullptr; ptr = ptr->ai_next) {
		const auto isValidFamily = ptr->ai_family == AF_INET || ptr->ai_family == AF_INET6;
		const auto isValidType = ptr->ai_socktype == SOCK_STREAM;
		const auto isValidProtocol = ptr->ai_protocol == IPPROTO_TCP;

		// Only look for addresses that work over TCP using IPv4 / IPv6
		if (isValidFamily && isValidType && isValidProtocol) {
			const auto address = ptr->ai_family == AF_INET
				? convertIntoIPv4(ptr->ai_addr)
				: convertIntoIPv6(ptr->ai_addr);

			std::cout << "Converted " << hostName << " into " << address << std::endl;

			socket = ::socket(
				ptr->ai_family,
				ptr->ai_socktype,
				ptr->ai_protocol);

			if (socket == INVALID_SOCKET) {
				std::cerr << "Unable to create socket: " << WSAGetLastError() << std::endl;
				continue;
			}

			if (::connect(socket, ptr->ai_addr, static_cast<int>(ptr->ai_addrlen)) == SOCKET_ERROR) {
				std::cerr << "Unable to connect to the server using the newly created socket: " << WSAGetLastError() << std::endl;
				continue;
			}

			// Success
			::freeaddrinfo(addrinfo);
			return true;
		}
	}

	// Failure - cannot recover from this
	std::cerr << "Exhausted all IP addresses - none could be used to establish a connection to the server" << std::endl;
	::freeaddrinfo(addrinfo);
	return false;
}

bool WindowsTcpSocket::send(const std::string_view& payload) {
	std::cout << "Sending payload..." << std::endl;
	const auto result = ::send(socket, payload.data(), static_cast<int>(payload.length()), 0);

	if (result == SOCKET_ERROR) {
		std::cerr << "Send failed: " << WSAGetLastError() << std::endl;
		return false;
	}

	std::cout << "Successfully sent payload" << std::endl;
	return true;
}

bool WindowsTcpSocket::receive() {
	{
		auto result = -1;

		do {
			result = recv(socket, reinterpret_cast<char*>(receiveBuffer), sizeof(receiveBuffer), 0);

			if (result > 0) {
				std::cout << "Bytes received: " << result << std::endl;
			} else if (result == 0) {
				std::cout << "Connection closed by server" << std::endl;
			} else {
				std::cerr << "Receive failed: " << WSAGetLastError() << std::endl;
				return false;
			}
		} while (result > 0);
	}

	return true;
}

bool WindowsTcpSocket::changeState(const SocketStateChange state) {
	auto result = SOCKET_ERROR;

	switch (state) {
		case SocketStateChange::CloseForSend:
			result = ::shutdown(socket, SD_SEND);
			break;

		case SocketStateChange::CloseForReceive:
			result = ::shutdown(socket, SD_RECEIVE);
			break;

		default:
			std::cerr << "Unknown socket state change - this should never happen" << std::endl;
			break;
	}

	if (result == SOCKET_ERROR) {
		std::cerr << "Socket shutdown failed: " << WSAGetLastError() << std::endl;
	}

	return result != SOCKET_ERROR;
}

void WindowsTcpSocket::close() {
	::closesocket(socket);
	socket = INVALID_SOCKET;
}

#endif // !WIN32
