/// @copyright Copyright (c) 2017 Tim Niederhausen (tim@rnc-ag.de)
/// Distributed under the Boost Software License, Version 1.0.
/// (See accompanying file LICENSE_1_0.txt or copy at
/// http://www.boost.org/LICENSE_1_0.txt)

#ifndef ASIOEXT_IMPL_CONNECT_HPP
#define ASIOEXT_IMPL_CONNECT_HPP

#include "asioext/composed_operation.hpp"

#if defined(ASIOEXT_USE_BOOST_ASIO)
# include <boost/asio/connect.hpp>
#else
# include <asio/connect.hpp>
#endif

ASIOEXT_NS_BEGIN

namespace detail {

template <class Handler>
class connect_op : public composed_operation<Handler>
{
public:
  connect_op(Handler& handler,
             asio::ip::tcp::socket::lowest_layer_type& socket,
             asio::ip::tcp::resolver& resolver,
             const asio::ip::tcp::resolver::query& q)
    : composed_operation<Handler>(ASIOEXT_MOVE_CAST(Handler)(handler))
    , socket_(socket)
    , resolver_(resolver)
    , state_(0)
  {
    // Open the socket to give the caller something to close to cancel the
    // asynchronous operation.
    error_code ec;
    socket_.open(asio::ip::tcp::v4(), ec);
    if (ec) {
      socket_.get_io_service().post(asio::detail::bind_handler(
          handler_, ec, asio::ip::tcp::resolver::iterator()));
      return;
    }
    resolver_.async_resolve(q, ASIOEXT_MOVE_CAST(connect_op)(*this));
  }

  void operator()(error_code ec, asio::ip::tcp::resolver::iterator iter);

private:
  asio::ip::tcp::socket::lowest_layer_type& socket_;
  asio::ip::tcp::resolver& resolver_;
  int state_;
};

template <class Handler>
void connect_op<Handler>::operator()(error_code ec,
                                     asio::ip::tcp::resolver::iterator iter)
{
  if (ec) {
    handler_(ec, asio::ip::tcp::resolver::iterator());
    return;
  }
  switch (state_) {
    case 0: {
      state_ = 1;
      asio::async_connect(socket_, iter, ASIOEXT_MOVE_CAST(connect_op)(*this));
      return;
    }
    case 1: {
      handler_(ec, iter);
      return;
    }
  }
}

}

template <typename ComposedConnectHandler>
ASIOEXT_INITFN_RESULT_TYPE(ComposedConnectHandler,
                           void(error_code, asio::ip::tcp::resolver::iterator))
async_connect(asio::ip::tcp::socket::lowest_layer_type& socket,
              asio::ip::tcp::resolver& resolver,
              const asio::ip::tcp::resolver::query& q,
              ASIOEXT_MOVE_ARG(ComposedConnectHandler) handler)
{
  typedef async_completion<
    ComposedConnectHandler,
    void (error_code, asio::ip::tcp::resolver::iterator)
  > completion;

  typedef detail::connect_op<completion::handler_type> operation;

  completion init(handler);
  operation op(init.completion_handler, socket, resolver, q);
  return init.result.get();
}

ASIOEXT_NS_END

#endif