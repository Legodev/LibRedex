//
// Copyright (c) 2017 Christopher M. Kohlhoff (chris at kohlhoff dot com)
// Modified by Legodev <legodevgit@mailbox.org>
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/boostorg/beast
//

#include "rest/http_connection.hpp"
#include "rest/handle_callback.hpp"

// Asynchronously receive a complete request message.
void http_connection::read_request()
{
	auto self = shared_from_this();

	http::async_read(
			socket_,
			buffer_,
			request_,
			[self](beast::error_code ec,
					std::size_t bytes_transferred)
			{
				boost::ignore_unused(bytes_transferred);
				if(!ec)
					self->process_request();
			});
}

// Determine what needs to be done with the request message.
void http_connection::process_request()
{
	response_.version(request_.version());
	response_.keep_alive(false);

	switch(request_.method())
	{
		case http::verb::get:
			//response_.result(http::status::ok);
			//response_.set(http::field::server, "Beast");

			response_.result(http::status::not_found);
			response_.set(http::field::content_type, "text/plain");
			beast::ostream(response_.body()) << "File not found\r\n";

			break;

		case http::verb::post: response_.result(http::status::ok);
			response_.set(http::field::server, "Beast");
			handle_post();
			break;

		default:
			// We return responses indicating an error if
			// we do not recognize the request method.
			response_.result(http::status::bad_request);
			response_.set(http::field::content_type, "text/plain");
			beast::ostream(response_.body())
					<< "Invalid request-method '"
					<< std::string(request_.method_string())
					<< "'";
			break;
	}

	write_response();
}

void http_connection::handle_post()
{
	std::string url = request_.target().to_string();
	std::string data = boost::beast::buffers_to_string(request_.body().data());

	handle_callback(url, data);

	response_.set(http::field::content_type, "text/html");
	beast::ostream(response_.body())
			<< "<html>\n"
			<< "<head><title>Dataview</title></head>\n"
			<< "<body>\n"
			<< "<h1>Data:</h1>\n"
			<< "<p>"
			<< data
			<< "</p>\n"
			<< "</body>\n"
			<< "</html>\n";

}

// Asynchronously transmit the response message.
void http_connection::write_response()
{
	auto self = shared_from_this();

	response_.set(http::field::content_length, response_.body().size());

	http::async_write(
			socket_,
			response_,
			[self](beast::error_code ec, std::size_t)
			{
				self->socket_.shutdown(tcp::socket::shutdown_send, ec);
				self->deadline_.cancel();
			});
}

// Check whether we have spent enough time on this connection.
void http_connection::check_deadline()
{
	auto self = shared_from_this();

	deadline_.async_wait(
			[self](beast::error_code ec)
			{
				if(!ec)
				{
					// Close socket to cancel any outstanding operation.
					self->socket_.close(ec);
				}
			});
}