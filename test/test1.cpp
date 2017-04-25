
#include <quippy.hpp>
#include <gtest/gtest.h>

#include <random>
#include <algorithm>
#include <iomanip>
#include <iostream>

namespace asio = quippy::asio;

using proto = asio::ip::tcp;

struct error_category_tests : ::testing::Test
{
    std::default_random_engine eng { 0 };
    std::uniform_int_distribution<char> alpha_dist { 'a', 'z' };

    char random_char()
    {
        return alpha_dist(eng);
    }

    std::string random_string(std::size_t length)
    {
        std::string result;
        std::generate_n(std::back_inserter(result),
                        length,
                        [this] { return this->random_char(); });
        return result;
    }


};


TEST_F(error_category_tests, message_registration)
{
    quippy::error::transport_category_name_cache my_cache;
    const quippy::error::transport_category_type cat(my_cache);

    int foo_code = cat.get_code("foo");
    int foo_code2 = cat.get_code("foo");
    ASSERT_EQ(foo_code2, foo_code);
    auto foo_message = cat.message(foo_code);
    ASSERT_EQ(foo_message, "foo");

    std::vector< std::pair<std::string, int> > mappings;
    for (int i = 0 ; i < 100000 ; ++i)
    {
        auto message = random_string(20);
        mappings.emplace_back(message, cat.get_code(message));
    }

    for (auto&& mapping : mappings)
    {
        auto&& message = mapping.first;
        auto&& code = mapping.second;
        ASSERT_EQ(code, cat.get_code(message));
        ASSERT_EQ(message, cat.message(code));
    }

    ASSERT_EQ(cat, cat);

}

struct error_tests : ::testing::Test
{
    using protocol = asio::ip::tcp;

    asio::io_service ios;
    proto::resolver resolv { ios };

    AMQP::Login good_login { "test", "test" };
    AMQP::Login bad_login { "test", "wrong_password" };
    std::string good_vhost { "/" };

};

TEST_F(error_tests, halts_from_idle)
{
    auto conn = quippy::connector(ios);
    conn.halt();
}

struct error_code_dump
{
    friend std::ostream& operator<<(std::ostream& os, error_code_dump const& d)
    {
        os << "code: " << d.ec.value();
        os << " message: " << std::quoted(d.ec.message());
        os << " category: " << std::quoted(d.ec.category().name());
        return os;
    }
    asio::error_code const& ec;
};

auto dump(asio::error_code const& ec)
{
    return error_code_dump { ec };
}

::testing::AssertionResult same_error(const asio::error_code& expected, const asio::error_code& actual)
{
    if (actual == expected) {
        return ::testing::AssertionSuccess();
    }
    else {
        return ::testing::AssertionFailure() << "error code mismatch:\n"
                                             "expected: " << dump(expected) << "\n"
                                                          "actual: " << dump(actual);
    }

}

TEST_F(error_tests, error_if_no_transport)
{
    auto conn = quippy::connector(ios);

    asio::error_code ec;
    conn.connect(good_login, good_vhost, ec);
    EXPECT_TRUE(same_error(make_error_code(quippy::error::impl_state::transport_down), ec));

}

TEST_F(error_tests, error_if_halted)
{
    auto conn = quippy::connector(ios);
    conn.halt();

    asio::error_code ec;
    auto first_addr = resolv.resolve(proto::resolver::query(proto::v4(), "localhost", "5672"));
    conn.connect_link(first_addr, ec);
    EXPECT_TRUE(same_error(make_error_code(quippy::error::impl_state::halted), ec));

    try {
        conn.connect_link(first_addr);
        EXPECT_TRUE(false) << "should throw";
    }
    catch(const asio::system_error& se)
    {
        EXPECT_TRUE(same_error(make_error_code(quippy::error::impl_state::halted), se.code())) << se.what();
    }
    catch(const std::exception& e) {
        EXPECT_TRUE(false) << "wrong exception" << e.what();
    }

    conn.connect(good_login, good_vhost, ec);
    EXPECT_TRUE(same_error(quippy::error::impl_state::halted, ec));
}


TEST_F(error_tests, destroy_on_connect_sync)
{
    auto first_addr = resolv.resolve(proto::resolver::query(proto::v4(), "localhost", "5672"));
    auto conn = quippy::connector(ios);
    conn.connect_link(first_addr);
}

TEST_F(error_tests, destroy_on_connect_async)
{
    proto::resolver resolv(ios);

    asio::error_code ec;
    int called = 0;
    auto handler = [&](auto&& error) {
        ec = error;
        ++called;
    };

    auto first_addr = resolv.resolve(proto::resolver::query(proto::v4(), "localhost", "5672"));
    {
        auto conn = quippy::connector(ios);
        conn.async_connect_link(first_addr, handler);
        ASSERT_NO_THROW(ios.run_one(););
    }
    ASSERT_EQ(1, called);

}

TEST(cock_test, first)
{
    asio::io_service ios;

    proto::resolver resolv(ios);
    auto first_addr = resolv.resolve(proto::resolver::query(proto::v4(), "localhost", "5672"));
    auto conn = quippy::connector(ios);
    conn.connect_link(first_addr);
    conn.connect(AMQP::Login("test", "test"), "/");
}

TEST_F(error_tests, incorrect_credentials)
{
    auto first_addr = resolv.resolve(proto::resolver::query(proto::v4(), "localhost", "5672"));
    auto conn = quippy::connector(ios);
    EXPECT_NO_THROW(conn.connect_link(first_addr));
    asio::error_code ec;
    conn.connect(bad_login, good_vhost, ec);
    EXPECT_TRUE(ec) << dump(ec);
    EXPECT_EQ(quippy::error::transport_category(), ec.category());



}