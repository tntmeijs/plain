#ifndef NETWORK_HTTP_METHOD_HPP
#define NETWORK_HTTP_METHOD_HPP

#include <iostream>

namespace network::http {

	enum class HttpMethod {
		// Requests a representation of the specified resource
		// GET requests should only retrieve data
		Get,

		// Asks for a response identical to a GET request,
		// but without the response body
		Head,

		// Submits an entity to the specified resource, often causing
		// a change in state or side effects on the server
		Post,

		// Replaces all current representations of the target resource
		// with the request payload
		Put,

		// Deletes the specified resource
		Delete,

		// Establishes a tunnel to the server identified by the target resource
		Connect,

		// Describes the communication options for the target resource
		Options,

		// Performs a message loop-back test along the path to the target resource
		Trace,

		// Applies partial modifications to a resource
		Patch
	};

	struct HttpMethods final {
		// Convert an HTTP method into its string representation
		static constexpr const char* const toString(const HttpMethod& httpMethod) {
			switch (httpMethod) {
				case HttpMethod::Get:
					return "GET";

				case HttpMethod::Head:
					return "HEAD";

				case HttpMethod::Post:
					return "POST";

				case HttpMethod::Put:
					return "PUT";

				case HttpMethod::Delete:
					return "DELETE";

				case HttpMethod::Connect:
					return "CONNECT";

				case HttpMethod::Options:
					return "OPTIONS";

				case HttpMethod::Trace:
					return "TRACE";

				case HttpMethod::Patch:
					return "PATCH";

				default:
				{
					std::cerr << "Unknown HTTP method" << std::endl;
					exit(1);
				}
			}
		}

	private:
		HttpMethods() = delete;
	};

}

#endif // !NETWORK_HTTP_METHOD_HPP
