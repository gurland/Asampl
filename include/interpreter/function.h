#pragma once

#include <array>
#include <cassert>
#include <functional>
#include <vector>
#include <memory>
#include <tuple>

#include <iostream>

#include "interpreter/value.h"

namespace Asampl::Interpreter {

struct ArgsRest { Tuple tuple; };

template< typename T >
T& convert_argument(ValuePtr& x) {
    if constexpr (std::is_same_v<std::decay_t<T>, ValuePtr>) {
        return x;
    } else if constexpr (std::is_same_v<std::decay_t<T>, Value>) {
        return *x;
    } else {
        return x->get<T>();
    }
}

template<typename T, typename... Ts>
std::tuple<T&, Ts&...> convert_arguments_impl(Utils::Slice<ValuePtr> args, ArgsRest& rest) {
    if constexpr (sizeof...(Ts) == 0) {
        if constexpr (std::is_same_v<std::decay_t<T>, ArgsRest>) {
            args.copy_to_vector(rest.tuple.values);
            return std::tie(rest);
        } else {
            return std::tie(convert_argument<T>(args.head()));
        }
    } else {
        return std::tuple_cat(
            std::tie(convert_argument<T>(args.head())),
            convert_arguments_impl<Ts...>(args.tail(), rest)
        );
    }
}

template<typename... Ts>
std::tuple<Ts&...> convert_arguments(Utils::Slice<ValuePtr> args, ArgsRest& rest) {
    if constexpr (sizeof...(Ts) == 0) {
        return std::make_tuple();
    } else {
        return convert_arguments_impl<Ts...>(args, rest);
    }
}

template<typename R, typename... Args>
Function make_asampl_function(const char* name, std::function<R(Args...)> func) {
    auto asampl_func = [name, func](Utils::Slice<ValuePtr> args) -> ValuePtr {
        assert(args.size() >= sizeof...(Args));

        ArgsRest rest;
        auto args_tuple = convert_arguments<Args...>(args, rest);
        if constexpr (std::is_same_v<R, void>) {
            std::apply(func, std::move(args_tuple));
            return Value::make_ptr(Undefined{});
        } else if constexpr (std::is_same_v<R, Value>) {
            return ValuePtr{std::apply(func, std::move(args_tuple))};
        } else if constexpr (std::is_same_v<R, ValuePtr>) {
            return std::apply(func, std::move(args_tuple));
        } else {
            return Value::make_ptr<R>(std::apply(func, std::move(args_tuple)));
        }
    };

    return Function{ name, std::move(asampl_func) };
}

template< typename R, typename... Args >
Function make_asampl_function(const char* name, R(*func)(Args...)) {
    std::function<R(Args...)> stdfunc{func};
    return make_asampl_function(name, stdfunc);
}

}

#define MAKE_ASAMPL_FUNCTION(F) make_asampl_function(#F, F)
