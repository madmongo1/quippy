//
// Created by Richard Hodges on 25/04/2017.
//

#pragma once

#include <quippy/config.hpp>
#include <vector>
#include <cassert>

namespace quippy { namespace detail {


    struct send_buffer {

        send_buffer()
        : send_buffer_()
        , sending_buffer_()
        , sending_(false)
        {}

        void reset()
        {
            send_buffer_.clear();
            sending_buffer_.clear();
            sending_ = false;
        }

        void put_data(const char *data, std::size_t length) {
            send_buffer_.insert(std::end(send_buffer_),
                                data,
                                data + length);
        }

        bool sending() const { return sending_; }

        bool has_pending_data() const { return not send_buffer_.empty(); }

        auto prepare() -> asio::const_buffers_1 {
            assert(!sending_);
            assert(sending_buffer_.empty());
            std::swap(send_buffer_, sending_buffer_);
            sending_ = true;
            return buffer();
        }

        auto buffer() const -> asio::const_buffers_1 {
            return asio::buffer(sending_buffer_);
        }

        void consume(std::size_t sent) {
            assert(sending_);
            sending_ = false;
            assert(sent <= sending_buffer_.size());
            auto diff = sending_buffer_.size() - sent;
            if (diff) {
                auto first = std::begin(sending_buffer_) + sent;
                auto last = std::end(sending_buffer_);
                send_buffer_.insert(std::begin(send_buffer_),
                                    first, last);
            }
            sending_buffer_.clear();
        }

    private:
        std::vector<char> send_buffer_;
        std::vector<char> sending_buffer_;
        bool sending_;
    };

}}