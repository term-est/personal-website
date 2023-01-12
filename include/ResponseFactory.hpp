#include <boost/asio.hpp>
#include <boost/beast.hpp>

#include <filesystem>
#include <optional>
#include <map>
#include <string_view>
#include <iostream>

namespace asio = boost::asio;
namespace beast = boost::beast;


namespace ResponseFactory
{

namespace
{


const std::map<std::filesystem::path, std::string_view> mime_map{
		{{"mp4", "video/mp4"},
		 {"webm", "video/webm"},
		 {"png", "image/png"},
		 {"jpeg", "image/jpeg"},
		 {"jpg", "image/jpg"},
		 {"html", "image/jpg"},
		 {"ico", "image/x-icon"}}
};
}

auto handle_get(const std::filesystem::path& path)
-> std::optional<std::shared_ptr<beast::http::response<beast::http::file_body>>>
{
	if (!std::filesystem::exists(path))
		return std::nullopt;

	auto response = std::make_shared<beast::http::response<beast::http::file_body>>();

	beast::error_code ec;
	beast::http::file_body::value_type data;

	// we need a char, not a wchar. hence the shenanigans
	data.open(path.string().c_str(), beast::file_mode::read, ec);

	if (ec)
	{
		std::cerr << "Encountered an error during file read operation: " << ec.message() << " path: " << path
				  << std::endl;
		return std::nullopt;
	}

	response->body() = std::move(data);

	if (auto itr = mime_map.find(path.extension()); itr != mime_map.end())
		response->set("Content-Type", itr->second.data());

	else
		response->set("Content-Type", "text/html; charset=utf-8");


	return response;
}

} // end of namespace ResponseFactory
