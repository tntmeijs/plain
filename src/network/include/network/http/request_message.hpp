#ifndef NETWORK_HTTP_REQUEST_MESSAGE_HPP
#define NETWORK_HTTP_REQUEST_MESSAGE_HPP

#include <map>
#include <string>

namespace network::http {

	// Represents an HTTP request message
	class RequestMessage final {
	public:
		friend class RequestMessageBuilder;

		// Combine all data in the HTTP request message into a single message
		std::string generate() const;

	private:
		// Should not be instantiated directly - please use "RequestMessageBuilder" instead
		RequestMessage() = default;

	private:
		// HTTP method type to use for this request
		std::string method;

		// Target of this request (usually a URL)
		std::string target;

		// HTTP version
		std::string version;

		// The request's body (a.k.a. "payload")
		std::string body;

		// All headers that should be included in the request
		std::map<std::string, std::string> headers;
	};

}

#endif // !NETWORK_HTTP_REQUEST_MESSAGE_HPP
