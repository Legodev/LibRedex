//
// Copyright (c) 2017 Christopher M. Kohlhoff (chris at kohlhoff dot com)
// Modified by Legodev <legodevgit@mailbox.org>
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/boostorg/beast
//

#ifndef REDEX_REST_HTTP_CONNECTION_HPP
#define REDEX_REST_HTTP_CONNECTION_HPP

#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/version.hpp>
#include <boost/asio.hpp>
#include <chrono>
#include <cstdlib>
#include <ctime>
#include <iostream>
#include <memory>
#include <string>

namespace beast = boost::beast;         // from <boost/beast.hpp>
namespace http = beast::http;           // from <boost/beast/http.hpp>
namespace net = boost::asio;            // from <boost/asio.hpp>
using tcp = boost::asio::ip::tcp;       // from <boost/asio/ip/tcp.hpp>

class http_connection: public std::enable_shared_from_this<http_connection> {
public:
	http_connection(tcp::socket socket)
			:socket_(std::move(socket))
	{
	}

	// Initiate the asynchronous operations associated with the connection.
	void
	start()
	{
		read_request();
		check_deadline();
	}

private:
	// The socket for the currently connected client.
	tcp::socket socket_;

	// The buffer for performing reads.
	beast::flat_buffer buffer_{8192};

	// The request message.
	http::request<http::dynamic_body> request_;

	// The response message.
	http::response<http::dynamic_body> response_;

	// The timer for putting a deadline on connection processing.
	net::basic_waitable_timer<std::chrono::steady_clock> deadline_{
			socket_.get_executor(), std::chrono::seconds(60)};

	// Asynchronously receive a complete request message.
	void read_request();

	// Determine what needs to be done with the request message.
	void process_request();

	// Handle the post method
	void handle_post();

	// Asynchronously transmit the response message.
	void write_response();

	// Check whether we have spent enough time on this connection.
	void check_deadline();

};
/*
void http_server(tcp::acceptor& acceptor, tcp::socket& socket)
{
	acceptor.async_accept(socket,
			[&](beast::error_code ec)
			{
				if(!ec)
					std::make_shared<http_connection>(std::move(socket))->start();
				http_server(acceptor, socket);
			});
}
*/
#endif //REDEX_REST_HTTP_CONNECTION_HPP
