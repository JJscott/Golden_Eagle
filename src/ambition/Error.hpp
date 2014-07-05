namespace ambition {
	

	namespace error {
		enum network_errors {
			NETERR_SOCKET_CREATE_FAILURE = 0,
			NETERR_SOCKET_SET_BLOCKING_FAILURE,
			NETERR_SELECT_FAILURE,
			NETERR_CONNECT_FAILURE,
			NETERR_RESOLVE_FAILURE
		};
	}
	class network_error : public std::runtime_error {
	public:
		error::network_errors type;
		int error_no;
		std::string error_message;
		network_error(error::network_errors et, std::string msg) : std::runtime_error(msg), type(et) {}
	};
}