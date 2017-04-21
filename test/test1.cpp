
#include <quippy/connector.hpp>
#include <gtest/gtest.h>

namespace asio = quippy::asio;

using proto = asio::ip::tcp;

TEST(cock_test, first)
{
    asio::io_service ios;

    proto::resolver resolv(ios);
    auto first_addr = resolv.resolve(proto::resolver::query(proto::v4(), "localhost", "5672"));
    auto conn = quippy::connector(ios);
    conn.connect(first_addr);
}