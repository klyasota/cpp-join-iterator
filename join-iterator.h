/*
 * Copyright (C) 2023 Gleb Bezborodov - All Rights Reserved
 */

#pragma once

#include <type_traits>
#include <stdexcept>
#include <array>

namespace detail {

    struct join_iterator_tag final {};

    template<typename T, size_t Is>
    struct indexed_ref_t {
        explicit indexed_ref_t(const T in_value)
            : value_(std::move(in_value)) {}
    private:
        T value_;
    public:
        const T& get_value() const
        {
            return value_;
        }
    };

    template<typename TValue, typename, typename... TContainer>
    class join_container_impl;

    template<typename TValue, std::size_t... Is, typename... TContainer>
    class join_container_impl<TValue, std::index_sequence<Is...>, TContainer...> : public indexed_ref_t<TContainer, Is>... {
    public:
        template<typename T, size_t I>
        struct iterator_pair_t {
            explicit iterator_pair_t(const std::pair<T, T>& in_pair)
                : begin(in_pair.first)
                , end(in_pair.second) { }

            T begin;
            T end;
        };

        template<typename... TIterator>
        class join_iterator_t;

        template<typename... TIterator>
        class join_iterator_t<std::index_sequence<Is...>, TIterator...> : public iterator_pair_t<TIterator, Is>... {
        public:
            explicit join_iterator_t(const std::pair<TIterator, TIterator>&... in_pair)
                : iterator_pair_t<TIterator, Is>(in_pair)...{}

            join_iterator_t& operator++() {
                const bool end_of_current_iterator_reached = at_number<bool>(active_container_index, [](auto&& iterator)
                    {
                        ++iterator.begin;
                        return iterator.begin == iterator.end;
                    });
                active_container_index += static_cast<uint32_t>(end_of_current_iterator_reached);
                return *this;
            }

            bool operator!=(const join_iterator_t& Rhs) const {
                return active_container_index != Rhs.active_container_index ||
                    (... || !is_same(static_cast<const iterator_pair_t<TIterator, Is> &>(*this),
                        static_cast<const iterator_pair_t<TIterator, Is> &>(Rhs))
                        );
            }

            decltype(auto) operator*() const {
                return *at_number<const TValue*>(active_container_index, [](auto&& iterator)
                    {
                        return &extract_value(iterator.begin, join_iterator_tag{});
                    });
            }

            uint32_t active_container_index{ 0 };

        private:
            template<typename TThisIterator, std::size_t Is, typename RetValue, typename Func>
            constexpr RetValue with_nth_iterator(Func f)
            {
                return f(static_cast<iterator_pair_t<TThisIterator, Is> &>(*this));
            }

            template<typename TThisIterator, std::size_t Is, typename RetValue, typename Func>
            constexpr RetValue with_nth_iterator_const(Func f) const
            {
                return f(static_cast<const iterator_pair_t<TThisIterator, Is> &>(*this));
            }

            template<typename RetValue, typename Func>
            constexpr RetValue at_number(uint32_t number, Func&& func) {
                constexpr auto invoke_array = [] {
                    return std::array{ &with_nth_iterator<TIterator, Is, RetValue, Func&&>... };
                    }();
                    return (this->*invoke_array.at(number))(std::forward<Func>(func));
            }

            template<typename RetValue, typename Func>
            constexpr RetValue at_number(uint32_t number, Func&& func) const {
                constexpr auto invoke_array = [] {
                    return std::array{ &with_nth_iterator_const<TIterator, Is, RetValue, Func&&>... };
                    }();
                    return (this->*invoke_array.at(number))(std::forward<Func>(func));
            }

            template<typename TThis>
            bool is_same(const TThis& lhs, const TThis& rhs) const {
                return lhs.begin == rhs.begin && lhs.end == rhs.end;
            }
        };

        template<typename... Ts>
        join_iterator_t(std::pair<Ts, Ts>...) ->
            join_iterator_t<std::make_index_sequence<sizeof...(Ts)>, std::decay_t<Ts>...>;

        explicit join_container_impl(const TContainer&... Containers)
            : indexed_ref_t<TContainer, Is>(Containers)...{}

        auto begin() const {
            return join_iterator_t(std::pair{
                static_cast<const indexed_ref_t<TContainer, Is>&>(*this).get_value().begin(),
                static_cast<const indexed_ref_t<TContainer, Is>&>(*this).get_value().end()
                }...);
        }

        auto end() const {
            auto end_it = join_iterator_t(std::pair{
                static_cast<const indexed_ref_t<TContainer, Is>&>(*this).get_value().end(),
                static_cast<const indexed_ref_t<TContainer, Is>&>(*this).get_value().end()
                }...);
            end_it.active_container_index = sizeof...(TContainer);
            return end_it;
        }
    };

    template<typename TIterator>
    decltype(auto) extract_value(TIterator&& iterator, detail::join_iterator_tag) {
        return *iterator;
    }

}
template<typename T>
struct type_holder_t{};

template<typename TValue, typename... Ts>
class join_container_t final :
    public detail::join_container_impl<TValue, std::make_index_sequence<sizeof...(Ts)>, Ts...> {
public:
    explicit join_container_t(type_holder_t<TValue>, const Ts&...args)
        : detail::join_container_impl<TValue, std::make_index_sequence<sizeof...(Ts)>, Ts...> (args...){}
};

template<typename TValue, typename... Ts>
join_container_t(type_holder_t<TValue>, Ts...)->join_container_t<TValue, Ts...>;