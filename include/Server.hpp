#pragma once

#include <memory>
#include <chrono>
#include <string>
#include <filesystem>
#include <fstream>

#include <boost/asio.hpp>
#include <boost/beast.hpp>

#include "ResponseFactory.hpp"

namespace asio = boost::asio;
namespace beast = boost::beast;

using namespace std::chrono_literals;

class Server : public std::enable_shared_from_this<Server>
{
	asio::ip::tcp::socket m_socket;

	beast::http::response<beast::http::dynamic_body> m_response{};
	beast::http::request<beast::http::dynamic_body> m_request{};
	beast::net::steady_timer m_deadline{m_socket.get_executor(), 2s};

public:

	explicit Server(asio::ip::tcp::socket socket) noexcept : m_socket{std::move(socket)}
	{}

	void start()
	{
		asyncRead();
		asyncDeadline();
	}

private:

	void asyncRead()
	{
		auto buffer = std::make_shared<beast::flat_static_buffer<2048>>();
		beast::http::async_read(m_socket, *buffer, m_request, [self = shared_from_this(), buffer](beast::error_code ec, std::size_t transfer_size)
		{
			self->receiveHandler(buffer, transfer_size);
		});
	}

	void receiveHandler(std::shared_ptr<beast::flat_static_buffer<2048>> buffer, std::size_t transfer_size)
	{
		if (m_request.method() == beast::http::verb::get)
		{
			std::string req_path = m_request.target().to_string();

			std::cout << req_path << std::endl;
			if (req_path == "/")
				req_path += "index.html";

			auto response_opt = ResponseFactory::handle_get(std::filesystem::path("../res" + req_path));

			if (!response_opt.has_value())
			{
				auto not_found = std::make_shared<beast::http::response<beast::http::dynamic_body>>();
				not_found->result(beast::http::status::not_found);
				not_found->version(m_request.version());
				not_found->keep_alive(false);
				asyncResponse(not_found);

				return;
			}

			auto& response = response_opt.value();
			response->version(m_request.version());
			response->keep_alive(false);
			response->set(beast::http::field::server, "Strawberry");
			response->content_length(response->body().size());

			asyncResponse(response);
		}
	}

	template <typename Body>
	void asyncResponse(std::shared_ptr<beast::http::response<Body>> response_index)
	{
		beast::http::async_write(m_socket, *response_index, [self = shared_from_this(), response_index](beast::error_code ec, std::size_t transfer_size)
		{
			if (ec)
			{
				std::cerr << "in async write: " << ec.message() << std::endl;
				return;
			}

			self->m_deadline.cancel();
			if (!response_index->keep_alive())
			{
				self->m_socket.shutdown(asio::ip::tcp::socket::shutdown_send);
				self->m_socket.close();
			}

		});
	}

	void asyncDeadline()
	{
		m_deadline.async_wait([self = shared_from_this()](beast::error_code ec)
		{
			if (!ec)
			{
				self->m_socket.cancel();
				self->m_socket.close();
			}

		});
	}

};