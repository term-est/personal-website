#include <iostream>
#include <thread>

#include "Server.hpp"


void make_server(asio::ip::tcp::acceptor& acceptor, asio::ip::tcp::socket& socket)
{
	acceptor.async_accept(socket, [&](beast::error_code ec)
	{
		if (!ec)
			std::make_shared<Server>(std::move(socket))->start();

		// use after move is okay for some dumb reason
		// don't blame me, this was in the official boost examples
		make_server(acceptor, socket);
	});
};

auto main() -> int try
{
	auto server = []
	{
		asio::io_context context{};
		asio::ip::tcp::acceptor acceptor{context, asio::ip::tcp::endpoint{asio::ip::make_address_v4("0.0.0.0"), 80}};
		asio::ip::tcp::socket socket{context};
		make_server(acceptor, socket);

		context.run();
	};

	server();
}
catch (std::exception& e)
{
	std::cerr << e.what();
}
