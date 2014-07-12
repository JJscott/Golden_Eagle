#include <ambition/ByteBuffer.hpp>
#include <ambition/Concurrent.hpp>
#include <ambition/ClientSocket.hpp>
#include <exception>

namespace ambition {
	class http_result {
		const byte_t *body_i;
		std::string body_i_s;
	public:
		http_result(const byte_t* b, size_t sz) {
			std::cout << __LINE__ << std::endl;
			//memcpy(body_i, b, sz); // the fuck are you doing with an unitialized pointer?
			body_i = b;
			body_i_s = std::string(reinterpret_cast<const char *>(body_i), sz);
			std::cout << __LINE__ << std::endl;
		}
		const byte_t* body() { return body_i; }
		std::string body_s() { return body_i_s; }
	};


	class http_request {
		http_result* result;
		
	public:		
		Event<http_result> on_complete;

		virtual std::string generate_request() const =0;
		void complete_request(const byte_t* data, size_t sz) {
			std::cout << __LINE__ << std::endl;
			result = new http_result(data, sz);
			std::cout << __LINE__ << std::endl;
			on_complete.notify(*result);
			std::cout << __LINE__ << std::endl;
		}

		http_result* reply() { return result; }
	};

	class http_get_request : public http_request {
		std::string page_req;

		std::string generate_request() const override {
			return "GET " + page_req;
		}
	public:
		http_get_request(std::string page) : page_req(page) {}
	};

	class http_connection {
		std::string destination;
		uint16_t port;
		bool con_ready = false;
		ClientSocket sock;
		http_request* current_request = nullptr;


		bool connected(const SocketResult &sr) {
			con_ready = sr.success;
			return false;
		}

		bool recieved(const SocketResult &sr) {
			std::cout << __LINE__ << std::endl;
			if(current_request == nullptr) 
				throw std::runtime_error("no request in progress");
			std::cout << __LINE__ << std::endl;
			//byte_buffer::reader r = sr.data.read();
			//byte_t* data = new byte_t(r.size()); // this was the main problem: () instead of []
			//r.get_array(data, r.size());
			std::cout << __LINE__ << std::endl;
			current_request->complete_request(sr.data.data(), sr.data.size());
			std::cout << __LINE__ << std::endl;
			return false;
		}

		void init() {	
			sock.on_connected.attach([this](const SocketResult &sr) { return this->connected(sr); } );
			sock.on_recieved.attach([this](const SocketResult &sr) { std::cout << __LINE__ << std::endl; this->recieved(sr); std::cout << __LINE__ << std::endl; return false; } );
		}
	public:
		http_connection(std::string dst) : destination(dst), port(80) { init(); }
		http_connection(std::string dst, uint16_t pt) : destination(dst), port(pt) { init(); }

		void open() {
			sock.begin_connect(destination, port, 5000);
			sock.on_connected.wait();
		}

		void execute(http_request *req) {
			if(!con_ready)
				throw std::runtime_error("http connection not ready");
			if(current_request != nullptr)
				throw std::runtime_error("http connection already processing one request");
			current_request = req;
			byte_buffer b;
			b << "Host: " << destination << "\r\n";
			b << req->generate_request() << "\r\n\r\n";
			sock.begin_send(b);
		}

		void close() {

		}
	};
}