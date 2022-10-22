#include "http/request_message.hpp"

using namespace network::http;

// Reference: https://www.rfc-editor.org/rfc/rfc2616#section-2.2
constexpr const char* const LINE_BREAK = "\r\n";

std::string RequestMessage::generate() const {
	// Start line
	auto data = method + ' ' + target + ' ' + version + LINE_BREAK;

	// Headers
	for (const auto& entry : headers) {
		data += entry.first + ": " + entry.second + LINE_BREAK;
	}

	// Blank line to indicate all meta-information for the request has been sent
	data += LINE_BREAK;

	// Optional body
	data += body;

	return data;
}
