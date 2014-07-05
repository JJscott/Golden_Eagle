namespace ambition {
	

	namespace error {
		enum network_errors {
			neterr_socket_create_failure,
			neterr_socket_set_blocking_failure,
			neterr_select_failure,
			neterr_connect_failure,
			neterr_resolve_failure
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