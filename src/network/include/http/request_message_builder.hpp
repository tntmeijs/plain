#ifndef NETWORK_HTTP_REQUEST_MESSAGE_BUILDER_HPP
#define NETWORK_HTTP_REQUEST_MESSAGE_BUILDER_HPP

#include "header.hpp"
#include "http_method.hpp"
#include "request_message.hpp"

#include <string_view>

namespace network::http {

	// Builder for request message objects
	class RequestMessageBuilder final {
	public:
		// Create a new builder
		RequestMessageBuilder();

		// Reset the builder
		void reset();

		// Add an HTTP method type
		RequestMessageBuilder& withHttpMethod(const HttpMethod& httpMethod);

		// Add a payload / body to the request
		RequestMessageBuilder& withBody(const std::string_view& body);

		// Add a header to the request
		RequestMessageBuilder& withHeader(const Header& header, const std::string_view& value);

		// Add a host to the request
		RequestMessageBuilder& withHost(const std::string_view& host);

		// Add a target to the request (usually a URL / absolute path of the protocol)
		RequestMessageBuilder& withTarget(const std::string_view& target);

		// Build the request message (defaults to HTTP 1.1 unless otherwise specified)
		RequestMessage build();

	private:
		RequestMessage request;
	};

}

#endif // !NETWORK_HTTP_REQUEST_MESSAGE_BUILDER_HPP
