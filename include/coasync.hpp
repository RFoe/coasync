#ifndef COASYNC_COASYNC_INCLUDED
#define COASYNC_COASYNC_INCLUDED

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include "coasync/co_spawn.hpp"
#include "coasync/co_bind.hpp"
#include "coasync/co_signal.hpp"
#include "coasync/co_mutex.hpp"
#include "coasync/co_condition_variable.hpp"
#include "coasync/co_latch.hpp"
#include "coasync/co_semaphore.hpp"

#include "coasync/async_fn.hpp"
#include "coasync/awaitable.hpp"
#include "coasync/generator.hpp"
#include "coasync/channel.hpp"
#include "coasync/execution_context.hpp"
#include "coasync/functional.hpp"
#include "coasync/set_stop_source.hpp"
#include "coasync/this_coro.hpp"
#include "coasync/when_all.hpp"
#include "coasync/when_any.hpp"
#include "coasync/deadline_timer.hpp"

#include "coasync/net/acceptor.hpp"
#include "coasync/net/address.hpp"
#include "coasync/net/address_v4.hpp"
#include "coasync/net/address_v6.hpp"
#include "coasync/net/address_v4_iterator.hpp"
#include "coasync/net/address_v6_iterator.hpp"
#include "coasync/net/endpoint.hpp"
#include "coasync/net/message_flags.hpp"
#include "coasync/net/option.hpp"
#include "coasync/net/port.hpp"
#include "coasync/net/protocol.hpp"
#include "coasync/net/receive.hpp"
#include "coasync/net/resolver.hpp"
#include "coasync/net/resolver_flags.hpp"
#include "coasync/net/scope_id.hpp"
#include "coasync/net/send.hpp"
#include "coasync/net/serde_stream.hpp"
#include "coasync/net/socket.hpp"
#include "coasync/net/socket_base.hpp"
#include "coasync/net/network_v4.hpp"
#include "coasync/net/network_v6.hpp"

#include "coasync/rpc/rpc_client.hpp"
#include "coasync/rpc/rpc_server.hpp"

#endif
