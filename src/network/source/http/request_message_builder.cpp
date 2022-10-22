#include "http/request_message_builder.hpp"

using namespace network::http;

constexpr const char* const HTTP_1_1 = "HTTP/1.1";
constexpr const char* const HEADER_CONTENT_LENGTH = "Content-Length";

RequestMessageBuilder::RequestMessageBuilder() {}

void RequestMessageBuilder::reset() {
	request = RequestMessage();
}

RequestMessageBuilder& RequestMessageBuilder::withHttpMethod(const HttpMethod& httpMethod) {
	request.method = HttpMethods::toString(httpMethod);
	return *this;
}

RequestMessageBuilder& RequestMessageBuilder::withBody(const std::string_view& body) {
	request.body = body;
	return *this;
}

RequestMessageBuilder& network::http::RequestMessageBuilder::withHeader(const Header& header, const std::string_view& value) {
	request.headers.emplace(Headers::toString(header), value);
	return *this;
}

RequestMessageBuilder& RequestMessageBuilder::withHost(const std::string_view& host) {
	withHeader(Header::Host, host);
	return *this;
}

RequestMessageBuilder& RequestMessageBuilder::withTarget(const std::string_view& target) {
	this->request.target = target;
	return *this;
}

RequestMessage network::http::RequestMessageBuilder::build() {
	// Content length header should be added if a body is present and the header does not exist yet
	if (request.body.length() > 0 && !request.headers.contains(HEADER_CONTENT_LENGTH)) {
		withHeader(Header::ContentLength, std::to_string(request.body.length()));
	}

	// Default to HTTP 1.1 if no version has been set
	if (request.version.empty()) {
		request.version = HTTP_1_1;
	}

	return request;
}
