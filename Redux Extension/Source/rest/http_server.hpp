//
// Copyright (c) 2017 Christopher M. Kohlhoff (chris at kohlhoff dot com)
// Modified by Legodev <legodevgit@mailbox.org>
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/boostorg/beast
//

#ifndef REDEX_REST_HTTP_SERVER_HPP
#define REDEX_REST_HTTP_SERVER_HPP

#include "rest/http_connection.hpp"

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

#endif //REDEX_REST_HTTP_SERVER_HPP
