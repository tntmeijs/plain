#ifndef NETWORK_HTTP_HEADER_HPP
#define NETWORK_HTTP_HEADER_HPP

#include "spdlog/spdlog.h"

#include <iostream>

namespace network::http {

	enum class Header {
		// The length of the request body in octets (8-bit bytes)
		ContentLength,

		// The domain name of the server (for virtual hosting), and the TCP port
		// number on which the server is listening. The port number may be omitted
		// if the port is the standard port for the service requested
		// 
		// Mandatory since HTTP/1.1. If the request is generated directly in HTTP/2,
		// it should not be used
		Host,

		// The user agent string of the user agent
		UserAgent
	};

	struct Headers final {
		// Convert a header into its string representation
		static constexpr const char* const toString(const Header& header) {
			switch (header) {
				case Header::ContentLength:
					return "Content-Length";

				case Header::Host:
					return "Host";

				case Header::UserAgent:
					return "User-Agent";

				default:
				{
					spdlog::error("Unknown header specified");
					return "Invalid-Header";
				}
			}
		}

	private:
		Headers() = delete;
	};

}

#endif // !NETWORK_HTTP_HEADER_HPP
