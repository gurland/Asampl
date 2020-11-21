#pragma once

#include <array>
#include <cassert>
#include <functional>
#include <vector>
#include <memory>
#include <tuple>

#include <iostream>

#include "interpreter/value.h"

using Function = std::function<ValuePtr(const std::vector<ValuePtr>&)>;

template< typename T >
auto convert_argument(ValuePtr& t) {
    if constexpr (std::is_same_v<T, ValuePtr>) {
        return std::tie(t);
    } else {
        return std::tie(t->template try_get<T>());
    }
}

template<std::size_t first_arg, typename ArgsArray, typename T, typename... Ts>
std::tuple<T&, Ts&...> convert_arguments(ArgsArray& args) {
    if constexpr (sizeof...(Ts) == 0) {
        return convert_argument<T>(args[first_arg]);
    } else {
        return std::tuple_cat(
            convert_argument<T>(args[first_arg]),
            convert_arguments<first_arg + 1, ArgsArray, Ts...>(args)
        );
    }
}

template<typename R, typename... Args>
Function make_asampl_function(std::function<R(Args...)> func) {
    return [func](const std::vector<ValuePtr>& args) -> ValuePtr {
        assert(args.size() >= sizeof...(Args));

        std::array<ValuePtr, sizeof...(Args)> args_array;
        for (std::size_t i = 0; i < sizeof...(Args); i++) {
            args_array[i] = std::move(args[i]);
        }

        auto args_tuple = convert_arguments<0, decltype(args_array), Args...>(args_array);
        if constexpr (std::is_same_v<R, void>) {
            std::apply(func, std::move(args_tuple));
            return std::make_shared<UndefinedValue>();
        } else if constexpr (std::is_same_v<R, ValuePtr>) {
            return std::apply(func, std::move(args_tuple));
        } else {
            return std::make_shared<Value<R>>(std::apply(func, std::move(args_tuple)));
        }
    };
}

template< typename R, typename... Args >
Function make_asampl_function(R(*func)(Args...)) {
    std::function<R(Args...)> stdfunc{func};
    return make_asampl_function(stdfunc);
}
