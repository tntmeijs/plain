#ifndef NETWORK_TCP_SOCKET_STATE_CHANGE_HPP
#define NETWORK_TCP_SOCKET_STATE_CHANGE_HPP

namespace network::tcp {

	enum class SocketStateChange {
		// Socket will no longer be able to send data
		CloseForSend,

		// Socket will no longer be able to receive data
		CloseForReceive,

		// Socket will no longer be able to send nor receive data
		CloseForSendAndReceive
	};

}

#endif // !NETWORK_TCP_SOCKET_STATE_CHANGE_HPP
