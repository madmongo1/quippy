
#include <quippy.hpp>
#include <gtest/gtest.h>

#include <random>
#include <algorithm>

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

TEST(cock_test, first)
{
    asio::io_service ios;

    proto::resolver resolv(ios);
    auto first_addr = resolv.resolve(proto::resolver::query(proto::v4(), "localhost", "5672"));
    auto conn = quippy::connector(ios);
    conn.connect_link(first_addr);
    conn.connect(AMQP::Login("test", "test"), "/");
}