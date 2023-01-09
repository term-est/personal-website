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
const std::map<std::wstring_view, std::string_view> mime_map{
		{{L"mp4", "video/mp4"},
		 {L"webm", "video/webm"},
		 {L"png", "image/png"},
		 {L"jpeg", "image/jpeg"},
		 {L"jpg", "image/jpg"},
		 {L"html", "image/jpg"},
		 {L"ico", "image/x-icon"}}
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

	if (auto itr = mime_map.find(path.extension().c_str()); itr != mime_map.end())
		response->set("Content-Type", itr->second.data());

	else
		response->set("Content-Type", "text/html; charset=utf-8");


	return response;
}


}