//
// Created by Richard Hodges on 22/04/2017.
//

#pragma once

namespace quippy { namespace error {

    struct transport_category_name_cache
    {
        mutable std::mutex mutex_;

        std::unordered_map<std::string, int> text_to_code_;
        std::vector<std::string> code_to_text_;

        auto get_lock() const {
            return std::unique_lock<std::mutex>(mutex_);
        }


        int get_code(std::string const& message)
        {
            auto lock = get_lock();
            auto ifind = text_to_code_.find(message);
            if (ifind != text_to_code_.end()) { return ifind->second; }
            auto index = text_to_code_.size();
            code_to_text_.push_back(message);
            text_to_code_.emplace(message, index);
            return index;
        }

        std::string get_name_no_error(std::size_t(ev)) const {
            auto lock = get_lock();
            if (ev >= int(code_to_text_.size())) {
                return "unregistered error code: " + std::to_string(ev);
            }
            return code_to_text_[ev];
        }

        auto operator[](std::size_t index) const {
            return get_name_no_error(index);
        }

        static auto common() -> transport_category_name_cache&
        {
            static transport_category_name_cache cache;
            return cache;
        }
    };


    struct transport_category_type : asio::error_category
    {
        transport_category_type(transport_category_name_cache& cache = transport_category_name_cache::common())
            : asio::error_category()
        , name_cache_(cache)
        {

        }

        const char *     name() const noexcept override
        {
            return "quippy::transport";
        }

        std::string      message( int ev ) const override {
            if (ev >= 0)
                return name_cache_[ev];
            else
                throw std::out_of_range("transport_category::message - code is negative: " + std::to_string(ev));
        }

        int get_code(const std::string message) const {
            return name_cache_.get_code(message);
        }

        transport_category_name_cache& name_cache_;
    };

    inline auto transport_category() -> const transport_category_type&
    {
        static transport_category_type global;
        return global;
    }


}}