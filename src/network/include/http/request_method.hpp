#ifndef NETWORK_HTTP_METHOD_HPP
#define NETWORK_HTTP_METHOD_HPP

namespace network::http {
	
	enum class RequestMethod {
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

		// Establishes a tunjnel to the server identified by the
		// target resource
		Connect,

		// Describes the communication options for the target resource
		Options,

		// Performs a message loop-back test along the path to the
		// target resource
		Trace,

		// Applies partial modifications to a resource
		Patch
	};

}

#endif // !NETWORK_HTTP_METHOD_HPP
